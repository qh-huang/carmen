#include <carmen/carmen.h>
#include <values.h>
#include <assert.h>
#include "roadmap.h"
#include "dynamics.h"

static double compute_path_time(carmen_roadmap_t *roadmap)
{
  int i;
  double total = 0;
  carmen_traj_point_t *start, *dest;
  int num_blocking_people;

  assert (roadmap->path->length > 0);

  start = (carmen_traj_point_t *)carmen_list_get(roadmap->path, 0);
  for (i = 1; i < roadmap->path->length; i++) {
    dest = (carmen_traj_point_t *)carmen_list_get(roadmap->path, i);
    total += carmen_distance_traj(start, dest)/roadmap->robot_speed;
    carmen_warn("%f : %f %f -> %f %f\n", total, start->x, start->y,
		dest->x, dest->y);
    if (!roadmap->avoid_people) {
      num_blocking_people = carmen_dynamics_num_blocking_people(start, dest);
      total += 30*num_blocking_people;
    }
    start = dest;
  }

  return total;
}

static void synchronize_node_lists(carmen_roadmap_t *roadmap,
				   carmen_roadmap_t *roadmap_without_people)
{
  carmen_roadmap_vertex_t *node;
  int i;

  if (roadmap_without_people->nodes->length >= roadmap->nodes->length) 
    return;

  for (i = roadmap_without_people->nodes->length; i < roadmap->nodes->length;
       i++) {
    node = (carmen_roadmap_vertex_t *)carmen_list_get(roadmap->nodes, i);
    carmen_roadmap_add_node(roadmap_without_people, node->x, node->y);
  }
}

carmen_list_t *carmen_roadmap_pomdp_generate_path(carmen_traj_point_t *robot,
						  carmen_roadmap_t *roadmap,
						  carmen_roadmap_t *
						  roadmap_without_people)
{
  int path_ok, path_without_people_ok;
  double time, time_without_people;

  carmen_warn("Synchronizing lists\n");

  synchronize_node_lists(roadmap, roadmap_without_people);

  carmen_warn("Synchronizing lists done\n");

  carmen_warn("About to generate path with people\n");

  path_ok = carmen_roadmap_generate_path(robot, roadmap);

  carmen_warn("Generated path with people: %d\n", path_ok);

  if (path_ok > 0) {
    time = compute_path_time(roadmap);
    carmen_warn("Computed path with people: %f\n", time);
  } else 
    time = MAXFLOAT;
  
  carmen_warn("About to generate path without people\n");

  path_without_people_ok = carmen_roadmap_generate_path
    (robot, roadmap_without_people);

  carmen_warn("Generated path without people: %d\n", path_without_people_ok);

  if (path_without_people_ok > 0) {
    time_without_people = 
      compute_path_time(roadmap_without_people);
    carmen_warn("Computed path without people: %f\n", time_without_people);
  } else 
    time_without_people = MAXFLOAT;

  if (time > MAXFLOAT/2 && time_without_people > MAXFLOAT/2)
    return NULL;
  
  if (time > time_without_people)
    return roadmap_without_people->path;

  return roadmap->path;
}
