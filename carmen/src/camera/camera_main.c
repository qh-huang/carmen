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
  static carmen_camera_image_message msg;

  IPC_RETURN_TYPE err;

  msg.host = carmen_get_host();
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
  double interframe_sleep = 5.0;
  int param_err;

  /* connect to IPC server */
  carmen_initialize_ipc(argv[0]);
  carmen_param_check_version(argv[0]);

  err = IPC_defineMsg(CARMEN_CAMERA_IMAGE_NAME, IPC_VARIABLE_LENGTH, 
		      CARMEN_CAMERA_IMAGE_FMT);
  carmen_test_ipc_exit(err, "Could not define", CARMEN_CAMERA_IMAGE_NAME);  

  carmen_param_allow_unfound_variables(0);
  param_err = carmen_param_get_double("camera_interframe_sleep", &interframe_sleep);
  if (param_err < 0)
    carmen_die("Could not find parameter in carmen.ini file: camera_interframe_sleep\n");

  image = carmen_camera_start(argc, argv);
  if (image == NULL)
    exit(-1);
  signal(SIGINT, shutdown_module);
  
  while(1) {
    carmen_camera_grab_image(image);
    if(image->is_new) {
      carmen_camera_publish_image_message(image);
      carmen_warn("c");
      image->is_new = 0;
    }
  
    // We are rate-controlling the camera.

    carmen_publish_heartbeat("camera_daemon");
    carmen_ipc_sleep(interframe_sleep);
  }
  return 0;
}
