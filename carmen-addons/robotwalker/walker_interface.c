#include <carmen/carmen.h>
#include "walker_interface.h"


static carmen_walker_goal_changed_msg *goal_changed_msg_ptr_ext = NULL;
static carmen_handler_t goal_changed_msg_handler_ext = NULL;


void carmen_walker_set_goal(int goal) {

  IPC_RETURN_TYPE err = IPC_OK;
  carmen_walker_set_goal_msg msg;

  msg.timestamp = carmen_get_time_ms();
  strcpy(msg.host, carmen_get_tenchar_host_name());

  msg.goal = goal;

  err = IPC_publishData(CARMEN_WALKER_SET_GOAL_MSG_NAME, &msg);
  carmen_test_ipc(err, "Could not publish", CARMEN_WALKER_SET_GOAL_MSG_NAME);
}

static void goal_changed_msg_handler(MSG_INSTANCE msgRef, BYTE_ARRAY callData,
				     void *clientData __attribute__ ((unused))) {
  
  FORMATTER_PTR formatter;
  IPC_RETURN_TYPE err = IPC_OK;

  formatter = IPC_msgInstanceFormatter(msgRef);

  if (goal_changed_msg_ptr_ext)
    err = IPC_unmarshallData(formatter, callData, goal_changed_msg_ptr_ext,
                             sizeof(carmen_walker_goal_changed_msg));
  IPC_freeByteArray(callData);
  carmen_test_ipc_return(err, "Could not unmarshall", IPC_msgInstanceName(msgRef));

  if (goal_changed_msg_handler_ext)
    goal_changed_msg_handler_ext(goal_changed_msg_ptr_ext);
}

void carmen_walker_subscribe_goal_changed_message
(carmen_walker_goal_changed_msg *msg, carmen_handler_t handler,
 carmen_subscribe_t subscribe_how) {

  IPC_RETURN_TYPE err = IPC_OK;  

  if(subscribe_how == CARMEN_UNSUBSCRIBE) {
    IPC_unsubscribe(CARMEN_WALKER_GOAL_CHANGED_MSG_NAME, 
		    goal_changed_msg_handler);
    return;
  }

  if(msg)
    goal_changed_msg_ptr_ext = msg;
  else {
    goal_changed_msg_ptr_ext = (carmen_walker_goal_changed_msg *)
      calloc(1, sizeof(carmen_walker_goal_changed_msg));
    carmen_test_alloc(goal_changed_msg_ptr_ext);
  }

  goal_changed_msg_handler_ext = handler;
  err = IPC_subscribe(CARMEN_WALKER_GOAL_CHANGED_MSG_NAME, 
		      goal_changed_msg_handler, NULL);

  if(subscribe_how == CARMEN_SUBSCRIBE_LATEST)
    IPC_setMsgQueueLength(CARMEN_WALKER_GOAL_CHANGED_MSG_NAME, 1);
  else
    IPC_setMsgQueueLength(CARMEN_WALKER_GOAL_CHANGED_MSG_NAME, 100);

  carmen_test_ipc(err, "Could not subscribe", CARMEN_WALKER_GOAL_CHANGED_MSG_NAME);
}
