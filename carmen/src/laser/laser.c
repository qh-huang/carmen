#include <carmen/carmen.h>
#include "sick.h"
#include "laser.h"
#include "laser_ipc.h"
#include "laser_main.h"

int main(int argc, char **argv) 
{
  /* connect to IPC server */
  carmen_initialize_ipc(argv[0]);
  carmen_param_check_version(argv[0]);

  if(carmen_laser_start(argc, argv) < 0)
    exit(-1);
  signal(SIGINT, shutdown_laser);
  
  while(1) {
    carmen_laser_run();
    sleep_ipc(0.001);
  }
  return 0;
}
