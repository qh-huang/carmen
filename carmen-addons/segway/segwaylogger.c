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
#include "carmen/logger.h"
#include "carmen/logger_io.h"
#include "segway_interface.h"
#include <carmen/amtec_messages.h>
#include <carmen/amtec_interface.h>
#include <ctype.h>

int carmen_logger_nogz;
int carmen_playback_nogz;

carmen_logger_file_p outfile = NULL;
double logger_starttime;
carmen_segway_pose_message pose;
carmen_amtec_status_message amtec_status;

void get_all_params(void)
{
  char **variables, **values;
  int list_length;
  int index;

  char *robot_name;
  char **modules;
  int num_modules;
  int module_index;

  robot_name = carmen_param_get_robot();
  carmen_param_get_modules(&modules, &num_modules);

  carmen_logger_write_robot_name(robot_name, outfile);
  free(robot_name);
	
  for (module_index = 0; module_index < num_modules; module_index++) {
    if(carmen_param_get_all(modules[module_index], &variables, &values, 
			    &list_length) < 0) {
      IPC_perror("Error retrieving all variables of module");
      exit(-1);
    }
    for(index = 0; index < list_length; index++) {
      carmen_logger_write_param(modules[module_index], variables[index], 
				values[index], outfile);
      free(variables[index]);
      free(values[index]);
    }
    free(variables);
    free(values);
    free(modules[module_index]);
  }

  free(modules);
}

void param_change_handler(MSG_INSTANCE msgRef, BYTE_ARRAY callData,
			  void *clientData __attribute__ ((unused)))
{
  FORMATTER_PTR formatter;
  IPC_RETURN_TYPE err = IPC_OK;
  carmen_param_variable_change_message msg;
  
  formatter = IPC_msgInstanceFormatter(msgRef);
  err = IPC_unmarshallData(formatter, callData, &msg, 
			   sizeof(carmen_param_variable_change_message));
  IPC_freeByteArray(callData);

  fprintf(stderr, "Warning: Parameter change recording not yet implemented.\n");
}

void base_odometry_handler(carmen_base_odometry_message *odometry)
{
  fprintf(stderr, "O");
  carmen_logger_write_odometry(odometry, outfile, 
			       carmen_get_time_ms() - logger_starttime);
}


void robot_frontlaser_handler(carmen_laser_laser_message *frontlaser)
{
  int i;
  fprintf(stderr, "F");
  carmen_logger_fprintf(outfile, "FLASER %d ", frontlaser->num_readings);
  for(i = 0; i < frontlaser->num_readings; i++)
    carmen_logger_fprintf(outfile, "%.2f ", frontlaser->range[i]);
  carmen_logger_fprintf(outfile, "%f %s %f\n", frontlaser->timestamp,
	  frontlaser->host, carmen_get_time_ms() - logger_starttime);

}

void segway_pose_handler(carmen_segway_pose_message *pose)
{
  fprintf(stderr, "S");
  carmen_logger_fprintf(outfile, "SEGWAY %f %f %f %f %f %f %f %f %f %f %s %f\n",
			pose->pitch, pose->pitch_rate, pose->roll,
			pose->roll_rate, pose->lw_velocity, pose->rw_velocity,
			pose->x, pose->y, pose->theta, pose->timestamp,
			pose->host, carmen_get_time_ms() - logger_starttime);
}

void amtec_status_handler(carmen_amtec_status_message *amtec_status)
{
  fprintf(stderr, "A");
  carmen_logger_fprintf(outfile, "AMTEC %f %f %f %f %f %s %f\n",
			amtec_status->pan, amtec_status->tilt,
			amtec_status->pan_vel, amtec_status->tilt_vel,
			amtec_status->timestamp, amtec_status->host,
			carmen_get_time_ms() - logger_starttime);
}

