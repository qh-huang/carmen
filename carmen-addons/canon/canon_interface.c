#include <carmen/carmen.h>
#include "canon_messages.h"

#include "canon_interface.h"

static unsigned int timeout = 10000;

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
