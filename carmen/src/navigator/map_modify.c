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

#include "map_modify.h"

#define LASER_HISTORY_LENGTH 5

#define EMPTY 0.01
#define FILLED .9
#define UNKNOWN -1

typedef struct {
  int x, y;
  double value;
} grid_cell_t, *grid_cell_p;

static grid_cell_p *laser_scan;
static int *scan_size;
static int *max_scan_size;
static int current_data_set = 0;

inline static int 
is_empty(double value) 
{
  return (value >= 0) && (value <= EMPTY);
}

inline static int
is_filled(double value) 
{
  return (value >= 0) && (value >= FILLED);
}

inline static int
is_in_map(int x, int y, carmen_map_p map)
{
  if (x < 0 || x >= map->config.x_size)
    return 0;
  if (y < 0 || y >= map->config.y_size)
    return 0;
  return 1;
}

static int 
point_exists(int x, int y, double value) 
{
  int num_points = scan_size[current_data_set];
  grid_cell_p cell = laser_scan[current_data_set];
  int index;

  if (num_points == 0)
    return 0;

  for (index = num_points-1; index >= 0; index--) {
    if (x == cell[index].x && y == cell[index].y &&
	fabs(value - cell[index].value) < .01)
      return 1;
  }
  
  return 0;
}

static void 
add_filled_point(int x, int y, carmen_map_p true_map, carmen_map_p modify_map) 
{
  int num_points;
  grid_cell_p cell_list;
  double value;

  if (!is_in_map(x, y, modify_map))
    return;

  value = true_map->map[x][y];  
  if (is_filled(value))
    return;

  /*   if (point_exists(x, y, FILLED)) */
  /*     return; */

  num_points = scan_size[current_data_set];
  if (num_points == max_scan_size[current_data_set]) {
    max_scan_size[current_data_set] *= 2;
    laser_scan[current_data_set] = 
      realloc(laser_scan[current_data_set], 
	      max_scan_size[current_data_set]*sizeof(grid_cell_t));
    carmen_test_alloc(laser_scan[current_data_set]);
  }

  cell_list = laser_scan[current_data_set];
  value = FILLED;
  /*      carmen_warn("Filling point %d (%d) %d %d\n", num_points, */
  /*  	 current_data_set, x, y); */
  cell_list[num_points].x = x;
  cell_list[num_points].y = y;
  cell_list[num_points].value = value;
  scan_size[current_data_set]++;
  modify_map->map[x][y] = 2.0;
}

static void 
add_clear_point(int x, int y, carmen_map_p true_map, carmen_map_p modify_map) 
{
  int num_points = scan_size[current_data_set];
  grid_cell_p cell_list;
  double value;

  if (!is_in_map(x, y, modify_map))
    return;

  value = true_map->map[x][y];

  /*   if (is_empty(value))  */
  /* 		return; */
		
  if (point_exists(x, y, EMPTY))
    return;

  if (num_points == max_scan_size[current_data_set]) {
    max_scan_size[current_data_set] *= 2;
    laser_scan[current_data_set] = 
      realloc(laser_scan[current_data_set], 
	      max_scan_size[current_data_set]*sizeof(grid_cell_t));
    carmen_test_alloc(laser_scan[current_data_set]);
  }

  cell_list = laser_scan[current_data_set];
  value = EMPTY;
  cell_list[num_points].x = x;
  cell_list[num_points].y = y;
  cell_list[num_points].value = value;
  scan_size[current_data_set]++;
  modify_map->map[x][y] = value;
}

static void 
update_existing_data(carmen_map_p true_map, carmen_map_p modify_map) 
{
  grid_cell_p cell;
  int num_changed_points;
  int index;
  double value;
  int list_index;

  current_data_set = (current_data_set+1) % LASER_HISTORY_LENGTH;
  
  if (laser_scan[current_data_set] == NULL) {
    laser_scan[current_data_set] = 
      (grid_cell_p)calloc(200, sizeof(grid_cell_t));
    carmen_test_alloc(laser_scan[current_data_set]);
    max_scan_size[current_data_set] = 200;
    scan_size[current_data_set] = 0;
  }

  /* The oldest scan is erased here */
  cell = laser_scan[current_data_set];

  num_changed_points = scan_size[current_data_set];
  cell = laser_scan[current_data_set];
  for (index = 0; index < num_changed_points; index++) {
    value = true_map->map[cell->x][cell->y];
    modify_map->map[cell->x][cell->y] = value;
    cell++;
  }
  scan_size[current_data_set] = 0;

  /* The other scans are recopied in here */

  for (list_index = 0; list_index < LASER_HISTORY_LENGTH; list_index++) {
    cell = laser_scan[list_index];
    if (cell == NULL)
      continue;
    num_changed_points = scan_size[list_index];
    for (index = 0; index < num_changed_points; index++) {
      modify_map->map[cell->x][cell->y] = cell->value;
      /*        carmen_warn("Filling point %d of %d in list %d: %d %d\n", index, */
      /*  	   num_changed_points, list_index, cell->x, cell->y); */
      cell++;
    }
  }
}

