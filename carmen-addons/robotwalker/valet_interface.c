
#include "valet_interface.h"


void carmen_valet_park() {

  carmen_valet_msg park_msg;
  IPC_RETURN_TYPE err;
  static int first = 1;

  if (first) {
    strcpy(path_msg.host, carmen_get_tenchar_host_name());
    first = 0;
  }

  path_msg.timestamp = carmen_get_time_ms();

  err = IPC_publishData(CARMEN_VALET_PARK_MSG_NAME, &park_msg);
  carmen_test_ipc_exit(err, "Could not publish", CARMEN_VALET_PARK_MSG_NAME);  
}

void carmen_valet_return() {

  carmen_valet_msg return_msg;
  IPC_RETURN_TYPE err;
  static int first = 1;

  if (first) {
    strcpy(path_msg.host, carmen_get_tenchar_host_name());
    first = 0;
  }

  path_msg.timestamp = carmen_get_time_ms();

  err = IPC_publishData(CARMEN_VALET_RETURN_MSG_NAME, &return_msg);
  carmen_test_ipc_exit(err, "Could not publish", CARMEN_VALET_RETURN_MSG_NAME);  
}
