#include <carmen/carmen.h>
#include "laser.h"
#include "laser_messages.h"
#include "sick.h"


int allocsize[4] = {0, 0, 0, 0};
float *range_buffer[4] = {NULL, NULL, NULL, NULL};

// *** REI - START *** //
int allocremsize[4] = {0, 0, 0, 0};
float *remission_buffer[4] = {NULL, NULL, NULL, NULL};
// *** REI - START *** //

void publish_laser_alive(int front_stalled, int rear_stalled,
			 int laser3_stalled, int laser4_stalled)
{
  IPC_RETURN_TYPE err;
  carmen_laser_alive_message msg;

  msg.frontlaser_stalled = front_stalled;
  msg.rearlaser_stalled = rear_stalled;
  msg.laser3_stalled = laser3_stalled;
  msg.laser4_stalled = laser4_stalled;

  err = IPC_publishData(CARMEN_LASER_ALIVE_NAME, &msg);
  carmen_test_ipc_exit(err, "Could not publish", CARMEN_LASER_ALIVE_NAME);
}

void publish_laser_message(sick_laser_p laser)
{
  static char *host = NULL;
  static carmen_laser_laser_message msg;

// *** REI - START *** //
  static carmen_laser_remission_message msg_rem;
// *** REI - END *** //

  IPC_RETURN_TYPE err;
  int i;

  if(host == NULL) {
    host = carmen_get_tenchar_host_name();
    strcpy(msg.host, host);
// *** REI - START *** //
	strcpy(msg_rem.host, host);
// *** REI - END *** //
  }
  msg.num_readings = laser->numvalues - 1;
  msg.timestamp = laser->timestamp;
  
  if(msg.num_readings != allocsize[laser->settings.laser_num]) {
    range_buffer[laser->settings.laser_num] = 
      realloc(range_buffer[laser->settings.laser_num],
	      msg.num_readings * sizeof(float));
    carmen_test_alloc(range_buffer[laser->settings.laser_num]);
    allocsize[laser->settings.laser_num] = msg.num_readings;
  }

  msg.range = range_buffer[laser->settings.laser_num];

  if( laser->settings.laser_flipped == 0)
    {
      for(i = 0; i < laser->numvalues - 1; i++)
	msg.range[i] = laser->range[i] / 100.0;
    }
  else
    {      
      for(i = 0; i < laser->numvalues - 1; i++)
	msg.range[i] = laser->range[laser->numvalues-2-i] / 100.0;
    }

// *** REI - START *** //
  msg_rem.num_readings = laser->numvalues - 1;
  msg_rem.timestamp = laser->timestamp;
  msg_rem.range = range_buffer[laser->settings.laser_num];

  if(msg_rem.num_readings != allocremsize[laser->settings.laser_num]) {
    remission_buffer[laser->settings.laser_num] = 
      realloc(remission_buffer[laser->settings.laser_num],
         msg_rem.num_readings * sizeof(float));
    carmen_test_alloc(remission_buffer[laser->settings.laser_num]);
    allocremsize[laser->settings.laser_num] = msg_rem.num_readings;
  }

  msg_rem.remission = remission_buffer[laser->settings.laser_num];
  
  if(laser->settings.use_remission == 1) {
    for(i = 0; i < laser->numvalues - 1; i++) msg_rem.remission[i] = laser->remission[i];
  }
  else for(i = 0; i < laser->numvalues - 1; i++) msg_rem.remission[i] = CARMEN_LASER_REMISSION_INVALID_VALUE;
  // *** REI - END *** //

  switch(laser->settings.laser_num) {
  case FRONT_LASER_NUM:
    err = IPC_publishData(CARMEN_LASER_FRONTLASER_NAME, &msg);
    carmen_test_ipc_exit(err, "Could not publish", 
			 CARMEN_LASER_FRONTLASER_NAME);
    // *** REI - START *** //
    if(laser->settings.use_remission == 1) {
      err = IPC_publishData(CARMEN_LASER_FRONTLASER_REMISSION_NAME, &msg_rem);
      carmen_test_ipc_exit(err, "Could not publish", 
			   CARMEN_LASER_FRONTLASER_REMISSION_NAME);
    }
    // *** REI - END *** //
    break;
  case REAR_LASER_NUM:
    err = IPC_publishData(CARMEN_LASER_REARLASER_NAME, &msg);
    carmen_test_ipc_exit(err, "Could not publish", 
			 CARMEN_LASER_REARLASER_NAME);
  
    // *** REI - START *** //
    if(laser->settings.use_remission == 1) {
      err = IPC_publishData(CARMEN_LASER_REARLASER_REMISSION_NAME, &msg_rem);
      carmen_test_ipc_exit(err, "Could not publish", 
			   CARMEN_LASER_REARLASER_REMISSION_NAME);
    }
    // *** REI - END *** //
    break;
  case LASER3_NUM:
    err = IPC_publishData(CARMEN_LASER_LASER3_NAME, &msg);
    carmen_test_ipc_exit(err, "Could not publish", 
			 CARMEN_LASER_LASER3_NAME);
    // *** REI - START *** //
    if(laser->settings.use_remission == 1) {
      err = IPC_publishData(CARMEN_LASER_LASER3_REMISSION_NAME, &msg_rem);
      carmen_test_ipc_exit(err, "Could not publish", 
			   CARMEN_LASER_LASER3_REMISSION_NAME);
    }
    // *** REI - END *** //
    break;
  case LASER4_NUM:
    err = IPC_publishData(CARMEN_LASER_LASER4_NAME, &msg);
    carmen_test_ipc_exit(err, "Could not publish", 
			 CARMEN_LASER_LASER4_NAME);
    // *** REI - START *** //
    if(laser->settings.use_remission == 1) {
      err = IPC_publishData(CARMEN_LASER_LASER4_REMISSION_NAME, &msg_rem);
      carmen_test_ipc_exit(err, "Could not publish", 
			   CARMEN_LASER_LASER4_REMISSION_NAME);
    }
    // *** REI - END *** //
    break;
}
}

