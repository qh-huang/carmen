#include <carmen/carmen.h>
#include "canon_messages.h"
#include "canon.h"

usb_dev_handle *camera_handle;

void shutdown_module(int x)
{
  if(x == SIGINT) {
    canon_stop_capture(camera_handle);
    canon_close_camera(camera_handle);
    close_ipc();
    fprintf(stderr, "Closed connection to camera.\n");
    exit(-1);
  }
}

void canon_image_query(MSG_INSTANCE msgRef, BYTE_ARRAY callData,
		       void *clientData __attribute__ ((unused)))
{
  FORMATTER_PTR formatter;
  IPC_RETURN_TYPE err = IPC_OK;
  carmen_canon_image_message response;
  carmen_canon_image_request query;

  formatter = IPC_msgInstanceFormatter(msgRef);
  err = IPC_unmarshallData(formatter, callData, &query, 
                           sizeof(carmen_canon_image_request));
  IPC_freeByteArray(callData);
  carmen_test_ipc(err, "Could not unmarshall", IPC_msgInstanceName(msgRef));  

  fprintf(stderr, "Received query image %d thumbnail %d\n", query.get_image,
	  query.get_thumbnail);

  if(canon_capture_image(camera_handle,
			 (unsigned char **)&response.thumbnail, 
			 &response.thumbnail_length,
			 (unsigned char **)&response.image, 
			 &response.image_length) < 0)
    carmen_warn("Warning: Could not capture image.\n");

  fprintf(stderr, "DONE CAPTURING IMAGE.\n");
  
  response.timestamp = carmen_get_time_ms();
  strcpy(response.host, carmen_get_tenchar_host_name());
  
  err = IPC_respondData(msgRef, CARMEN_CANON_IMAGE_NAME, &response);
  carmen_test_ipc(err, "Could not respond", CARMEN_CANON_IMAGE_NAME);
  
  if(response.image != NULL)
    free(response.image);
  if(response.thumbnail != NULL)
    free(response.thumbnail);
}

void initialize_ipc_messages(void)
{
  IPC_RETURN_TYPE err;
  
  /* register image request message */
  err = IPC_defineMsg(CARMEN_CANON_IMAGE_REQUEST_NAME, IPC_VARIABLE_LENGTH, 
                      CARMEN_CANON_IMAGE_REQUEST_FMT);
  carmen_test_ipc_exit(err, "Could not define", 
		       CARMEN_CANON_IMAGE_REQUEST_NAME);
  
  /* register image message */
  err = IPC_defineMsg(CARMEN_CANON_IMAGE_NAME, IPC_VARIABLE_LENGTH, 
                      CARMEN_CANON_IMAGE_FMT);
  carmen_test_ipc_exit(err, "Could not define", CARMEN_CANON_IMAGE_NAME);

  /* subscribe to image requests */
  err = IPC_subscribe(CARMEN_CANON_IMAGE_REQUEST_NAME, 
		      canon_image_query, NULL);
  carmen_test_ipc_exit(err, "Could not subscribe to", 
                       CARMEN_CANON_IMAGE_REQUEST_NAME);
  IPC_setMsgQueueLength(CARMEN_CANON_IMAGE_REQUEST_NAME, 100);
}

int main(int argc __attribute__ ((unused)), char **argv)
{
  /* initialize IPC */
  carmen_initialize_ipc(argv[0]);
  carmen_param_check_version(argv[0]);
  initialize_ipc_messages();
  signal(SIGINT, shutdown_module);

  /* initialize the digital camera */
  camera_handle = canon_open_camera();
  if(camera_handle == NULL)
    carmen_die("Erorr: could not open connection to camera.\n");
  if(canon_initialize_camera(camera_handle) < 0)
    carmen_die("Erorr: could not open initialize camera.\n");
  sleep(1);
  if(canon_initialize_capture(camera_handle, THUMB_TO_PC | FULL_TO_PC) < 0)
    carmen_die("Error: could not start image capture.\n");

  fprintf(stderr, "READY TO CAPTURE\n");
  /* run the main loop */
  IPC_dispatch();
  return 0;
}
