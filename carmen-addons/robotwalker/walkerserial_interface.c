
#include <carmen/carmen.h>
#include "walkerserial_interface.h"


static carmen_walkerserial_button_msg *button_msg_ptr_ext = NULL;
static carmen_handler_t button_msg_handler_ext = NULL;
static carmen_walkerserial_clutch_msg *clutch_msg_ptr_ext = NULL;
static carmen_handler_t clutch_msg_handler_ext = NULL;


static void button_msg_handler(MSG_INSTANCE msgRef, BYTE_ARRAY callData,
			       void *clientData __attribute__ ((unused))) {
  
  FORMATTER_PTR formatter;
  IPC_RETURN_TYPE err = IPC_OK;

  formatter = IPC_msgInstanceFormatter(msgRef);

  if (button_msg_ptr_ext)
    err = IPC_unmarshallData(formatter, callData, button_msg_ptr_ext,
                             sizeof(carmen_walkerserial_button_msg));
  IPC_freeByteArray(callData);
  carmen_test_ipc_return(err, "Could not unmarshall",
			 IPC_msgInstanceName(msgRef));

  if (button_msg_handler_ext)
    button_msg_handler_ext(button_msg_ptr_ext);
}

void carmen_walkerserial_subscribe_button_message
(carmen_walkerserial_button_msg *button_msg,
 carmen_handler_t handler,
 carmen_subscribe_t subscribe_how) {

  IPC_RETURN_TYPE err = IPC_OK;  

  if(subscribe_how == CARMEN_UNSUBSCRIBE) {
    IPC_unsubscribe(CARMEN_WALKERSERIAL_BUTTON_MSG_NAME, 
		    button_msg_handler);
    return;
  }

  if(button_msg)
    button_msg_ptr_ext = button_msg;
  else {
    button_msg_ptr_ext = (carmen_walkerserial_button_msg *)
      calloc(1, sizeof(carmen_walkerserial_button_msg));
    carmen_test_alloc(button_msg_ptr_ext);
  }

  button_msg_handler_ext = handler;
  err = IPC_subscribe(CARMEN_WALKERSERIAL_BUTTON_MSG_NAME, 
		      button_msg_handler, NULL);

  if(subscribe_how == CARMEN_SUBSCRIBE_LATEST)
    IPC_setMsgQueueLength(CARMEN_WALKERSERIAL_BUTTON_MSG_NAME, 1);
  else
    IPC_setMsgQueueLength(CARMEN_WALKERSERIAL_BUTTON_MSG_NAME, 100);

  carmen_test_ipc(err, "Could not subscribe",
		  CARMEN_WALKERSERIAL_BUTTON_MSG_NAME);
}

static void clutch_msg_handler(MSG_INSTANCE msgRef, BYTE_ARRAY callData,
			       void *clientData __attribute__ ((unused))) {
  
  FORMATTER_PTR formatter;
  IPC_RETURN_TYPE err = IPC_OK;

  formatter = IPC_msgInstanceFormatter(msgRef);

  if (clutch_msg_ptr_ext)
    err = IPC_unmarshallData(formatter, callData, clutch_msg_ptr_ext,
                             sizeof(carmen_walkerserial_clutch_msg));
  IPC_freeByteArray(callData);
  carmen_test_ipc_return(err, "Could not unmarshall",
			 IPC_msgInstanceName(msgRef));

  if (clutch_msg_handler_ext)
    clutch_msg_handler_ext(clutch_msg_ptr_ext);
}

void carmen_walkerserial_subscribe_clutch_message
(carmen_walkerserial_clutch_msg *clutch_msg,
 carmen_handler_t handler,
 carmen_subscribe_t subscribe_how) {

  IPC_RETURN_TYPE err = IPC_OK;  

  if(subscribe_how == CARMEN_UNSUBSCRIBE) {
    IPC_unsubscribe(CARMEN_WALKERSERIAL_CLUTCH_MSG_NAME, 
		    clutch_msg_handler);
    return;
  }

  if(clutch_msg)
    clutch_msg_ptr_ext = clutch_msg;
  else {
    clutch_msg_ptr_ext = (carmen_walkerserial_clutch_msg *)
      calloc(1, sizeof(carmen_walkerserial_clutch_msg));
    carmen_test_alloc(clutch_msg_ptr_ext);
  }

  clutch_msg_handler_ext = handler;
  err = IPC_subscribe(CARMEN_WALKERSERIAL_CLUTCH_MSG_NAME, 
		      clutch_msg_handler, NULL);

  if(subscribe_how == CARMEN_SUBSCRIBE_LATEST)
    IPC_setMsgQueueLength(CARMEN_WALKERSERIAL_CLUTCH_MSG_NAME, 1);
  else
    IPC_setMsgQueueLength(CARMEN_WALKERSERIAL_CLUTCH_MSG_NAME, 100);

  carmen_test_ipc(err, "Could not subscribe",
		  CARMEN_WALKERSERIAL_CLUTCH_MSG_NAME);
}

static void publish_clutch_message(int action) {

  static carmen_walkerserial_clutch_msg clutch_msg;
  static int first = 1;
  IPC_RETURN_TYPE err;
  
  printf("publish_clutch_msg(%d)\n", action);

  if (first) {
    strcpy(clutch_msg.host, carmen_get_tenchar_host_name());
    first = 0;
  }

  clutch_msg.timestamp = carmen_get_time_ms();
  clutch_msg.action = action;

  err = IPC_publishData(CARMEN_WALKERSERIAL_CLUTCH_MSG_NAME,
			&clutch_msg);
  carmen_test_ipc_exit(err, "Could not publish",
		       CARMEN_WALKERSERIAL_CLUTCH_MSG_NAME);
}

void carmen_walkerserial_engage_clutch() {

  publish_clutch_message(CARMEN_WALKERSERIAL_CLUTCH_ENGAGE);
}

void carmen_walkerserial_disengage_clutch() {

  publish_clutch_message(CARMEN_WALKERSERIAL_CLUTCH_DISENGAGE);
}
