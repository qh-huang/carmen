#include <carmen/carmen.h>
#include "walker_interface.h"


void carmen_walker_set_goal(int goal) {

  IPC_RETURN_TYPE err = IPC_OK;
  carmen_walker_set_goal_msg msg;

  msg.timestamp = carmen_get_time_ms();
  strcpy(msg.host, carmen_get_tenchar_host_name());

  msg.goal = goal;

  err = IPC_publishData(CARMEN_WALKER_SET_GOAL_MSG_NAME, &msg);
  carmen_test_ipc(err, "Could not publish", CARMEN_WALKER_SET_GOAL_MSG_NAME);
}