void 
trace_laser(int x_1, int y_1, int x_2, int y_2, carmen_map_p true_map, 
	    carmen_map_p modify_map) 
{
  carmen_bresenham_param_t params;
  int X, Y;
  double map_value;

  carmen_get_bresenham_parameters(x_1, y_1, x_2, y_2, &params);

  do {
    carmen_get_current_point(&params, &X, &Y);
    if (!is_in_map(X, Y, modify_map))
      break;
    map_value = true_map->map[X][Y];
    if (!is_empty(map_value)) 
      add_clear_point(X, Y, true_map, modify_map);
  } while (carmen_get_next_point(&params));
}

void 
map_modify_update(float *range, int num_readings, carmen_navigator_config_t *config,
		  carmen_world_point_p world_point, carmen_map_p true_map, 
		  carmen_map_p modify_map) 
{
  int index;
  double angle, separation;
  int laser_x, laser_y;
  double dist;
  int increment;
  carmen_map_point_t map_point;  
  int count;

  if (true_map == NULL || modify_map == NULL)
    {
      carmen_warn("%s called with NULL map argument.\n", __FUNCTION__);
      return;
    }

  if (laser_scan == NULL) {
    laser_scan = (grid_cell_p *)calloc(LASER_HISTORY_LENGTH, sizeof(grid_cell_p));
    carmen_test_alloc(laser_scan);

    scan_size = (int *)calloc(LASER_HISTORY_LENGTH, sizeof(int));
    carmen_test_alloc(scan_size);

    max_scan_size = (int *)calloc(LASER_HISTORY_LENGTH, sizeof(int));
    carmen_test_alloc(max_scan_size);
  }

  update_existing_data(true_map, modify_map);

  if (num_readings < config->num_lasers_to_use)
    {
      increment = 1;
      separation = carmen_normalize_theta(M_PI / (num_readings+1));
    } 
  else 
    {
      increment = num_readings / config->num_lasers_to_use;
      separation = carmen_normalize_theta(M_PI / (config->num_lasers_to_use+1));
    }

  angle = carmen_normalize_theta(world_point->pose.theta + carmen_degrees_to_radians(-90));
  carmen_world_to_map(world_point, &map_point); 

  count = 0;
  for (index = 0; index < num_readings; index += increment) {    
    dist = range[index] - modify_map->config.resolution;
    if (dist < 0)
      dist = 0;
    if (dist > config->max_collision_range)
      dist = config->max_collision_range;

    laser_x = carmen_round(map_point.x + (cos(angle)*dist)/
			   modify_map->config.resolution);
    laser_y = carmen_round(map_point.y + (sin(angle)*dist)/
			   modify_map->config.resolution);    
    //    trace_laser(map_point.x, map_point.y, laser_x, laser_y, true_map, modify_map);

    dist = range[index];
    laser_x = carmen_round(map_point.x + (cos(angle)*dist)/
			   modify_map->config.resolution);
    laser_y = carmen_round(map_point.y + (sin(angle)*dist)/
			   modify_map->config.resolution);
    if (is_in_map(laser_x, laser_y, modify_map) && 
	dist < config->max_collision_range &&
	dist < (config->max_range - 2*modify_map->config.resolution)) {
      count++;
      add_filled_point(laser_x, laser_y, true_map, modify_map);
    }
    angle += separation;
  }
}

void 
map_modify_clear(carmen_map_p true_map, carmen_map_p modify_map)
{
  int index;

  if (scan_size == NULL)
    return;

  for (index = 0; index < LASER_HISTORY_LENGTH; index++)
    scan_size[index] = 0;

  memcpy(modify_map->complete_map, true_map->complete_map, 
	 true_map->config.x_size*true_map->config.y_size*sizeof(float));
    
  current_data_set = 0;
}
