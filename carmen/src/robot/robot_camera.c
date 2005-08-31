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
#include "robot_camera.h"

#define CAMERA_AVERAGE             3

extern int carmen_robot_odometry_count; 

static carmen_camera_image_message camera_image;
static double camera_local_timestamp;
static carmen_robot_camera_message robot_camera;

static int camera_count = 0;
static int camera_ready = 0;
static int image_size = 0;

double carmen_robot_interpolate_heading(double head1, double head2, 
					double fraction);

static void check_message_data_chunk_sizes(void)
{
  static int first = 1;

  if (first) {
    image_size = camera_image.image_size;
    robot_camera.image_size = image_size;
    robot_camera.image = (char *)calloc(image_size, sizeof(char));
    carmen_test_alloc(robot_camera.image);
    first = 0;
  } 
}
      

static void construct_camera_message(carmen_robot_camera_message *msg, 
				     int low, int high, double fraction)
{
  msg->odom_x=carmen_robot_odometry[low].x + fraction *
    (carmen_robot_odometry[high].x - carmen_robot_odometry[low].x);
  msg->odom_y= carmen_robot_odometry[low].y + fraction *
    (carmen_robot_odometry[high].y - carmen_robot_odometry[low].y);
  msg->odom_theta=carmen_robot_interpolate_heading
    (carmen_robot_odometry[high].theta, 
     carmen_robot_odometry[low].theta, fraction);

  msg->tv = carmen_robot_odometry[low].tv + 
    fraction*(carmen_robot_odometry[high].tv - 
	      carmen_robot_odometry[low].tv);
  msg->rv = carmen_robot_odometry[low].rv + 
    fraction*(carmen_robot_odometry[high].rv - 
	      carmen_robot_odometry[low].rv);

  robot_camera.timestamp = camera_image.timestamp;
  strncpy(robot_camera.host, camera_image.host, 10);
  robot_camera.width = camera_image.width;
  robot_camera.height = camera_image.height;
  robot_camera.bytes_per_pixel = camera_image.bytes_per_pixel;
  robot_camera.image_size = camera_image.image_size;

  memcpy(robot_camera.image, camera_image.image, 
	 camera_image.image_size*sizeof(char));

}

static void publish_camera_message(carmen_robot_camera_message camera)
{
  IPC_RETURN_TYPE err;
  err = IPC_publishData(CARMEN_ROBOT_CAMERA_NAME, &camera);
  carmen_test_ipc_exit(err, "Could not publish", CARMEN_ROBOT_CAMERA_NAME);
}

void carmen_robot_correct_camera_and_publish(void) 
{  
  double camera_skew;
  double odometry_skew;
  double fraction;
  int i, low, high;
  
  if (!camera_ready) 
    return;

  check_message_data_chunk_sizes();
  
  if (strcmp(carmen_robot_host, camera_image.host) == 0)
    camera_skew = 0;
  else 
    camera_skew = carmen_running_average_report(CAMERA_AVERAGE);
  
  odometry_skew = carmen_robot_get_odometry_skew();
  
  if (camera_count < ESTIMATES_CONVERGE) {
    carmen_warn("Waiting for camera data to accumulate\n");
    return;
  }

  if (carmen_robot_odometry_count < ESTIMATES_CONVERGE) {
    carmen_warn("Waiting for odometry to accumulate\n");
    return;
  }
  
  low = 0;
  high = 1;
  for(i = 0; i < MAX_READINGS; i++) 
    if (camera_image.timestamp + camera_skew <
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

  fraction = (camera_image.timestamp - carmen_robot_odometry[low].timestamp)/
    (carmen_robot_odometry[high].timestamp - 
     carmen_robot_odometry[low].timestamp);

  construct_camera_message(&robot_camera, low, high, fraction);
  
  // publish odometry corrected laser message 
  publish_camera_message(robot_camera);
  fprintf(stderr, "s");
    
  camera_ready = 0;  
}

static void camera_handler(void)
{
  carmen_warn("c");
  
  camera_local_timestamp = carmen_get_time();
  if(camera_count <= ESTIMATES_CONVERGE)
    camera_count++;

  carmen_running_average_add
    (CAMERA_AVERAGE, camera_local_timestamp-camera_image.timestamp);    
  
  camera_ready=1;
  
  if(carmen_robot_camera_on())
    carmen_robot_stop_robot(ALL_STOP);  
}

int carmen_robot_camera_on() 
{
  return 0;
}

void carmen_robot_add_camera_handler(void) 
{
  IPC_RETURN_TYPE err;

  err = IPC_defineMsg(CARMEN_ROBOT_CAMERA_NAME,
		      IPC_VARIABLE_LENGTH,
		      CARMEN_ROBOT_CAMERA_FMT);
  carmen_test_ipc_exit(err, "Could not define", CARMEN_ROBOT_CAMERA_NAME);

  carmen_camera_subscribe_images(&camera_image, 
				 (carmen_handler_t)camera_handler,
				 CARMEN_SUBSCRIBE_LATEST);
}

void carmen_robot_add_camera_parameters(char *progname __attribute__ ((unused)) ) 
{

}

