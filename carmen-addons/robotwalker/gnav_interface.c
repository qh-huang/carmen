
#include <carmen/carmen.h>
#include "gnav_interface.h"


static carmen_gnav_room_msg *room_msg_ptr_ext = NULL;
static carmen_handler_t room_msg_handler_ext = NULL;
static carmen_gnav_rooms_msg *rooms_msg_ptr_ext = NULL;
static carmen_handler_t rooms_msg_handler_ext = NULL;
static carmen_gnav_path_msg *path_msg_ptr_ext = NULL;
static carmen_handler_t path_msg_handler_ext = NULL;


static void room_msg_handler(MSG_INSTANCE msgRef, BYTE_ARRAY callData,
			     void *clientData __attribute__ ((unused))) {
  
  FORMATTER_PTR formatter;
  IPC_RETURN_TYPE err = IPC_OK;

  formatter = IPC_msInstanceFormatter(msgRef);

  if (room_msg_ptr_ext)
    err = IPC_unmarshallData(formatter, callData, room_msg_ptr_ext,
                             sizeof(carmen_gnav_room_msg));
  IPC_freeByteArray(callData);
  carmen_test_ipc_return(err, "Could not unmarshall", IPC_msgInstanceName(msgRef));

  if (room_msg_handler_ext)
    room_msg_handler_ext(room_msg_ptr_ext);
}

void carmen_gnav_subscribe_room_message(carmen_gnav_room_msg *room_msg,
					carmen_handler_t handler,
					carmen_subscribe_t subscribe_how) {

  IPC_RETURN_TYPE err = IPC_OK;  

  if(subscribe_how == CARMEN_UNSUBSCRIBE) {
    IPC_unsubscribe(CARMEN_GNAV_ROOM_NAME, 
		    room_msg_handler);
    return;
  }

  if(room_msg)
    room_msg_ptr_ext = room_msg;
  else {
    room_msg_ptr_ext = (carmen_gnav_room_msg *)
      calloc(1, sizeof(carmen_gnav_room_msg));
    carmen_test_alloc(room_msg_ptr_ext);
  }

  room_msg_handler_ext = handler;
  err = IPC_subscribe(CARMEN_GNAV_ROOM_NAME, 
		      room_msg_handler, NULL);

  if(subscribe_how == CARMEN_SUBSCRIBE_LATEST)
    IPC_setMsgQueueLength(CARMEN_GNAV_ROOM_NAME, 1);
  else
    IPC_setMsgQueueLength(CARMEN_GNAV_ROOM_NAME, 100);

  carmen_test_ipc(err, "Could not subscribe", CARMEN_GNAV_ROOM_NAME);
}

carmen_rooms_topology_p carmen_gnav_get_rooms_topology() {

  return;
}

static void path_msg_handler(MSG_INSTANCE msgRef, BYTE_ARRAY callData,
			     void *clientData __attribute__ ((unused))) {
  
  FORMATTER_PTR formatter;
  IPC_RETURN_TYPE err = IPC_OK;

  formatter = IPC_msInstanceFormatter(msgRef);

  if (path_msg_ptr_ext)
    err = IPC_unmarshallData(formatter, callData, path_msg_ptr_ext,
                             sizeof(carmen_gnav_path_msg));
  IPC_freeByteArray(callData);
  carmen_test_ipc_return(err, "Could not unmarshall", IPC_msgInstanceName(msgRef));

  if (path_msg_handler_ext)
    path_msg_handler_ext(path_msg_ptr_ext);
}

void carmen_gnav_subscribe_path_message(carmen_gnav_path_msg *path_msg,
					carmen_handler_t handler,
					carmen_subscribe_t subscribe_how) {

  IPC_RETURN_TYPE err = IPC_OK;

  if(subscribe_how == CARMEN_UNSUBSCRIBE) {
    IPC_unsubscribe(CARMEN_GNAV_PATH_NAME, 
		    path_msg_handler);
    return;
  }

  if(path_msg)
    path_msg_ptr_ext = path_msg;
  else {
    path_msg_ptr_ext = (carmen_gnav_path_msg *)
      calloc(1, sizeof(carmen_gnav_path_msg));
    carmen_test_alloc(path_msg_ptr_ext);
  }

  path_msg_handler_ext = handler;
  err = IPC_subscribe(CARMEN_GNAV_PATH_NAME, 
		      path_msg_handler, NULL);

  if(subscribe_how == CARMEN_SUBSCRIBE_LATEST)
    IPC_setMsgQueueLength(CARMEN_GNAV_PATH_NAME, 1);
  else
    IPC_setMsgQueueLength(CARMEN_GNAV_PATH_NAME, 100);

  carmen_test_ipc(err, "Could not subscribe", CARMEN_GNAV_PATH_NAME);
}

void carmen_gnav_set_goal(int room) {

  return;
}
