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

#ifndef CARMEN_LOGWRITE_H
#define CARMEN_LOGWRITE_H

#include <carmen/carmen.h>
#include <carmen/carmen_stdio.h>

#define CARMEN_LOGFILE_HEADER "# CARMEN Logfile"

void carmen_logwrite_write_robot_name(char *robot_name, 
				      carmen_FILE *outfile);

void carmen_logwrite_write_header(carmen_FILE *outfile);

void carmen_logwrite_write_odometry(carmen_base_odometry_message *odometry, 
				    carmen_FILE *outfile, double timestamp);  

void carmen_logwrite_write_laser_laser(carmen_laser_laser_message *laser,
				       int laser_num, carmen_FILE *outfile,
				       double timestamp);

void carmen_logwrite_write_robot_laser(carmen_robot_laser_message *laser,
				       int laser_num, carmen_FILE *outfile,
				       double timestamp);

void carmen_logwrite_write_param(char *module, char *variable, char *value, 
				 double ipc_time, char *hostname, 
				 carmen_FILE *outfile, double timestamp);

void carmen_logwrite_write_sync(carmen_logger_sync_message *sync_message, 
				carmen_FILE *outfile);

void carmen_logwrite_write_truepos(carmen_simulator_truepos_message *truepos, 
				   carmen_FILE *outfile, double timestamp);  

void carmen_logwrite_write_localize(carmen_localize_globalpos_message *msg, 
				    carmen_FILE *outfile, double timestamp);

void carmen_logwrite_write_gps_gpgga(carmen_gps_gpgga_message *gps, 
				     carmen_FILE *outfile, double timestamp);


#endif
