#include <carmen/carmen.h>
#include "canon_interface.h"

int main(int argc __attribute__ ((unused)), char **argv)
{
  carmen_canon_image_message *image = NULL;
  int thumb_over_ipc, image_over_ipc, image_to_drive;
  FILE *fp;

  carmen_initialize_ipc(argv[0]);
  carmen_param_check_version(argv[0]);

  if(argc < 4)
    carmen_die("Error: not enough arguments.\n");

  thumb_over_ipc = atoi(argv[1]);
  image_over_ipc = atoi(argv[2]);
  image_to_drive = atoi(argv[3]);

  if((image = carmen_canon_get_image(thumb_over_ipc, image_over_ipc, 
				     image_to_drive, FLASH_OFF)) == NULL)
    carmen_die("Error: could not get image from server.\n");
  fprintf(stderr, "Received image.\n");

  if(image->image_length > 0) {
    fp = fopen("image.jpg", "w");
    if(fp == NULL)
      carmen_die("Error: could not open file image.jpg for writing.\n");
    fprintf(stderr, "Writing %d bytes to file.\n", image->image_length);
    fwrite(image->image, image->image_length, 1, fp);
    fclose(fp);
  }

  if(image->thumbnail_length > 0) {
    fp = fopen("thumbnail.jpg", "w");
    if(fp == NULL)
      carmen_die("Error: could not open file thumbnail.jpg for writing.\n");
    fprintf(stderr, "Writing %d bytes to file.\n", image->thumbnail_length);
    fwrite(image->thumbnail, image->thumbnail_length, 1, fp);
    fclose(fp);
  }

  carmen_canon_free_image(&image);

  return 0;
}
