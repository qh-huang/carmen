#include <carmen/carmen.h>
#include <assert.h>
#include "img3.xpm"
#include "../camera_hw_interface.h"

static carmen_camera_image_t saved_image;

int lookup_colour(char *xpm_ptr, char *colour_table[], int num_colours)
{
  int i;

  for (i = 0; i < num_colours; i++) {
    if (strncmp(xpm_ptr, colour_table[i], 2) == 0) 
      return strtol(colour_table[i]+6, NULL, 16);
  }
  return 0;
}


carmen_camera_image_t *carmen_camera_start(int argc __attribute__ ((unused)), 
					   char **argv __attribute__ ((unused)))
{
  carmen_camera_image_t *image;
  char *xpm_ptr;
  char *image_ptr;
  int x, y;
  long int colour;
  int num_colours;
  
  sscanf(img3[0], "%d %d %d", &(saved_image.width), &(saved_image.height),
	 &num_colours);

  saved_image.bytes_per_pixel = 3;

  saved_image.image_size = saved_image.width*saved_image.height*
    saved_image.bytes_per_pixel;
  saved_image.is_new = 0;
  saved_image.timestamp = 0;

  saved_image.image = (char *)calloc(saved_image.image_size, sizeof(char));
  carmen_test_alloc(saved_image.image);

  image_ptr = saved_image.image;
  for (y = 0; y < saved_image.height; y++) {
    xpm_ptr = img3[num_colours+y+1];
    for (x = 0; x < saved_image.width; x++) {    
      colour = lookup_colour(xpm_ptr, img3+1, num_colours);    
      image_ptr[0] = (colour >> 16) & 0xFF;
      image_ptr[1] = (colour >> 8) & 0xFF;
      image_ptr[2] = colour & 0xFF;
      
      xpm_ptr += 2;
      image_ptr += 3;
    }
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

