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

#ifndef DYNAMICS_H
#define DYNAMICS_H

#include <carmen/dot.h>
#include <carmen/dot_messages.h>
#include <carmen/dot_interface.h>


#ifdef __cplusplus
extern "C" {
#endif

  void carmen_dynamics_initialize(carmen_map_t *new_map);
  void carmen_dynamics_initialize_no_ipc(carmen_map_t *new_map);
  void carmen_dynamics_update_person(carmen_dot_person_t *update_person);
  void carmen_dynamics_update(void);
  int carmen_dynamics_test_for_block(carmen_roadmap_vertex_t *n1, 
				     carmen_roadmap_vertex_t *n2,
				     int avoid_people);
  int carmen_dynamics_test_node(carmen_roadmap_vertex_t *n1, 
				int avoid_people);
#ifdef __cplusplus
}
#endif
 
#endif
