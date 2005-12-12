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
#include "logger.h"
#include "logger_io.h"
#include <ctype.h>

int carmen_logger_nogz;
int carmen_playback_nogz;

carmen_logger_file_p outfile = NULL;
double logger_starttime;

void get_all_params(void)
{
  char **variables, **values;
  int list_length;
  int index;

  char *robot_name;
  char **modules;
  int num_modules;
  int module_index;
  char *hostname;

  robot_name = carmen_param_get_robot();
  carmen_param_get_modules(&modules, &num_modules);

  carmen_logger_write_robot_name(robot_name, outfile);
  free(robot_name);
  carmen_param_get_paramserver_host(&hostname);
  for (module_index = 0; module_index < num_modules; module_index++) {
    if(carmen_param_get_all(modules[module_index], &variables, &values, 
			    &list_length) < 0) {
      IPC_perror("Error retrieving all variables of module");
      exit(-1);
    }
    for(index = 0; index < list_length; index++) {
      carmen_logger_write_param(modules[module_index], variables[index], 
				values[index], carmen_get_time(), 
				hostname, outfile, carmen_get_time());
      free(variables[index]);
      free(values[index]);
    }
    free(variables);
    free(values);
    free(modules[module_index]);
  }
  free(hostname);
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

  carmen_logger_write_param(msg.module_name, msg.variable_name, 
			    msg.value, msg.timestamp, 
			    msg.host, outfile, carmen_get_time());
}

/* Truepos message support added by Cyrill Stachniss, 10/9/2003 */

void carmen_simulator_truepos_handler(carmen_simulator_truepos_message *truepos)
{
  fprintf(stderr, "T");
  carmen_logger_write_truepos(truepos, outfile, 
                              carmen_get_time() - logger_starttime);
}

void base_odometry_handler(carmen_base_odometry_message *odometry)
{
  fprintf(stderr, "O");
  carmen_logger_write_odometry(odometry, outfile, 
			       carmen_get_time() - logger_starttime);
}


void robot_frontlaser_handler(carmen_robot_laser_message *frontlaser)
{
  fprintf(stderr, "F");
  carmen_logger_write_frontlaser(frontlaser, outfile, 
				 carmen_get_time() - logger_starttime);
}


void laser_laser3_handler(carmen_laser_laser_message *laser)
{
  fprintf(stderr, "3");
  carmen_logger_write_laser3(laser, outfile,
			     carmen_get_time() - logger_starttime);
}

void laser_laser4_handler(carmen_laser_laser_message *laser)
{
  fprintf(stderr, "4");
  carmen_logger_write_laser4(laser, outfile,
			     carmen_get_time() - logger_starttime);
}


// *** REI - START *** //
void laser_frontlaser_remission_handler(carmen_laser_remission_message *rem)
{
  fprintf(stderr, "f");
  carmen_logger_write_frontlaser_remission(rem, outfile,
					   carmen_get_time() - logger_starttime);
}

void laser_rearlaser_remission_handler(carmen_laser_remission_message *rem)
{
  fprintf(stderr, "r");
  carmen_logger_write_rearlaser_remission(rem, outfile,
					   carmen_get_time() - logger_starttime);
}

void laser_laser3_remission_handler(carmen_laser_remission_message *rem)
{
  fprintf(stderr, "3");
  carmen_logger_write_laser3_remission(rem, outfile,
					   carmen_get_time() - logger_starttime);
}

void laser_laser4_remission_handler(carmen_laser_remission_message *rem)
{
  fprintf(stderr, "4");
  carmen_logger_write_laser4_remission(rem, outfile,
					   carmen_get_time() - logger_starttime);
}
// *** REI - END *** //

void robot_rearlaser_handler(carmen_robot_laser_message *rearlaser)
{
  fprintf(stderr, "R");
  carmen_logger_write_rearlaser(rearlaser, outfile, 
				carmen_get_time() - logger_starttime);
}

void localize_handler(carmen_localize_globalpos_message *msg)
{
  fprintf(stderr, "L");
  carmen_logger_write_localize(msg, outfile, carmen_get_time() - logger_starttime);
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
    carmen_ipc_disconnect();
    fprintf(stderr, "\nDisconnecting.\n");
    exit(0);
  }
}

int main(int argc, char **argv)
{
  char filename[1024];
  IPC_RETURN_TYPE err;
  char key;

/*   carmen_base_odometry_message odometry; */
/*   carmen_robot_laser_message frontlaser, rearlaser; */

// *** REI - START *** //
  carmen_laser_remission_message frontlaser_remission, rearlaser_remission, laser3_remission, laser4_remission;
// *** REI - START *** //

  carmen_ipc_initialize(argc, argv);
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

  carmen_base_subscribe_odometry_message(NULL, (carmen_handler_t)base_odometry_handler, 
					 CARMEN_SUBSCRIBE_ALL);
  carmen_robot_subscribe_frontlaser_message(NULL, 
					    (carmen_handler_t)robot_frontlaser_handler, 
					    CARMEN_SUBSCRIBE_ALL);
  carmen_robot_subscribe_rearlaser_message(NULL, (carmen_handler_t)robot_rearlaser_handler, 
					   CARMEN_SUBSCRIBE_ALL);

  carmen_laser_subscribe_laser3_message(NULL, 
					(carmen_handler_t)laser_laser3_handler, 
					CARMEN_SUBSCRIBE_ALL);


  carmen_laser_subscribe_laser4_message(NULL, 
					(carmen_handler_t)laser_laser4_handler, 
					CARMEN_SUBSCRIBE_ALL);

  carmen_localize_subscribe_globalpos_message(NULL, (carmen_handler_t) localize_handler,
					      CARMEN_SUBSCRIBE_ALL);

/* Truepos message support added by Cyrill Stachniss, 10/9/2003 */

  carmen_simulator_subscribe_truepos_message(NULL, (carmen_handler_t)  carmen_simulator_truepos_handler,
                                             CARMEN_SUBSCRIBE_ALL);

// *** REI - START *** //

  carmen_laser_subscribe_frontlaser_remission_message(&frontlaser_remission, (carmen_handler_t)  laser_frontlaser_remission_handler,
							 CARMEN_SUBSCRIBE_ALL);
  carmen_laser_subscribe_rearlaser_remission_message(&rearlaser_remission, (carmen_handler_t)  laser_rearlaser_remission_handler,
							 CARMEN_SUBSCRIBE_ALL);
  carmen_laser_subscribe_laser3_remission_message(&laser3_remission, (carmen_handler_t)  laser_laser3_remission_handler,
							 CARMEN_SUBSCRIBE_ALL);
  carmen_laser_subscribe_laser4_remission_message(&laser4_remission, (carmen_handler_t)  laser_laser4_remission_handler,
							 CARMEN_SUBSCRIBE_ALL);
// *** REI - END *** //

  logger_starttime = carmen_get_time();


  signal(SIGINT, shutdown_module);
  carmen_ipc_dispatch();
  return 0;
}
