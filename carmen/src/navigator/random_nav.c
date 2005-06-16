#include <carmen/carmen.h>

static carmen_map_t *c_space;
static int up_to_speed = 0;

static void pick_random_place(void)
{
  carmen_map_point_t map_pt;
  carmen_world_point_t world_pt;
  map_pt.map = c_space;

  do {
    map_pt.x = carmen_uniform_random(0, c_space->config.x_size);
    map_pt.y = carmen_uniform_random(0, c_space->config.y_size);    
  } while (c_space->map[map_pt.x][map_pt.y] < 0);

  carmen_map_to_world(&map_pt, &world_pt);
  carmen_navigator_set_goal(world_pt.pose.x, world_pt.pose.y);
  carmen_navigator_go();
  up_to_speed = 0;
}

static void autonomous_stopped_handler
(carmen_navigator_autonomous_stopped_message *autonomous_stopped
 __attribute__ ((unused)))
{
  pick_random_place();
}

static void odometry_handler(carmen_base_odometry_message odometry)
{
  if (!up_to_speed && odometry.tv > .25)
    up_to_speed = 1;
  else if (up_to_speed && odometry.tv < .2)
    pick_random_place();
}

void check_free_space_from_navigator(carmen_map_p c_space)
{
  float *map_ptr;
  float *utility_ptr;
  carmen_map_t utility_map;
  int x, y;
  carmen_map_placelist_t placelist;
  carmen_point_t mean, std_dev;

  if (carmen_map_get_placelist(&placelist) < 0)
    carmen_die("The map must have at least one place in it.\n");

  if (placelist.num_places <= 0)
    carmen_die("The map must have at least one place in it.\n");

  carmen_navigator_set_goal_place(placelist.places[0].name);
  mean.x = placelist.places[0].x;
  mean.y = placelist.places[0].y;
  mean.theta = placelist.places[0].theta;
  std_dev.x = 0.2;
  std_dev.y = 0.2;
  std_dev.theta = carmen_degrees_to_radians(4.0);

  carmen_localize_initialize_gaussian_command(mean, std_dev);

  carmen_navigator_get_map(CARMEN_NAVIGATOR_UTILITY_v, &utility_map);

  map_ptr = c_space->complete_map;
  utility_ptr = utility_map.complete_map;
  for (x = 0; x < c_space->config.x_size; x++)
    for (y = 0; y < c_space->config.y_size; y++) {
      if (*utility_ptr < 0)
	*map_ptr = -1.0;
      utility_ptr++;
      map_ptr++;
    }

  free(utility_map.complete_map);
  free(utility_map.map);
  sleep(1);
}

int main(int argc, char *argv[]) 
{
  carmen_initialize_ipc(argv[0]);
  carmen_randomize(&argc, &argv);

  c_space = (carmen_map_t *)calloc(1, sizeof(carmen_map_t));
  carmen_test_alloc(c_space);

  carmen_map_get_gridmap(c_space);

  carmen_navigator_subscribe_autonomous_stopped_message
    (NULL, (carmen_handler_t)autonomous_stopped_handler,
     CARMEN_SUBSCRIBE_LATEST);
  
  carmen_base_subscribe_odometry_message
    (NULL, (carmen_handler_t)odometry_handler, CARMEN_SUBSCRIBE_LATEST);
  
  pick_random_place();
  IPC_dispatch();

  return 0;
}
