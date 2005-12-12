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
#include <carmen/laser_interface.h>

void
carmen_laser_subscribe_frontlaser_message(carmen_laser_laser_message *laser,
					  carmen_handler_t handler,
					  carmen_subscribe_t subscribe_how)
{
  carmen_subscribe_message(CARMEN_LASER_FRONTLASER_NAME, 
			   CARMEN_LASER_FRONTLASER_FMT,
			   laser, sizeof(carmen_laser_laser_message), handler,
			   subscribe_how);
}

void
carmen_laser_unsubscribe_frontlaser_message(carmen_handler_t handler)
{
  carmen_unsubscribe_message(CARMEN_LASER_FRONTLASER_NAME, handler);
}

void
carmen_laser_subscribe_rearlaser_message(carmen_laser_laser_message *laser,
					 carmen_handler_t handler,
					 carmen_subscribe_t subscribe_how)
{
  carmen_subscribe_message(CARMEN_LASER_REARLASER_NAME, 
			   CARMEN_LASER_REARLASER_FMT,
			   laser, sizeof(carmen_laser_laser_message), handler,
			   subscribe_how);
}

void
carmen_laser_unsubscribe_rearlaser_message(carmen_handler_t handler)
{
  carmen_unsubscribe_message(CARMEN_LASER_REARLASER_NAME, handler);
}

void
carmen_laser_subscribe_laser1_message(carmen_laser_laser_message *laser,
				      carmen_handler_t handler,
				      carmen_subscribe_t subscribe_how)
{
  carmen_laser_subscribe_frontlaser_message(laser, handler, subscribe_how);
}

void
carmen_laser_subscribe_laser2_message(carmen_laser_laser_message *laser,
				      carmen_handler_t handler,
				      carmen_subscribe_t subscribe_how)
{
  carmen_laser_subscribe_rearlaser_message(laser, handler, subscribe_how);
}

void
carmen_laser_subscribe_laser3_message(carmen_laser_laser_message *laser,
				      carmen_handler_t handler,
				      carmen_subscribe_t subscribe_how)
{
  carmen_subscribe_message(CARMEN_LASER_LASER3_NAME, 
			   CARMEN_LASER_LASER3_FMT,
			   laser, sizeof(carmen_laser_laser_message), handler,
			   subscribe_how);
}

void
carmen_laser_unsubscribe_laser3_message(carmen_handler_t handler)
{
  carmen_unsubscribe_message(CARMEN_LASER_LASER3_NAME, handler);
}

void
carmen_laser_subscribe_laser4_message(carmen_laser_laser_message *laser,
				      carmen_handler_t handler,
				      carmen_subscribe_t subscribe_how)
{
  carmen_subscribe_message(CARMEN_LASER_LASER4_NAME, 
			   CARMEN_LASER_LASER4_FMT,
			   laser, sizeof(carmen_laser_laser_message), handler,
			   subscribe_how);
}

void
carmen_laser_unsubscribe_laser4_message(carmen_handler_t handler)
{
  carmen_unsubscribe_message(CARMEN_LASER_LASER4_NAME, handler);
}

void
carmen_laser_subscribe_alive_message(carmen_laser_alive_message *alive,
				     carmen_handler_t handler,
				     carmen_subscribe_t subscribe_how)
{
  carmen_subscribe_message(CARMEN_LASER_ALIVE_NAME, 
			   CARMEN_LASER_ALIVE_FMT,
			   alive, sizeof(carmen_laser_alive_message), handler,
			   subscribe_how);
}

void
carmen_laser_unsubscribe_alive_message(carmen_handler_t handler)
{
  carmen_unsubscribe_message(CARMEN_LASER_ALIVE_NAME, handler);
}

void
carmen_laser_subscribe_frontlaser_remission_message(carmen_laser_remission_message *laser,
						    carmen_handler_t handler,
						    carmen_subscribe_t subscribe_how)
{
  carmen_subscribe_message(CARMEN_LASER_FRONTLASER_REMISSION_NAME, 
			   CARMEN_LASER_FRONTLASER_REMISSION_FMT,
			   laser, sizeof(carmen_laser_remission_message), 
			   handler, subscribe_how);
}

