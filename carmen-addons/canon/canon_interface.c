#include <carmen/carmen.h>
#include "canon_messages.h"

static unsigned int timeout = 10000;

int carmen_canon_get_image(carmen_canon_image_message **response)
{
  IPC_RETURN_TYPE err;
  carmen_canon_image_request query;
  
  if(*response != NULL) {
    if((*response)->thumbnail != NULL)
      free((*response)->thumbnail);
    if((*response)->image != NULL)
      free((*response)->image);
  }
  query.timestamp = carmen_get_time_ms();
  strcpy(query.host, carmen_get_tenchar_host_name());
  query.get_image = 1;
  query.get_thumbnail = 1;
  err = IPC_queryResponseData(CARMEN_CANON_IMAGE_REQUEST_NAME, &query,
                              (void **)response, timeout);
  carmen_test_ipc(err, "Could not query image",
		  CARMEN_CANON_IMAGE_REQUEST_NAME);
  if(IPC_Error || err == IPC_Timeout) {
    *response = NULL;
    return -1;
  }
  return 0;
}
