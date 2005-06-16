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

void 
carmen_logger_write_robot_name(char *robot_name, carmen_logger_file_p outfile)
{
  carmen_logger_fprintf(outfile, "# robot: %s\n", robot_name);
}

void 
carmen_logger_write_header(carmen_logger_file_p outfile)
{
  carmen_logger_fprintf(outfile, "%s\n", CARMEN_LOGFILE_HEADER);
  carmen_logger_fprintf(outfile, "# file format is one message per line\n");
  carmen_logger_fprintf(outfile, "# message_name [message contents] ipc_timestamp ipc_hostname logger_timestamp\n");
  carmen_logger_fprintf(outfile, "# message formats defined: PARAM SYNC ODOM FLASER RLASER\n");
  carmen_logger_fprintf(outfile, "# PARAM param_name param_value\n");
  carmen_logger_fprintf(outfile, "# SYNC tagname\n");
  carmen_logger_fprintf(outfile, "# ODOM x y theta tv rv accel\n");
  carmen_logger_fprintf(outfile, "# FLASER num_readings [range_readings] x y theta odom_x odom_y odom_theta\n");
  carmen_logger_fprintf(outfile, "# RLASER num_readings [range_readings] x y theta odom_x odom_y odom_theta\n");
  carmen_logger_fprintf(outfile, "# LASER3 num_readings [range_readings]\n");
  carmen_logger_fprintf(outfile, "# LASER4 num_readings [range_readings]\n");
// *** REI - START *** //
  carmen_logger_fprintf(outfile, "# REMISSIONFLASER num_readings [range_readings remission_value]\n");
  carmen_logger_fprintf(outfile, "# REMISSIONRLASER num_readings [range_readings remission_value]\n");
  carmen_logger_fprintf(outfile, "# REMISSIONLASER3 num_readings [range_readings remission_value]\n");
  carmen_logger_fprintf(outfile, "# REMISSIONLASER4 num_readings [range_readings remission_value]\n");
// *** REI - END *** //
  carmen_logger_fprintf(outfile, "# TRUEPOS true_x true_y true_theta odom_x odom_y odom_theta\n");
}

void
carmen_logger_write_odometry(carmen_base_odometry_message *odometry, carmen_logger_file_p outfile,
			     double timestamp)
{
  carmen_logger_fprintf(outfile, "ODOM %f %f %f %f %f %f %f %s %f\n", odometry->x,
			odometry->y, odometry->theta, odometry->tv, odometry->rv, 
			odometry->acceleration, odometry->timestamp, odometry->host,
			timestamp);
}

/* Added by Cyrill Stachniss, 10/9/2003 */

void carmen_logger_write_truepos(carmen_simulator_truepos_message *truepos, 
				 carmen_logger_file_p outfile, double timestamp) {

  carmen_logger_fprintf(outfile, "TRUEPOS %f %f %f %f %f %f %f %s %f\n", truepos->truepose.x,
			truepos->truepose.y, truepos->truepose.theta,  truepos->odometrypose.x,
			truepos->odometrypose.y, truepos->odometrypose.theta, truepos->timestamp, truepos->host,
			timestamp);
}

// *** REI - START *** //
void
carmen_logger_write_frontlaser_remission(carmen_laser_remission_message *rem, carmen_logger_file_p outfile,
					 double timestamp)
{
  int i;

  carmen_logger_fprintf(outfile, "REMISSIONFLASER %d ", rem->num_readings);
  for(i = 0; i < rem->num_readings; i++)
    carmen_logger_fprintf(outfile, "%.2f %.1f ", rem->range[i], rem->remission[i]);
  carmen_logger_fprintf(outfile, "%f %s %f\n", rem->timestamp,
			rem->host, timestamp);
}

void
carmen_logger_write_rearlaser_remission(carmen_laser_remission_message *rem, carmen_logger_file_p outfile,
					 double timestamp)
{
  int i;

  carmen_logger_fprintf(outfile, "REMISSIONRLASER %d ", rem->num_readings);
  for(i = 0; i < rem->num_readings; i++)
    carmen_logger_fprintf(outfile, "%.2f %.1f ", rem->range[i], rem->remission[i]);
  carmen_logger_fprintf(outfile, "%f %s %f\n", rem->timestamp,
			rem->host, timestamp);
}

void
carmen_logger_write_laser3_remission(carmen_laser_remission_message *rem, carmen_logger_file_p outfile,
					 double timestamp)
{
  int i;

  carmen_logger_fprintf(outfile, "REMISSIONLASER3 %d ", rem->num_readings);
  for(i = 0; i < rem->num_readings; i++)
    carmen_logger_fprintf(outfile, "%.2f %.1f ", rem->range[i], rem->remission[i]);
  carmen_logger_fprintf(outfile, "%f %s %f\n", rem->timestamp,
			rem->host, timestamp);
}

