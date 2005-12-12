#include <carmen/carmen.h>

static void
handler(carmen_navigator_autonomous_stopped_message *msg 
	__attribute__ ((unused)))
{
  carmen_die("Received autonomous stopped\n");
}

int main(int argc, char **argv)
{
  carmen_point_t goal;
  carmen_navigator_status_message *status;

  carmen_ipc_initialize(argc, argv);

  carmen_navigator_query_status(&status);
  carmen_warn("status %f %f %f\n", status->robot.x,
	      status->robot.y, carmen_radians_to_degrees(status->robot.theta));
  goal.x = status->robot.x;
  goal.y = status->robot.y;
  goal.theta = carmen_normalize_theta(status->robot.theta + M_PI);
  free(status);
  carmen_navigator_set_goal_triplet(&goal);
 
  carmen_navigator_subscribe_autonomous_stopped_message
    (NULL, (carmen_handler_t)handler, CARMEN_SUBSCRIBE_LATEST);
 
  carmen_navigator_go();
  carmen_ipc_dispatch();
  
  return 0;
}

