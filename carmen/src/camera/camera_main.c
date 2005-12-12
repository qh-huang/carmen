#include <carmen/carmen.h>
#include "camera_hw_interface.h"
#include "camera_messages.h"

void shutdown_module(int signo __attribute__ ((unused)))
{
  carmen_camera_shutdown();
  exit(0);
}

void carmen_camera_publish_image_message(carmen_camera_image_t *image)
{
  static char *host = NULL;
  static carmen_camera_image_message msg;

  IPC_RETURN_TYPE err;

  if(host == NULL) {
    host = carmen_get_tenchar_host_name();
    strcpy(msg.host, host);
  }

  msg.timestamp = image->timestamp;
  
  msg.image_size = image->image_size;
  msg.width = image->width;
  msg.height = image->height;
  msg.bytes_per_pixel = image->bytes_per_pixel;
  msg.image = image->image;

  err = IPC_publishData(CARMEN_CAMERA_IMAGE_NAME, &msg);
  carmen_test_ipc_exit(err, "Could not publish", 
		       CARMEN_CAMERA_IMAGE_NAME);
}
  
int main(int argc, char **argv) 
{
  carmen_camera_image_t *image;
  IPC_RETURN_TYPE err;

  /* connect to IPC server */
  carmen_initialize_ipc(argv[0]);
  carmen_param_check_version(argv[0]);

  err = IPC_defineMsg(CARMEN_CAMERA_IMAGE_NAME, IPC_VARIABLE_LENGTH, 
		      CARMEN_CAMERA_IMAGE_FMT);
  carmen_test_ipc_exit(err, "Could not define", CARMEN_CAMERA_IMAGE_NAME);  

  image = carmen_camera_start(argc, argv);
  if (image == NULL)
    exit(-1);
  signal(SIGINT, shutdown_module);
  
  while(1) {
    carmen_camera_grab_image(image);
    if(image->is_new) {
      carmen_camera_publish_image_message(image);
      image->is_new = 0;
    }
  
    // We are rate-controlling the camera at 100 Hz.

    sleep_ipc(0.1);
  }
  return 0;
}
