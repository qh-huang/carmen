/*********************************************************
 *
 * This source code is part of the Carnegie Mellon Robot
 * Navigation Toolkit (CARMEN)
 *
 * CARMEN Copyright (c) 2002 Michael Montemerlo, Nicholas
 * Roy, and Sebastian Thrun
 *
 * CARMEN is free software; you can redistribute it and/or 
 * modify it under the terms of the GNU General Public 
 * License as published by the Free Software Foundation; 
 * either version 2 of the License, or (at your option)
 * any later version.
 *
 * CARMEN is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied 
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 * PURPOSE.  See the GNU General Public License for more 
 * details.
 *
 * You should have received a copy of the GNU General 
 * Public License along with CARMEN; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place, 
 * Suite 330, Boston, MA  02111-1307 USA
 *
 ********************************************************/

#include <carmen/carmen.h>
#include <assert.h>
#include "roadmap.h"
#include "dynamics.h"
#include "planner_interface.h"
#include "navigator.h"

static carmen_map_t *map = NULL;

static int have_goal = 0, have_robot = 0;
static int allow_any_orientation = 0;
static carmen_world_point_t goal;
static carmen_traj_point_t robot;
static double max_t_vel;
static double approach_dist;

static carmen_roadmap_t *roadmap = NULL;

int carmen_planner_update_goal(carmen_world_point_p new_goal, 
			       int any_orientation)
{
  allow_any_orientation = any_orientation;
  have_goal = 1;
  goal = *new_goal;

  if (map) {
    carmen_dynamics_update();
    carmen_roadmap_plan(roadmap, new_goal);
  }

  carmen_verbose("Set goal to %.1f %.1f, done planning\n",
	      new_goal->pose.x, new_goal->pose.y);

  return 1;
}

int carmen_planner_update_robot(carmen_traj_point_p new_position)
{
  robot = *new_position;
  have_robot = 1;

  return 0;
}

void carmen_planner_set_map(carmen_map_p new_map)
{
  map = new_map;

  carmen_verbose("Initialized with map\n");

  roadmap = carmen_roadmap_initialize(new_map);
  carmen_param_set_module("robot");
  carmen_param_get_double("max_t_vel", &max_t_vel);
  carmen_param_get_double("min_approach_dist", &approach_dist);
  carmen_param_subscribe_double("robot", "max_t_vel", &max_t_vel, NULL);
  carmen_param_subscribe_double("robot", "min_approach_dist", 
				&approach_dist, NULL);

  carmen_warn("max robot speed %f\n", max_t_vel);

  carmen_dynamics_initialize(new_map);
}

void carmen_planner_reset(void)
{
  have_goal = 0;
  have_robot = 0;
}

void carmen_planner_update_map(carmen_robot_laser_message *laser_msg)
{
  laser_msg = laser_msg;

}

static double check_path(carmen_roadmap_t *road, carmen_world_point_t *start,
			 int *num_segments)
{
  carmen_roadmap_vertex_t *n1, *n2;
  double length;

  n1 = carmen_roadmap_nearest_node(start, road);

  if (carmen_dynamics_test_node(n1, 1)) 
    return -2;

  n2 = (carmen_roadmap_vertex_t *)carmen_list_get(road->nodes, road->goal_id);
  if (carmen_dynamics_test_node(n2, 1)) 
    return -2;
  
  length = 0;
  *num_segments = 1;
  while (n1 != NULL && n1->utility > 0) {
    n2 = carmen_roadmap_next_node(n1, road);
    if (n2 != NULL) 
      length += hypot(n2->x - n1->x, n2->y - n1->y);
    n1 = n2;
    (*num_segments)++;
  }

  if (!n1 || n1->utility > 0) 
    return -1;

  return length;
}

