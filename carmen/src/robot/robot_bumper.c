/*********************************************************
 *
 * This source code is part of the Carnegie Mellon Robot
 * Navigation Toolkit (CARMEN)
 *
 * CARMEN Copyright (c) 2002 Michael Montemerlo, Nicholas
 * Roy, and Sebastian Thrun
 *
 * CARMEN is free software; you can redistribute it and/or 
 * modify it under the terms of the GNU General Public 
 * License as published by the Free Software Foundation; 
 * either version 2 of the License, or (at your option)
 * any later version.
 *
 * CARMEN is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied 
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 * PURPOSE.  See the GNU General Public License for more 
 * details.
 *
 * You should have received a copy of the GNU General 
 * Public License along with CARMEN; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place, 
 * Suite 330, Boston, MA  02111-1307 USA
 *
 ********************************************************/

#include <carmen/carmen.h>

#include "robot_central.h"
#include "robot_main.h"
#include "robot_bumper.h"

#define BUMPER_AVERAGE             3

extern int carmen_robot_odometry_count; 

static carmen_base_bumper_message base_bumper;
static double bumper_local_timestamp;
static carmen_robot_bumper_message robot_bumper;

static int bumper_count = 0;
static int bumper_ready = 0;
static int num_bumpers;

double carmen_robot_interpolate_heading(double head1, double head2, 
					double fraction);

static void check_message_data_chunk_sizes(void)
{
  int first = 1;

  if (first) {
    robot_bumper.num_bumpers = num_bumpers;
    robot_bumper.state = (char *)calloc(num_bumpers, sizeof(char));
    carmen_test_alloc(robot_bumper.state);
    first = 0;
  } 
}
      

static void construct_bumper_message(carmen_robot_bumper_message *msg, 
				     int low, int high, double fraction)
{
  int i;

  msg->robot_pose.x=carmen_robot_odometry[low].x + fraction *
    (carmen_robot_odometry[high].x - carmen_robot_odometry[low].x);
  msg->robot_pose.y= carmen_robot_odometry[low].y + fraction *
    (carmen_robot_odometry[high].y - carmen_robot_odometry[low].y);
  msg->robot_pose.theta=carmen_robot_interpolate_heading
    (carmen_robot_odometry[high].theta, 
     carmen_robot_odometry[low].theta, fraction);

  msg->tv = carmen_robot_odometry[low].tv + 
    fraction*(carmen_robot_odometry[high].tv - 
	      carmen_robot_odometry[low].tv);
  msg->rv = carmen_robot_odometry[low].rv + 
    fraction*(carmen_robot_odometry[high].rv - 
	      carmen_robot_odometry[low].rv);

  robot_bumper.timestamp = base_bumper.timestamp;
  strncpy(robot_bumper.host, base_bumper.host, 10);
  robot_bumper.num_bumpers = base_bumper.num_bumpers;

  for(i=0; i< msg->num_bumpers; i++)
    msg->state[i] = base_bumper.state[i];
}

void carmen_robot_correct_bumper_and_publish(void) 
{  
  double bumper_skew;
  double odometry_skew;
  double fraction;
  int i, low, high;
  
  if(!bumper_ready)
    return;

  check_message_data_chunk_sizes();
  
  if(!bumper_ready) 
    return;

  if (strcmp(carmen_robot_host, base_bumper.host) == 0)
    bumper_skew = 0;
  else {
    bumper_skew = carmen_running_average_report(BUMPER_AVERAGE);
    if (bumper_count < ESTIMATES_CONVERGE) {
      carmen_warn("Waiting for bumper data to accumulate\n");
      return;
    }
  }

  if (carmen_robot_odometry_count < ESTIMATES_CONVERGE) {
    carmen_warn("Waiting for odometry to accumulate\n");
    return;
  }  

  odometry_skew = carmen_robot_get_odometry_skew();
  
  low = 0;
  high = 1;
  for(i = 0; i < MAX_READINGS; i++) 
    if (base_bumper.timestamp + bumper_skew <
	carmen_robot_odometry[i].timestamp + odometry_skew) {
      if (i == 0) {
	low = 0;
	high = 1;
      } else {
	low = i-1;
	high = i;
      }      
      break;
    }

  if (i == MAX_READINGS) {
    low = i-2;
    high = i-1;
  }

  fraction = (base_bumper.timestamp - carmen_robot_odometry[low].timestamp)/
    (carmen_robot_odometry[high].timestamp - 
     carmen_robot_odometry[low].timestamp);
  
  construct_bumper_message(&robot_bumper, low, high, fraction);
    
  IPC_RETURN_TYPE err;
  err = IPC_publishData(CARMEN_ROBOT_BUMPER_NAME, &robot_bumper);
  carmen_test_ipc_exit(err, "Could not publish", CARMEN_ROBOT_BUMPER_NAME);

  fprintf(stderr, "b");
  bumper_ready = 0;  
}

int carmen_robot_bumper_on() 
{
  int i;

  for (i = 0; i < base_bumper.num_bumpers; i++)
    if (base_bumper.state[i] == 0)
      return 1;

  return 0;
}

static void bumper_handler(void)
{
  bumper_local_timestamp = carmen_get_time();
  if(bumper_count <= ESTIMATES_CONVERGE)
    bumper_count++;

  carmen_running_average_add
    (BUMPER_AVERAGE, bumper_local_timestamp-base_bumper.timestamp);    
  
  bumper_ready=1;  
}

void carmen_robot_add_bumper_handler(void) 
{
  IPC_RETURN_TYPE err;

  err = IPC_defineMsg(CARMEN_ROBOT_BUMPER_NAME,
		      IPC_VARIABLE_LENGTH,
		      CARMEN_ROBOT_BUMPER_FMT);
  carmen_test_ipc_exit(err, "Could not define", CARMEN_ROBOT_BUMPER_NAME);

  carmen_base_subscribe_bumper_message(&base_bumper,
				       (carmen_handler_t)bumper_handler,
				       CARMEN_SUBSCRIBE_LATEST);
}

void carmen_robot_add_bumper_parameters(char *progname __attribute__ ((unused)) ) 
{

}

