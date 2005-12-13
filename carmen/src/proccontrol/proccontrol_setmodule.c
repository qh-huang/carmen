#include <carmen/carmen.h>
#include <carmen/proccontrol_interface.h>

int main(int argc, char **argv)
{
  int requested_state = 0;

  if(argc < 3)
    carmen_die("Error: not enough arguments.\n"
	       "Usage: %s modulename requested-state\n", argv[0]);
  
  if(strcmp(argv[2], "0") == 0 ||
     strcmp(argv[2], "OFF") == 0 ||
     strcmp(argv[2], "DOWN") == 0)
    requested_state = 0;
  else if(strcmp(argv[2], "1") == 0 ||
          strcmp(argv[2], "ON") == 0 ||
          strcmp(argv[2], "UP") == 0)
    requested_state = 1;
  else
    carmen_die("Error: requested state %s invalid.\n", argv[2]);

  /* connect to the IPC server */
  carmen_ipc_initialize(argc, argv);
  carmen_proccontrol_set_module_state(argv[1], requested_state);
  return 0;
}
