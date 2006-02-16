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


/** @addtogroup navigator libnavigator_interface **/
// @{

/** \file navigator_interface.h
 * \brief Definition of the interface of the module navigator.
 *
 * This file specifies the interface to subscribe the messages of
 * that module and to receive its data via ipc.
 **/

#ifndef NAVIGATOR_INTERFACE_H
#define NAVIGATOR_INTERFACE_H

#include <carmen/navigator_messages.h>

#ifdef __cplusplus
extern "C" {
#endif

void carmen_navigator_subscribe_status_message(carmen_navigator_status_message 
					       *status,
					       carmen_handler_t handler,
					       carmen_subscribe_t 
					       subscribe_how);

void carmen_navigator_subscribe_plan_message(carmen_navigator_plan_message 
					     *plan,
					     carmen_handler_t handler,
					     carmen_subscribe_t subscribe_how);
  
void carmen_navigator_subscribe_autonomous_stopped_message
  (carmen_navigator_autonomous_stopped_message *autonomous_stopped,
   carmen_handler_t handler,
   carmen_subscribe_t subscribe_how);
  
int carmen_navigator_query_status(carmen_navigator_status_message **status);
int carmen_navigator_query_plan(carmen_navigator_plan_message **plan);
int carmen_navigator_set_goal(double x, double y);
int carmen_navigator_set_goal_triplet(carmen_point_p goal);
int carmen_navigator_set_goal_place(char *name);
int carmen_navigator_stop(void);
int carmen_navigator_go(void);
int carmen_navigator_get_map(carmen_navigator_map_t map_type, 
			     carmen_map_t *map);

#ifdef __cplusplus
}
#endif

#endif
// @}
