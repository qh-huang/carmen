#include <carmen/carmen.h>
#include "canon.h"

int main(void)
{
  usb_dev_handle *camera_handle;
  unsigned char *thumbnail, *image;
  int thumbnail_length, image_length;

  camera_handle = canon_open_camera();
  if(camera_handle == NULL)
    carmen_die("Erorr: could not open connection to camera.\n");

  if(canon_initialize_camera(camera_handle) < 0)
    carmen_die("Erorr: could not open initialize camera.\n");

  if(canon_initialize_capture(camera_handle, THUMB_TO_PC | FULL_TO_PC) < 0)
    carmen_die("Error: could not start image capture.\n");

  if(canon_capture_image(camera_handle, &thumbnail, &thumbnail_length,
			 &image, &image_length) < 0)
    carmen_die("Erorr: could not capture image.\n");
  
  if(canon_stop_capture(camera_handle) < 0)
    carmen_die("Error: could not stop capturing.\n");

  canon_close_camera(camera_handle);
  return 0;
}
