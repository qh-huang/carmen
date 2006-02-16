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

/** @addtogroup navigator libconventional **/
// @{

/** 
 * \file conventional.h 
 * \brief Library for conventional planning.
 *
 * ...
 **/


#ifndef CONVENTIONAL_H
#define CONVENTIONAL_H

#ifdef __cplusplus
extern "C" {
#endif

  void carmen_conventional_dynamic_program(int goal_x, int goal_y);
  void carmen_conventional_find_best_action(carmen_map_point_p curpoint);
  void carmen_conventional_end_planner(void);
  double *carmen_conventional_get_utility_ptr(void);
  double *carmen_conventional_get_costs_ptr(void);
  double carmen_conventional_get_utility(int x, int y);
  double carmen_conventional_set_utility(int x, int y, double new_utility);
  double carmen_conventional_get_cost(int x, int y);
  void carmen_conventional_build_costs(carmen_robot_config_t *robot_conf,
				       carmen_map_point_t *robot_posn,
				       carmen_navigator_config_t *navigator_conf);

#ifdef __cplusplus
}
#endif

#endif
// @}
