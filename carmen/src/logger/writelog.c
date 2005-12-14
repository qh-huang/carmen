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

void carmen_logwrite_write_robot_name(char *robot_name, carmen_FILE *outfile)
{
  carmen_fprintf(outfile, "# robot: %s\n", robot_name);
}

void carmen_logwrite_write_header(carmen_FILE *outfile)
{
  carmen_fprintf(outfile, "%s\n", CARMEN_LOGFILE_HEADER);
  carmen_fprintf(outfile, "# file format is one message per line\n");
  carmen_fprintf(outfile, "# message_name [message contents] ipc_timestamp ipc_hostname logger_timestamp\n");
  carmen_fprintf(outfile, "# message formats defined: PARAM SYNC ODOM FLASER RLASER\n");
  carmen_fprintf(outfile, "# PARAM param_name param_value\n");
  carmen_fprintf(outfile, "# SYNC tagname\n");
  carmen_fprintf(outfile, "# ODOM x y theta tv rv accel\n");
  carmen_fprintf(outfile, "# FLASER num_readings [range_readings] x y theta odom_x odom_y odom_theta\n");
  carmen_fprintf(outfile, "# RLASER num_readings [range_readings] x y theta odom_x odom_y odom_theta\n");
  carmen_fprintf(outfile, "# LASER3 num_readings [range_readings]\n");
  carmen_fprintf(outfile, "# LASER4 num_readings [range_readings]\n");
  carmen_fprintf(outfile, "# REMISSIONFLASER num_readings [range_readings remission_value]\n");
  carmen_fprintf(outfile, "# REMISSIONRLASER num_readings [range_readings remission_value]\n");
  carmen_fprintf(outfile, "# REMISSIONLASER3 num_readings [range_readings remission_value]\n");
  carmen_fprintf(outfile, "# REMISSIONLASER4 num_readings [range_readings remission_value]\n");
  carmen_fprintf(outfile, "# TRUEPOS true_x true_y true_theta odom_x odom_y odom_theta\n");
}

void carmen_logwrite_write_odometry(carmen_base_odometry_message *odometry, 
				    carmen_FILE *outfile,
				    double timestamp)
{
  carmen_fprintf(outfile, "ODOM %f %f %f %f %f %f %f %s %f\n", odometry->x,
		 odometry->y, odometry->theta, odometry->tv, odometry->rv, 
		 odometry->acceleration, odometry->timestamp, odometry->host,
		 timestamp);
}

void carmen_logwrite_write_truepos(carmen_simulator_truepos_message *truepos, 
				   carmen_FILE *outfile, double timestamp)
{

  carmen_fprintf(outfile, "TRUEPOS %f %f %f %f %f %f %f %s %f\n", 
		 truepos->truepose.x, truepos->truepose.y, 
		 truepos->truepose.theta,  truepos->odometrypose.x,
		 truepos->odometrypose.y, truepos->odometrypose.theta, 
		 truepos->timestamp, truepos->host, timestamp);
}

void carmen_logwrite_write_laser_laser(carmen_laser_laser_message *laser,
				       int laser_num, carmen_FILE *outfile,
				       double timestamp)
{
  int i;

  carmen_fprintf(outfile, "RAWLASER%d ", laser_num);
  carmen_fprintf(outfile, "%d %f %f %f %f %f ", laser->config.laser_type,
		 laser->config.start_angle, laser->config.fov, 
		 laser->config.angular_resolution, 
		 laser->config.maximum_range, laser->config.accuracy);
  carmen_fprintf(outfile, "%d ", laser->num_readings);
  for(i = 0; i < laser->num_readings; i++)
    carmen_fprintf(outfile, "%.2f ", laser->range[i]);
  carmen_fprintf(outfile, "%d ", laser->num_remissions);
  for(i = 0; i < laser->num_remissions; i++)
    carmen_fprintf(outfile, "%.1f ", laser->range[i]);
  carmen_fprintf(outfile, "%f %s %f\n", laser->timestamp,
		 laser->host, timestamp);
}

void carmen_logwrite_write_robot_laser(carmen_robot_laser_message *laser,
				       int laser_num, carmen_FILE *outfile,
				       double timestamp)
{
  int i;

  carmen_fprintf(outfile, "ROBOTLASER%d ", laser_num);
  carmen_fprintf(outfile, "%d %f %f %f %f %f ", laser->config.laser_type,
		 laser->config.start_angle, laser->config.fov, 
		 laser->config.angular_resolution, 
		 laser->config.maximum_range, laser->config.accuracy);
  carmen_fprintf(outfile, "%d ", laser->num_readings);
  for(i = 0; i < laser->num_readings; i++)
    carmen_fprintf(outfile, "%.2f ", laser->range[i]);
  carmen_fprintf(outfile, "%d ", laser->num_remissions);
  for(i = 0; i < laser->num_remissions; i++)
    carmen_fprintf(outfile, "%.1f ", laser->range[i]);
  carmen_fprintf(outfile, "%f %f %f %f %f %f ", laser->laser_pose.x,
		 laser->laser_pose.y, laser->laser_pose.theta,
		 laser->robot_pose.x, laser->robot_pose.y, 
		 laser->robot_pose.theta);
  carmen_fprintf(outfile, "%f %f %f %f %f ", laser->tv, laser->rv, 
		 laser->forward_safety_dist, laser->side_safety_dist,
		 laser->turn_axis);
  carmen_fprintf(outfile, "%f %s %f\n", laser->timestamp,
		 laser->host, timestamp);
}

void carmen_logwrite_write_param(char *module, char *variable, char *value, 
				 double ipc_time, char *hostname, 
				 carmen_FILE *outfile, double timestamp)
{
  carmen_fprintf(outfile, "PARAM %s_%s %s %f %s %f\n", module, 
		 variable, value, ipc_time, hostname, timestamp);
}

void carmen_logwrite_write_sync(carmen_logger_sync_message *sync_message, 
				carmen_FILE *outfile)
{
  carmen_fprintf(outfile, "SYNC %s %f %s %f\n", sync_message->tag,
		 sync_message->timestamp, sync_message->host,
		 carmen_get_time());
}

void carmen_logwrite_write_localize(carmen_localize_globalpos_message *msg, 
				    carmen_FILE *outfile, double timestamp)
{
  carmen_fprintf(outfile, "TRUEPOS %f %f %f %f %f %f %f %s %f\n",
		 msg->globalpos.x, msg->globalpos.y, msg->globalpos.theta,
		 msg->odometrypos.x, msg->odometrypos.y, 
		 msg->odometrypos.theta, msg->timestamp, msg->host, timestamp);
}

