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
 * Public License along with Foobar; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place, 
 * Suite 330, Boston, MA  02111-1307 USA
 *
 ********************************************************/

#ifndef CARMEN_GPS_INTERFACE_H
#define CARMEN_GPS_INTERFACE_H

#ifdef __cplusplus
extern "C" {
#endif

  //#include <carmen/gps_messages.h>
#include <ringo_gps_messages.h>

void carmen_gps_subscribe_position_message(carmen_gps_position_message 
					   *gps_position_msg,
					   carmen_handler_t handler,
					   carmen_subscribe_t subscribe_how);
#ifdef __cplusplus
}
#endif

#endif
