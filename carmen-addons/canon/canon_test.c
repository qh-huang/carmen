#include <carmen/carmen.h>
#include "canon_interface.h"

int main(int argc __attribute__ ((unused)), char **argv)
{
  carmen_canon_image_message *image = NULL;
  FILE *fp;

  carmen_initialize_ipc(argv[0]);
  carmen_param_check_version(argv[0]);

  fp = fopen("image.jpg", "w");
  if(fp == NULL)
    carmen_die("Error: could not open file for writing.\n");

  carmen_canon_get_image(&image);
  fprintf(stderr, "Done.\n");

  fprintf(stderr, "Writing %d bytes to file.\n", image->image_length);
  fwrite(image->image, image->image_length, 1, fp);
  fclose(fp);
  return 0;
}
