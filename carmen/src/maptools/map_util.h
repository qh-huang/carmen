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

#ifndef CARMEN_MAP_UTIL_H
#define CARMEN_MAP_UTIL_H

void carmen_map_util_change_resolution(carmen_map_p map, double new_resolution);
void carmen_map_minimize_gridmap(carmen_map_p map, int *x_offset, int *y_offset);
void carmen_map_move_offlimits_chunk(carmen_offlimits_list_t *offlimits_list, 
				     int x_offset, int y_offset);
void carmen_map_free_hmap(carmen_hmap_p hmap);

void carmen_rotate_places_chunk(carmen_place_p places, int num_places, 
				carmen_map_p map, int rotation);

void carmen_rotate_gridmap(carmen_map_p map, int rotation);

#endif
