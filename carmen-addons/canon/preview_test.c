#include <carmen/carmen.h>
#include "canon_interface.h"

carmen_canon_preview_message preview;

void canon_preview_handler(void)
{
  FILE *fp;

  fprintf(stderr, "received a preview 0x%x bytes.\n", preview.preview_length);

  if(preview.preview_length > 0) {
    fp = fopen("preview.jpg", "w");
    if(fp == NULL)
      carmen_die("Error: could not open file preview.jpg for writing.\n");
    fwrite(preview.preview, preview.preview_length, 1, fp);
    fclose(fp);
  }
}

int main(int argc __attribute__ ((unused)), char **argv)
{
  carmen_initialize_ipc(argv[0]);
  carmen_param_check_version(argv[0]);
  
  carmen_canon_subscribe_preview_message(&preview, 
					 (carmen_handler_t)canon_preview_handler,
					 CARMEN_SUBSCRIBE_LATEST);

  IPC_dispatch();
  return 0;
}
