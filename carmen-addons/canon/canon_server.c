#include <carmen/carmen.h>
#include "canon_messages.h"
#include "canon.h"

usb_dev_handle *camera_handle;
int use_flash;

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
  int transfer_mode;

  formatter = IPC_msgInstanceFormatter(msgRef);
  err = IPC_unmarshallData(formatter, callData, &query, 
                           sizeof(carmen_canon_image_request));
  IPC_freeByteArray(callData);
  carmen_test_ipc(err, "Could not unmarshall", IPC_msgInstanceName(msgRef));  

  transfer_mode = 0;
  if(query.thumbnail_over_ipc)
    transfer_mode |= THUMB_TO_PC;
  if(query.image_over_ipc)
    transfer_mode |= FULL_TO_PC;
  if(query.image_to_drive)
    transfer_mode |= FULL_TO_DRIVE;
  
  if(canon_rcc_set_flash(camera_handle, query.flash_mode) < 0)
    carmen_warn("Warning: Could not set flash mode.\n");
  if(canon_rcc_set_transfer_mode(camera_handle, transfer_mode) < 0)
    carmen_warn("Warning: Could not set transfer mode.\n");
  
  carmen_time_code(
  fprintf(stderr, "Starting capture... ");
  if(canon_capture_image(camera_handle,
			 (unsigned char **)&response.thumbnail, 
			 &response.thumbnail_length,
			 (unsigned char **)&response.image, 
			 &response.image_length) < 0)
    fprintf(stderr, "error.\n");
  else
    fprintf(stderr, "done.\n");
  , "CAPTURE");
  response.timestamp = carmen_get_time_ms();
  strcpy(response.host, carmen_get_tenchar_host_name());
  
  carmen_time_code(
  err = IPC_respondData(msgRef, CARMEN_CANON_IMAGE_NAME, &response);
  carmen_test_ipc(err, "Could not respond", CARMEN_CANON_IMAGE_NAME);
  ,"TRANSMIT");
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

  /* register preview message */
  err = IPC_defineMsg(CARMEN_CANON_PREVIEW_NAME, IPC_VARIABLE_LENGTH, 
                      CARMEN_CANON_PREVIEW_FMT);
  carmen_test_ipc_exit(err, "Could not define", CARMEN_CANON_PREVIEW_NAME);

  /* subscribe to image requests */
  err = IPC_subscribe(CARMEN_CANON_IMAGE_REQUEST_NAME, 
		      canon_image_query, NULL);
  carmen_test_ipc_exit(err, "Could not subscribe to", 
                       CARMEN_CANON_IMAGE_REQUEST_NAME);
  IPC_setMsgQueueLength(CARMEN_CANON_IMAGE_REQUEST_NAME, 100);
}

void read_parameters(int argc, char **argv)
{
  carmen_param_t camera_params[] = {
    {"canon", "flash", CARMEN_PARAM_ONOFF, &use_flash, 0, NULL}
  };

  carmen_param_install_params(argc, argv, camera_params,
                              sizeof(camera_params) / 
			      sizeof(camera_params[0]));
}

void publish_preview(void *clientdata __attribute__ ((unused)), 
		     unsigned long t1 __attribute__ ((unused)), 
		     unsigned long t2 __attribute__ ((unused)))
{
  carmen_canon_preview_message preview;
  IPC_RETURN_TYPE err;

  if(canon_rcc_download_preview(camera_handle,
				(unsigned char **)&preview.preview, 
				&preview.preview_length) < 0) {
    fprintf(stderr, "Error: could not download a preview.\n");
    return;
  }
  fprintf(stderr, "Preview size 0x%x bytes\n", preview.preview_length);

  err = IPC_publishData(CARMEN_CANON_PREVIEW_NAME, &preview);
  carmen_test_ipc_exit(err, "Could not publish", 
		       CARMEN_CANON_PREVIEW_NAME);

  free(preview.preview);
}

int main(int argc, char **argv)
{
  /* initialize IPC */
  carmen_initialize_ipc(argv[0]);
  carmen_param_check_version(argv[0]);
  read_parameters(argc, argv);
  initialize_ipc_messages();
  signal(SIGINT, shutdown_module);

  /* initialize the digital camera */
  camera_handle = canon_open_camera();
  if(camera_handle == NULL)
    carmen_die("Erorr: could not open connection to camera.\n");
  if(canon_initialize_camera(camera_handle) < 0)
    carmen_die("Erorr: could not open initialize camera.\n");

  if(canon_initialize_capture(camera_handle, FULL_TO_DRIVE | THUMB_TO_PC,
			      use_flash) < 0)
    carmen_die("Error: could not start image capture.\n");

  if(canon_initialize_preview(camera_handle) < 0)
    carmen_die("Error: could not initialize preview\n");

  fprintf(stderr, "Camera is ready to capture.\n");
  
  IPC_addTimer(60, TRIGGER_FOREVER, publish_preview, NULL);

  /* run the main loop */
  IPC_dispatch();
  return 0;
}