void
carmen_logger_write_laser4_remission(carmen_laser_remission_message *rem, carmen_logger_file_p outfile,
					 double timestamp)
{
  int i;

  carmen_logger_fprintf(outfile, "REMISSIONLASER4 %d ", rem->num_readings);
  for(i = 0; i < rem->num_readings; i++)
    carmen_logger_fprintf(outfile, "%.2f %.1f ", rem->range[i], rem->remission[i]);
  carmen_logger_fprintf(outfile, "%f %s %f\n", rem->timestamp,
			rem->host, timestamp);
}
// *** REI - END *** //

void
carmen_logger_write_frontlaser(carmen_robot_laser_message *frontlaser, carmen_logger_file_p outfile,
			       double timestamp)
{
  int i;

  carmen_logger_fprintf(outfile, "FLASER %d ", frontlaser->num_readings);
  for(i = 0; i < frontlaser->num_readings; i++)
    carmen_logger_fprintf(outfile, "%.2f ", frontlaser->range[i]);
  carmen_logger_fprintf(outfile, "%f %f %f %f %f %f %f %s %f\n", frontlaser->x, 
	  frontlaser->y, frontlaser->theta, frontlaser->odom_x,
	  frontlaser->odom_y, frontlaser->odom_theta, frontlaser->timestamp,
	  frontlaser->host, timestamp);
}

void
carmen_logger_write_rearlaser(carmen_robot_laser_message *rearlaser, 
			      carmen_logger_file_p outfile,
			      double timestamp)
{
  int i;

  carmen_logger_fprintf(outfile, "RLASER %d ", rearlaser->num_readings);
  for(i = 0; i < rearlaser->num_readings; i++)
    carmen_logger_fprintf(outfile, "%.2f ", rearlaser->range[i]);
  carmen_logger_fprintf(outfile, "%f %f %f %f %f %f %f %s %f\n", rearlaser->x, 
	  rearlaser->y, rearlaser->theta, rearlaser->odom_x,
	  rearlaser->odom_y, rearlaser->odom_theta, rearlaser->timestamp,
	  rearlaser->host, timestamp);
}

void
carmen_logger_write_laser3(carmen_laser_laser_message *laser, carmen_logger_file_p outfile,
			   double timestamp)
{
  int i;
  
  carmen_logger_fprintf(outfile, "LASER3 %d ", laser->num_readings);
  for(i = 0; i < laser->num_readings; i++)
    carmen_logger_fprintf(outfile, "%.2f ", laser->range[i]);
  carmen_logger_fprintf(outfile, "%f %s %f\n", laser->timestamp,
	  laser->host, timestamp);
}

void
carmen_logger_write_laser4(carmen_laser_laser_message *laser, carmen_logger_file_p outfile,
			   double timestamp)
{
  int i;
  
  carmen_logger_fprintf(outfile, "LASER4 %d ", laser->num_readings);
  for(i = 0; i < laser->num_readings; i++)
    carmen_logger_fprintf(outfile, "%.2f ", laser->range[i]);
  carmen_logger_fprintf(outfile, "%f %s %f\n", laser->timestamp,
	  laser->host, timestamp);
}

void
carmen_logger_write_param(char *module, char *variable, char *value, 
			  double ipc_time, char *hostname, 
			  carmen_logger_file_p outfile, double timestamp)
{
  carmen_logger_fprintf(outfile, "PARAM %s_%s %s %f %s %f\n", module, 
			variable, value, ipc_time, hostname, timestamp);
}

void
carmen_logger_write_sync(carmen_logger_sync_message *sync_message, 
			 carmen_logger_file_p outfile) {
  carmen_logger_fprintf(outfile, "SYNC %s %f %s %f\n", sync_message->tag,
			sync_message->timestamp, sync_message->host,
			carmen_get_time_ms());
}

void carmen_logger_write_localize(carmen_localize_globalpos_message *msg, 
				  carmen_logger_file_p outfile, double timestamp)
{
  carmen_logger_fprintf(outfile, "TRUEPOS %f %f %f %f %f %f %f %s %f\n",
			msg->globalpos.x, msg->globalpos.y, msg->globalpos.theta,
			msg->odometrypos.x, msg->odometrypos.y, msg->odometrypos.theta,
			msg->timestamp, msg->host, timestamp);
}

#ifndef NO_ZLIB

void
carmen_logger_fprintf(carmen_logger_file_p stream, const char *format, ...)
{
  va_list args;
  static char buf[40000];

  va_start(args, format);
  if (carmen_logger_nogz)
    vfprintf(stream, format, args);
  else {
    vsnprintf(buf, 40000, format, args);
    gzprintf(stream, buf);
  }
  va_end(args);
}

void
carmen_playback_fprintf(carmen_logger_file_p stream, const char *format, ...)
{
  va_list args;
  static char buf[40000];

  va_start(args, format);
  if (carmen_playback_nogz)
    vfprintf(stream, format, args);
  else {
    vsnprintf(buf, 40000, format, args);
    gzprintf(stream, buf);
  }
  va_end(args);
}

#endif
