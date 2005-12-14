#include <carmen/carmen.h>
#include "laser.h"
#include "laser_messages.h"
#include "sick.h"


int allocsize[4] = {0, 0, 0, 0};
float *range_buffer[4] = {NULL, NULL, NULL, NULL};

int allocremsize[4] = {0, 0, 0, 0};
float *remission_buffer[4] = {NULL, NULL, NULL, NULL};


void publish_laser_alive(int front_stalled, int rear_stalled,
			 int laser3_stalled, int laser4_stalled)
{
  IPC_RETURN_TYPE err;
  carmen_laser_alive_message msg;

  msg.frontlaser_stalled = front_stalled;
  msg.rearlaser_stalled  = rear_stalled;
  msg.laser3_stalled     = laser3_stalled;
  msg.laser4_stalled     = laser4_stalled;

  err = IPC_publishData(CARMEN_LASER_ALIVE_NAME, &msg);
  carmen_test_ipc_exit(err, "Could not publish", CARMEN_LASER_ALIVE_NAME);
}

void publish_laser_message(sick_laser_p laser,
			   const carmen_laser_laser_config_t* config)
{
  static carmen_laser_laser_message msg;

  IPC_RETURN_TYPE err;
  int i;

  msg.host = carmen_get_host();
  msg.num_readings = laser->numvalues - 1; 
  msg.timestamp = laser->timestamp;
  msg.config = *config;
  
  if(msg.num_readings != allocsize[laser->settings.laser_num]) {
    range_buffer[laser->settings.laser_num] = 
      realloc(range_buffer[laser->settings.laser_num],
	      msg.num_readings * sizeof(float));
    carmen_test_alloc(range_buffer[laser->settings.laser_num]);
    allocsize[laser->settings.laser_num] = msg.num_readings;
  }
  msg.range = range_buffer[laser->settings.laser_num];

  if( laser->settings.laser_flipped == 0)  {
    for(i = 0; i < msg.num_readings; i++)
      msg.range[i] = laser->range[i] / 100.0;
  }
  else {      
    for(i = 0; i < msg.num_readings; i++)
      msg.range[i] = laser->range[msg.num_readings-1-i] / 100.0;
  }

  if(laser->settings.use_remission == 1) {

    if(msg.num_remission != allocremsize[laser->settings.laser_num]) {
      remission_buffer[laser->settings.laser_num] = 
	realloc(remission_buffer[laser->settings.laser_num],
		msg.num_readings * sizeof(float));
      carmen_test_alloc(remission_buffer[laser->settings.laser_num]);
      allocremsize[laser->settings.laser_num] = msg.num_readings;
    }
    msg.remission = remission_buffer[laser->settings.laser_num];
    
    
    msg.num_remission = msg.num_readings;
    if( laser->settings.laser_flipped == 0)  {
      for(i = 0; i < msg.num_remission; i++) 
	msg.remission[i] = laser->remission[i];
    }
    else {
      for(i = 0; i < msg.num_remission; i++)
	msg.remission[i] = laser->remission[msg.num_remission-1-i];
    }      
  }
  else {
    msg.num_remission = 0;
    msg.remission = NULL ;
  }

  switch(laser->settings.laser_num) {
  case CARMEN_FRONT_LASER_NUM:
    err = IPC_publishData(CARMEN_LASER_FRONTLASER_NAME, &msg);
    carmen_test_ipc_exit(err, "Could not publish", 
			 CARMEN_LASER_FRONTLASER_NAME);
    break;
  case CARMEN_REAR_LASER_NUM:
    err = IPC_publishData(CARMEN_LASER_REARLASER_NAME, &msg);
    carmen_test_ipc_exit(err, "Could not publish", 
			 CARMEN_LASER_REARLASER_NAME);
    break;
  case CARMEN_LASER3_NUM:
    err = IPC_publishData(CARMEN_LASER_LASER3_NAME, &msg);
    carmen_test_ipc_exit(err, "Could not publish", 
			 CARMEN_LASER_LASER3_NAME);
    break;
  case CARMEN_LASER4_NUM:
    err = IPC_publishData(CARMEN_LASER_LASER4_NAME, &msg);
    carmen_test_ipc_exit(err, "Could not publish", 
			 CARMEN_LASER_LASER4_NAME);
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
}
