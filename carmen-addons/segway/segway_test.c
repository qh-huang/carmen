#include <carmen/carmen.h>
#include "segway_interface.h"

void segway_pose_handler(carmen_segway_pose_message *pose)
{
  fprintf(stderr, "P %f %f %f\n", pose->x, pose->y, pose->theta);
}

int main(int argc __attribute__ ((unused)), char **argv)
{
  carmen_initialize_ipc(argv[0]);
  carmen_param_check_version(argv[0]);
 
  carmen_segway_subscribe_pose_message(NULL, 
				       (carmen_handler_t)segway_pose_handler,
				       CARMEN_SUBSCRIBE_ALL);
  IPC_dispatch();
  return 0;
}
