#include <carmen/carmen.h>

int main(int argc, char **argv)
{
  /* connect to IPC */
  carmen_initialize_ipc(argv[0]);

  if(argc < 2)
    carmen_die("Error: not enough arguments.\n"
	       "Usage: %s location\n"
	       "  Use 'global' for global localization.\n", argv[0]);
  if(strncmp(argv[1], "global", 6) == 0) {
    /* send uniform initialization command */
    carmen_localize_initialize_uniform_command();
  }
  else
    carmen_localize_initialize_placename_command(argv[1]);
  return 0;
}