void
carmen_laser_unsubscribe_frontlaser_remission_message(carmen_handler_t handler)
{
  carmen_unsubscribe_message(CARMEN_LASER_FRONTLASER_REMISSION_NAME, handler);
}

void
carmen_laser_subscribe_rearlaser_remission_message(carmen_laser_remission_message *laser,
						   carmen_handler_t handler,
						   carmen_subscribe_t subscribe_how)
{
  carmen_subscribe_message(CARMEN_LASER_REARLASER_REMISSION_NAME, 
			   CARMEN_LASER_REARLASER_REMISSION_FMT,
			   laser, sizeof(carmen_laser_remission_message), 
			   handler, subscribe_how);
}

void
carmen_laser_unsubscribe_rearlaser_remission_message(carmen_handler_t handler)
{
  carmen_unsubscribe_message(CARMEN_LASER_REARLASER_REMISSION_NAME, handler);
}

void
carmen_laser_subscribe_laser3_remission_message(carmen_laser_remission_message *laser,
						carmen_handler_t handler,
						carmen_subscribe_t subscribe_how)
{
  carmen_subscribe_message(CARMEN_LASER_LASER3_REMISSION_NAME, 
			   CARMEN_LASER_LASER3_REMISSION_FMT,
			   laser, sizeof(carmen_laser_remission_message), 
			   handler, subscribe_how);
}

void
carmen_laser_unsubscribe_laser3_remission_message(carmen_handler_t handler)
{
  carmen_unsubscribe_message(CARMEN_LASER_LASER3_REMISSION_NAME, handler);
}

void
carmen_laser_subscribe_laser4_remission_message(carmen_laser_remission_message *laser,
						carmen_handler_t handler,
						carmen_subscribe_t subscribe_how)
{
  carmen_subscribe_message(CARMEN_LASER_LASER4_REMISSION_NAME, 
			   CARMEN_LASER_LASER4_REMISSION_FMT,
			   laser, sizeof(carmen_laser_remission_message), 
			   handler, subscribe_how);
}

void
carmen_laser_unsubscribe_laser4_remission_message(carmen_handler_t handler)
{
  carmen_unsubscribe_message(CARMEN_LASER_LASER4_REMISSION_NAME, handler);
}

void
carmen_laser_subscribe_laser1_remission_message(carmen_laser_remission_message *laser,
						carmen_handler_t handler,
						carmen_subscribe_t subscribe_how)
{
  carmen_laser_subscribe_frontlaser_remission_message(laser, handler, subscribe_how);
}

void
carmen_laser_subscribe_laser2_remission_message(carmen_laser_remission_message *laser,
						carmen_handler_t handler,
						carmen_subscribe_t subscribe_how)
{
  carmen_laser_subscribe_rearlaser_remission_message(laser, handler, subscribe_how);
}

double 
carmen_laser_get_fov(int num_beams) {

  if (num_beams == 181) 
    return M_PI;                  /* 180 degrees */
  else if (num_beams == 180) 
    return M_PI / 180.0 * 179.0;  /* 179 degrees (last beam ignored)*/
  else if (num_beams == 361) 
    return M_PI;                  /* 180 degrees */
  else if (num_beams == 360) 
    return M_PI / 180.0 * 179.5 ; /* 179.5 degrees (last beam ignored)*/
  else if (num_beams == 401) 
    return M_PI / 180.0 * 100.0 ; /* 100.0 degrees */
  else if (num_beams == 400) 
    return M_PI / 100.0 * 99.75 ; /* 99.75 degrees (last beam ignored)*/
  else  
    return M_PI;                  /* assume 180 degrees */
}

double 
carmen_laser_get_angle_increment(int num_beams) {
  
  if (num_beams == 181 || num_beams == 180) 
    return  M_PI / 180.0; /* 1 degree = M_PI/180 */
  else if (num_beams == 361 || num_beams == 360) 
    return M_PI / 360.0;  /* 0.5 degrees = M_PI/360 */
  else if (num_beams == 401 || num_beams == 400) 
    return M_PI / 720.0;  /* 0.25 degrees = M_PI/720 */
  else 
    return carmen_laser_get_fov(num_beams) / 
      ((double) (num_beams-1));
}