static void sync_handler(MSG_INSTANCE msgRef, BYTE_ARRAY callData,
			 void *clientData __attribute__ ((unused)))
{
  IPC_RETURN_TYPE err;
  carmen_logger_sync_message sync_message;
  FORMATTER_PTR formatter;
  
  formatter = IPC_msgInstanceFormatter(msgRef);
  err = IPC_unmarshallData(formatter, callData, &sync_message,
                           sizeof(carmen_logger_sync_message));
  IPC_freeByteArray(callData);
  carmen_test_ipc_return(err, "Could not unmarshall", 
			 IPC_msgInstanceName(msgRef));

  carmen_logger_write_sync(&sync_message, outfile);
  free(sync_message.tag);
}

void register_ipc_messages(void)
{
  IPC_RETURN_TYPE err;

  err = IPC_defineMsg(CARMEN_LOGGER_SYNC_NAME, IPC_VARIABLE_LENGTH,
                      CARMEN_LOGGER_SYNC_FMT);
  carmen_test_ipc_exit(err, "Could not define", CARMEN_LOGGER_SYNC_NAME);

  err = IPC_subscribe(CARMEN_LOGGER_SYNC_NAME, sync_handler, NULL);
  carmen_test_ipc_exit(err, "Could not subscribe", 
		       CARMEN_LOGGER_SYNC_NAME);
  IPC_setMsgQueueLength(CARMEN_LOGGER_SYNC_NAME, 1);
}

void shutdown_module(int sig)
{
  if(sig == SIGINT) {
    carmen_logger_fclose(outfile);
    close_ipc();
    fprintf(stderr, "\nDisconnecting.\n");
    exit(0);
  }
}

int main(int argc, char **argv)
{
  char filename[1024];
  IPC_RETURN_TYPE err;
  char key;

  carmen_base_odometry_message odometry;
  carmen_laser_laser_message frontlaser;

  carmen_initialize_ipc(argv[0]);
  carmen_param_check_version(argv[0]);	

  if (argc < 2) 
    carmen_die("usage: %s [--nogz] <logfile>\n", argv[0]);

  carmen_logger_nogz = 0;
  if (argc == 2)
    sprintf(filename, argv[1]);
  else if (argc == 3) {
    if (!strcmp(argv[1], "--nogz"))
      carmen_logger_nogz = 1;
    else
      carmen_die("usage: %s [--nogz] <logfile>\n", argv[0]);
    sprintf(filename, argv[2]);
  }

#ifndef NO_ZLIB
  if (!carmen_logger_nogz && strcmp(filename + strlen(filename) - 3, ".gz"))
    sprintf(filename + strlen(filename), ".gz");
#endif

  outfile = carmen_logger_fopen(filename, "r");
  if (outfile != NULL) {
    fprintf(stderr, "Overwrite %s? ", filename);
    scanf("%c", &key);
    if (toupper(key) != 'Y')
      exit(-1);
    carmen_logger_fclose(outfile);
  }
  outfile = carmen_logger_fopen(filename, "w");
  if(outfile == NULL)
    carmen_die("Error: Could not open file %s for writing.\n", filename);
  carmen_logger_write_header(outfile);

  get_all_params();
  err = IPC_subscribe(CARMEN_PARAM_VARIABLE_CHANGE_NAME, param_change_handler,
		      NULL);
  carmen_test_ipc(err, "Could not subscribe", 
		  CARMEN_PARAM_VARIABLE_CHANGE_NAME);
  IPC_setMsgQueueLength(CARMEN_PARAM_VARIABLE_CHANGE_NAME, 1);

  carmen_base_subscribe_odometry_message(&odometry, (carmen_handler_t)base_odometry_handler, 
					 CARMEN_SUBSCRIBE_ALL);
  carmen_laser_subscribe_frontlaser_message(&frontlaser, 
					    (carmen_handler_t)robot_frontlaser_handler, 
					    CARMEN_SUBSCRIBE_ALL);
  carmen_segway_subscribe_pose_message(&pose, (carmen_handler_t)segway_pose_handler, CARMEN_SUBSCRIBE_ALL);
  carmen_amtec_subscribe_status_message(&amtec_status, (carmen_handler_t)amtec_status_handler, CARMEN_SUBSCRIBE_ALL);

  logger_starttime = carmen_get_time_ms();


  signal(SIGINT, shutdown_module);
  IPC_dispatch();
  return 0;
}