void ipc_initialize_messages(void)
{
  IPC_RETURN_TYPE err;
  
  err = IPC_defineMsg(CARMEN_LASER_FRONTLASER_NAME, IPC_VARIABLE_LENGTH, 
		      CARMEN_LASER_FRONTLASER_FMT);
  carmen_test_ipc_exit(err, "Could not define", CARMEN_LASER_FRONTLASER_NAME);
  
  err = IPC_defineMsg(CARMEN_LASER_REARLASER_NAME, IPC_VARIABLE_LENGTH,
                      CARMEN_LASER_REARLASER_FMT);
  carmen_test_ipc_exit(err, "Could not define", CARMEN_LASER_REARLASER_NAME);

  err = IPC_defineMsg(CARMEN_LASER_LASER3_NAME, IPC_VARIABLE_LENGTH, 
		      CARMEN_LASER_LASER3_FMT);
  carmen_test_ipc_exit(err, "Could not define", CARMEN_LASER_FRONTLASER_NAME);
  
  err = IPC_defineMsg(CARMEN_LASER_LASER4_NAME, IPC_VARIABLE_LENGTH,
                      CARMEN_LASER_LASER4_FMT);
  carmen_test_ipc_exit(err, "Could not define", CARMEN_LASER_REARLASER_NAME);
  
  err = IPC_defineMsg(CARMEN_LASER_ALIVE_NAME, IPC_VARIABLE_LENGTH,
                      CARMEN_LASER_ALIVE_FMT);
  carmen_test_ipc_exit(err, "Could not define", CARMEN_LASER_ALIVE_NAME);

// *** REI - START *** //
  err = IPC_defineMsg(CARMEN_LASER_FRONTLASER_REMISSION_NAME, IPC_VARIABLE_LENGTH, 
		      CARMEN_LASER_FRONTLASER_REMISSION_FMT);
  carmen_test_ipc_exit(err, "Could not define", CARMEN_LASER_FRONTLASER_REMISSION_NAME);
  
  err = IPC_defineMsg(CARMEN_LASER_REARLASER_REMISSION_NAME, IPC_VARIABLE_LENGTH,
                      CARMEN_LASER_REARLASER_REMISSION_FMT);
  carmen_test_ipc_exit(err, "Could not define", CARMEN_LASER_REARLASER_REMISSION_NAME);

  err = IPC_defineMsg(CARMEN_LASER_LASER3_REMISSION_NAME, IPC_VARIABLE_LENGTH, 
		      CARMEN_LASER_LASER3_REMISSION_FMT);
  carmen_test_ipc_exit(err, "Could not define", CARMEN_LASER_FRONTLASER_REMISSION_NAME);
  
  err = IPC_defineMsg(CARMEN_LASER_LASER4_REMISSION_NAME, IPC_VARIABLE_LENGTH,
                      CARMEN_LASER_LASER4_REMISSION_FMT);
  carmen_test_ipc_exit(err, "Could not define", CARMEN_LASER_REARLASER_REMISSION_NAME);
// *** REI - END *** //
}
