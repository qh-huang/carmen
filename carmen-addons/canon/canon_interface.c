#include <carmen/carmen.h>
#include "canon_messages.h"

#include "canon_interface.h"

static unsigned int timeout = 15000;

static carmen_canon_preview_message *preview_message_pointer_external = NULL;
static carmen_handler_t preview_message_handler_external = NULL;

carmen_canon_image_message *carmen_canon_get_image(int thumbnail_over_ipc,
						   int image_over_ipc,
						   int image_to_drive)
{
  IPC_RETURN_TYPE err;
  carmen_canon_image_message *response;
  carmen_canon_image_request query;
  
  if(!thumbnail_over_ipc && !image_over_ipc && !image_to_drive) {
    carmen_warn("Error: need to set at least one location for the image.\n");
    return NULL;
  }
  query.timestamp = carmen_get_time_ms();
  strcpy(query.host, carmen_get_tenchar_host_name());
  query.thumbnail_over_ipc = thumbnail_over_ipc;
  query.image_over_ipc = image_over_ipc;
  query.image_to_drive = image_to_drive;
  err = IPC_queryResponseData(CARMEN_CANON_IMAGE_REQUEST_NAME, &query,
                              (void **)&response, timeout);
  carmen_test_ipc(err, "Could not query image",
		  CARMEN_CANON_IMAGE_REQUEST_NAME);
  if(IPC_Error || err == IPC_Timeout)
    return NULL;
  return response;
}

void carmen_canon_free_image(carmen_canon_image_message **message)
{
  if(message != NULL) 
    if(*message != NULL) {
      if((*message)->thumbnail != NULL)
	free((*message)->thumbnail);
      if((*message)->image != NULL)
	free((*message)->image);
      free(*message);
      *message = NULL;
    }
}

static void preview_interface_handler(MSG_INSTANCE msgRef, BYTE_ARRAY callData,
				      void *clientData __attribute__ ((unused)))
{
  IPC_RETURN_TYPE err = IPC_OK;
  FORMATTER_PTR formatter;
  
  formatter = IPC_msgInstanceFormatter(msgRef);
  if(preview_message_pointer_external) {
    if(preview_message_pointer_external->preview != NULL)
      free(preview_message_pointer_external->preview);
    
    err = IPC_unmarshallData(formatter, callData, 
                             preview_message_pointer_external,
                             sizeof(carmen_canon_preview_message));
  }
  IPC_freeByteArray(callData);
  carmen_test_ipc_return(err, "Could not unmarshall", 
                         IPC_msgInstanceName(msgRef));
  if(preview_message_handler_external)
    preview_message_handler_external(preview_message_pointer_external);
}

void
carmen_canon_subscribe_preview_message(carmen_canon_preview_message *preview,
				       carmen_handler_t handler,
				       carmen_subscribe_t subscribe_how)
{
  IPC_RETURN_TYPE err = IPC_OK;
  
  if(subscribe_how == CARMEN_UNSUBSCRIBE) {
    IPC_unsubscribe(CARMEN_CANON_PREVIEW_NAME, preview_interface_handler);
    return;
  }
  if(preview) {
    preview_message_pointer_external = preview;
    memset(preview_message_pointer_external, 0, 
           sizeof(carmen_canon_preview_message));
  }
  else if (preview_message_pointer_external == NULL) {
    preview_message_pointer_external = (carmen_canon_preview_message *)
      calloc(1, sizeof(carmen_canon_preview_message));
    carmen_test_alloc(preview_message_pointer_external);
  }
  
  preview_message_handler_external = handler;
  err = IPC_subscribe(CARMEN_CANON_PREVIEW_NAME, 
                      preview_interface_handler, NULL);
  if (subscribe_how == CARMEN_SUBSCRIBE_LATEST)
    IPC_setMsgQueueLength(CARMEN_CANON_PREVIEW_NAME, 1);
  else
    IPC_setMsgQueueLength(CARMEN_CANON_PREVIEW_NAME, 100);
  carmen_test_ipc(err, "Could not subscribe", CARMEN_CANON_PREVIEW_NAME);
}