void carmen_planner_get_status(carmen_planner_status_p status) 
{
  int index;
  carmen_roadmap_vertex_t *node;
  carmen_map_point_t map_node;
  carmen_world_point_t world_robot, wp;
  //  double short_length;
  double long_length;
  int num_segments;

  status->goal_set = have_goal;

  status->path.length = 0;
  status->path.points = NULL;

  if (have_goal)
    status->goal = goal.pose;

  if (have_robot)
    status->robot = robot;

  if (!have_robot || !have_goal)
    return;  

  world_robot.pose.x = robot.x;
  world_robot.pose.y = robot.y;
  world_robot.map = map;

  long_length =  check_path(roadmap, &world_robot, &num_segments);

  if (long_length < 0) {
    status->path.length = 0;
    carmen_dynamics_update();
    if (long_length > -2)
      carmen_roadmap_plan(roadmap, &goal);
    return;
  } 
  status->path.length = num_segments;

  status->path.points = (carmen_traj_point_p)
    calloc(status->path.length, sizeof(carmen_traj_point_t));
  carmen_test_alloc(status->path.points);
  node = carmen_roadmap_nearest_node(&world_robot, roadmap);
  map_node.x = node->x;
  map_node.y = node->y;
  map_node.map = map;
  carmen_map_to_world(&map_node, &wp);
  
  status->path.points[0].x = wp.pose.x;
  status->path.points[0].y = wp.pose.y;
  
  index = 1;
  while (node != NULL && node->utility > 0) {
    node = carmen_roadmap_next_node(node, roadmap);
    if (node != NULL) {
      map_node.x = node->x;
      map_node.y = node->y;
      carmen_map_to_world(&map_node, &wp);
      
      status->path.points[index].x = wp.pose.x;
      status->path.points[index].y = wp.pose.y;
      index++;
    }
  }

  return;
}

int carmen_planner_next_waypoint(carmen_traj_point_p waypoint, int *is_goal,
				 carmen_navigator_config_t *nav_conf)
{  
  double delta_dist, delta_theta;
  carmen_roadmap_vertex_t *node, *next_node;
  carmen_world_point_t start, dest;
  carmen_map_point_t dest_map;

  if (!have_goal || !map)
    return -1;

  start.pose.x = waypoint->x;
  start.pose.y = waypoint->y;
  start.pose.theta = waypoint->theta;
  start.map = map;


  node = carmen_roadmap_nearest_node(&start, roadmap);
  if (node == NULL)
    return -1;

  if (carmen_dynamics_test_node(node, 1)) 
    return -1;

  next_node = (carmen_roadmap_vertex_t *)
    carmen_list_get(roadmap->nodes, roadmap->goal_id);
  if (carmen_dynamics_test_node(next_node, 1)) 
    return -1;
  
  dest_map.x = node->x;
  dest_map.y = node->y;
  dest_map.map = map;
  carmen_map_to_world(&dest_map, &dest);
  
  delta_dist = carmen_distance_world(&start, &dest);
  
  while (delta_dist < 2*nav_conf->goal_size && node->utility > 0) {
    next_node = carmen_roadmap_next_node(node, roadmap);
    if (next_node == NULL)
      return -1;
    if (!carmen_roadmap_is_visible(next_node, &start, roadmap) &&
	delta_dist < approach_dist)
      break;
    node = next_node;
    dest_map.x = node->x;
    dest_map.y = node->y;
    dest_map.map = map;
    carmen_map_to_world(&dest_map, &dest);
    delta_dist = carmen_distance_world(&start, &dest);    
  } 

  if (delta_dist < 2*nav_conf->goal_size && node->utility == 0) {
    *is_goal = 1;
    if (allow_any_orientation)
      return 1;
    delta_theta = fabs(waypoint->theta - goal.pose.theta) ;
    if (delta_theta < nav_conf->goal_theta_tolerance)
      return 1;      
    waypoint->theta = goal.pose.theta;
  } else {
    waypoint->x = dest.pose.x;
    waypoint->y = dest.pose.y;
  }

  return 0;
}

carmen_navigator_map_message *
carmen_planner_get_map_message(carmen_navigator_map_t map_type) 
{
  carmen_navigator_map_message *reply;
  int size, x_Size, y_Size;
  float *map_ptr;

  reply = (carmen_navigator_map_message *)
    calloc(1, sizeof(carmen_navigator_map_message));
  carmen_test_alloc(reply);
  
  x_Size = map->config.x_size;
  y_Size = map->config.y_size;
  size = x_Size * y_Size;

  reply->data = (unsigned char *)calloc(size, sizeof(float));
  carmen_test_alloc(reply->data);

  reply->config = map->config;

  switch (map_type) {
  case CARMEN_NAVIGATOR_MAP_v:
    map_ptr = map->complete_map;
    break;
  default:
    carmen_warn("Request for unsupported data type : %d.\n", map_type);
    reply->size = 0;
    /*    gcc 3.3.3 does not compile with: */
    /*    (int)(reply->map_type) = -1; */
    reply->map_type = -1;
    return reply;
  }
  reply->size = size*sizeof(float);
  reply->map_type = map_type;
  memcpy(reply->data, map_ptr, size*sizeof(float));
  if (map_type == CARMEN_NAVIGATOR_UTILITY_v)
    free(map_ptr);
  return reply;
}

