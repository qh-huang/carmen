#include <carmen/carmen.h>
#include "walker_messages.h"
#include "gnav_interface.h"


static void set_goal_handler(MSG_INSTANCE msgRef, BYTE_ARRAY callData,
			     void *clientData __attribute__ ((unused))) {

  carmen_walker_set_goal_msg goal_msg;
  FORMATTER_PTR formatter;
  IPC_RETURN_TYPE err = IPC_OK;

  formatter = IPC_msgInstanceFormatter(msgRef);
  err = IPC_unmarshallData(formatter, callData, &goal_msg,
			   sizeof(carmen_walker_set_goal_msg));
  IPC_freeByteArray(callData);
  carmen_test_ipc_return(err, "Could not unmarshall", IPC_msgInstanceName(msgRef));

  carmen_verbose("goal = %d\n", goal_msg.goal);

  carmen_gnav_set_goal(goal_msg.goal);
}

static void ipc_init() {

  IPC_RETURN_TYPE err;

  err = IPC_defineMsg(CARMEN_WALKER_SET_GOAL_MSG_NAME, IPC_VARIABLE_LENGTH, 
		      CARMEN_WALKER_SET_GOAL_MSG_FMT);
  carmen_test_ipc_exit(err, "Could not define", CARMEN_WALKER_SET_GOAL_MSG_NAME);

  err = IPC_subscribe(CARMEN_WALKER_SET_GOAL_MSG_NAME, 
		      set_goal_handler, NULL);
  carmen_test_ipc_exit(err, "Could not subcribe", 
		       CARMEN_WALKER_SET_GOAL_MSG_NAME);
  IPC_setMsgQueueLength(CARMEN_WALKER_SET_GOAL_MSG_NAME, 100);
}

int main(int argc __attribute__ ((unused)), char *argv[]) {

  carmen_initialize_ipc(argv[0]);
  carmen_param_check_version(argv[0]);

  ipc_init();

  IPC_dispatch();

  return 0;
}
