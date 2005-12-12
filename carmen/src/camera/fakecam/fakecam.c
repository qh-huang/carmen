#include <carmen/carmen.h>
#include <assert.h>
#include "img3.xpm"
#include <X11/xpm.h>
#include "../camera_hw_interface.h"

static carmen_camera_image_t saved_image;

carmen_camera_image_t *carmen_camera_start(int argc __attribute__ ((unused)), 
					   char **argv __attribute__ ((unused)))
{
  carmen_camera_image_t *image;
  XpmImage xpm_image;
  XpmInfo xpm_info;  
  unsigned int *xpm_ptr;
  char *image_ptr;
  int i;
  long int colour;

  XpmCreateXpmImageFromData(img3, &xpm_image, &xpm_info);

  saved_image.width = xpm_image.width;
  saved_image.height = xpm_image.height;
  saved_image.bytes_per_pixel = 3;

  saved_image.image_size = saved_image.width*saved_image.height*
    saved_image.bytes_per_pixel;
  saved_image.is_new = 0;
  saved_image.timestamp = 0;

  saved_image.image = (char *)calloc(saved_image.image_size, sizeof(char));
  carmen_test_alloc(saved_image.image);

  xpm_ptr = xpm_image.data;
  image_ptr = saved_image.image;

  for (i = 0; i < saved_image.width*saved_image.height; i++) {
    assert ((unsigned int)*xpm_ptr < xpm_image.ncolors);
    colour = strtol(xpm_image.colorTable[*xpm_ptr].c_color+1, NULL, 16);
    
    image_ptr[0] = (colour >> 16) & 0xFF;
    image_ptr[1] = (colour >> 8) & 0xFF;
    image_ptr[2] = colour & 0xFF;

    xpm_ptr++;
    image_ptr += 3;
  }
  
  image = (carmen_camera_image_t *)calloc(1, sizeof(carmen_camera_image_t));
  memcpy(image, &saved_image, sizeof(carmen_camera_image_t));
  image->image = (char *)calloc(image->image_size, sizeof(char));
  carmen_test_alloc(image->image);
  memcpy(image->image, saved_image.image, sizeof(char)*image->image_size);

  return image;
}

void carmen_camera_shutdown(void)
{
}

void carmen_camera_grab_image(carmen_camera_image_t *image)
{
  memcpy(image->image, saved_image.image, sizeof(char)*image->image_size);
  image->timestamp = carmen_get_time();
  image->is_new = 1;
}

