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

int 
carmen_cerebellum_connect_robot(char *dev)
{
  dev = dev;
  return 0;
}

int carmen_cerebellum_ac(int acc)
{
  acc = acc;
  return 0;
}

int 
carmen_cerebellum_set_velocity(int command_vl, int command_vr)
{
  command_vl = command_vl;
  command_vr = command_vr;
  return 0;
}

int 
carmen_cerebellum_get_state(int *left_tics, int *right_tics,
			    int *left_vel, int *right_vel)
{
  left_tics = left_tics;
  right_tics = right_tics;

  left_vel = left_vel;
  right_vel = right_vel;

  return 0;
}

int 
carmen_cerebellum_limp(void)
{
  return 0;
}

int 
carmen_cerebellum_disconnect_robot(void)
{
  return 0;
}
