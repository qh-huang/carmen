#include <carmen/carmen.h>

#include "segway_messages.h"

static carmen_segway_pose_message *segway_pose_pointer_external = NULL;
static carmen_handler_t segway_pose_handler_external = NULL;

static void 
segway_pose_interface_handler(MSG_INSTANCE msgRef, BYTE_ARRAY callData,
			      void *clientData __attribute__ ((unused)))
{
  IPC_RETURN_TYPE err = IPC_OK;
  
  if(segway_pose_pointer_external)
    err = IPC_unmarshallData(IPC_msgInstanceFormatter(msgRef), callData, 
			     segway_pose_pointer_external,
			     sizeof(carmen_segway_pose_message));
  IPC_freeByteArray(callData);
  carmen_test_ipc_return(err, "Could not unmarshall data", 
                         IPC_msgInstanceName(msgRef));
  if(segway_pose_handler_external)
    segway_pose_handler_external(segway_pose_pointer_external);
}

void
carmen_segway_subscribe_pose_message(carmen_segway_pose_message *pose,
				     carmen_handler_t handler,
				     carmen_subscribe_t subscribe_how)
{
  IPC_RETURN_TYPE err = IPC_OK;

  err = IPC_defineMsg(CARMEN_SEGWAY_POSE_NAME, IPC_VARIABLE_LENGTH, 
                      CARMEN_SEGWAY_POSE_FMT);
  carmen_test_ipc_exit(err, "Could not define message", 
                       CARMEN_SEGWAY_POSE_NAME);
  if(subscribe_how == CARMEN_UNSUBSCRIBE) {
    IPC_unsubscribe(CARMEN_SEGWAY_POSE_NAME, segway_pose_interface_handler);
    return;
  }

  if(pose)
    segway_pose_pointer_external = pose;
  else if(segway_pose_pointer_external == NULL) {                   
    segway_pose_pointer_external =
      (carmen_segway_pose_message *)
      calloc(1, sizeof(carmen_segway_pose_message));
    carmen_test_alloc(segway_pose_pointer_external);
  }
  segway_pose_handler_external = handler;
  err = IPC_subscribe(CARMEN_SEGWAY_POSE_NAME, 
		      segway_pose_interface_handler, NULL);
  if(subscribe_how == CARMEN_SUBSCRIBE_LATEST)
    IPC_setMsgQueueLength(CARMEN_SEGWAY_POSE_NAME, 1);
  else
    IPC_setMsgQueueLength(CARMEN_SEGWAY_POSE_NAME, 100);
  carmen_test_ipc(err, "Could not subscribe", CARMEN_SEGWAY_POSE_NAME);
}


