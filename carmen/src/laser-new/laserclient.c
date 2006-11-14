#include <carmen/carmen.h>
#include "laser_interface.h"
#include <stdio.h>

static void 
laser_handler(carmen_laser_laser_message *laser)
{
 fprintf(stderr, "n_ranges %d, n_remissions %d, timestamp %.3lf, host %s\n",
	laser->num_readings, laser->num_remissions, laser->timestamp, laser->host);
}


int 
main(int argc, char **argv)
{  
  carmen_ipc_initialize(argc, argv);
  carmen_param_check_version(argv[0]);
  static carmen_laser_laser_message laser;
  carmen_laser_subscribe_laser_message(0,&laser, (carmen_handler_t)
					      laser_handler,
					      CARMEN_SUBSCRIBE_LATEST);
  
  while(1) IPC_listen(10);
  return 0;
}


