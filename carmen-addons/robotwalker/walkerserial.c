#include <carmen/carmen.h>
#include "walkerserial_interface.h"


static void publish_button_msg(int button) {

  static carmen_walkerserial_button_msg button_msg;
  static int first = 1;
  IPC_RETURN_TYPE err;
  
  if (first) {
    strcpy(button_msg.host, carmen_get_tenchar_host_name());
    first = 0;
  }

  button_msg.timestamp = carmen_get_time_ms();
  button_msg.button = button;

  err = IPC_publishData(CARMEN_WALKERSERIAL_BUTTON_MSG_NAME,
			&button_msg);
  carmen_test_ipc_exit(err, "Could not publish",
		       CARMEN_WALKERSERIAL_BUTTON_MSG_NAME);  
}
