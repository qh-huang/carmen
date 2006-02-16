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

/** @addtogroup joystick libjoyctrl **/
// @{

/** 
 * \file joycrtl.h 
 * \brief Library for using joysticks.
 *
 * ...
 **/


#ifndef CARMEN_JOYCTRL_H
#define CARMEN_JOYCTRL_H

#ifdef __cplusplus
extern "C" {
#endif

#include <carmen/carmen.h>

#include <sys/ioctl.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <linux/joystick.h>

#define         CARMEN_JOYSTICK_DEVICE         "/dev/js0"
#define         CARMEN_JOYSTICK_DEADSPOT       1
#define         CARMEN_JOYSTICK_DEADSPOT_SIZE  0.2

typedef struct {
  int fd;
  int nb_axes;
  int nb_buttons;
  int *axes;
  int *buttons;
  int deadspot;
  double deadspot_size;
  int initialized;
} carmen_joystick_type;

/* Set joystick deadspot */
void carmen_set_deadspot(carmen_joystick_type *joystick, int on_off, double size);

/* Initialize joystick; has to be called before any other function; *
 * returns 0 on success, else -1                                    */
int carmen_initialize_joystick(carmen_joystick_type *joystick);

/* Request joystick state; returns number of bytes read on success, *
 * else -1;                                                         */
int carmen_get_joystick_state(carmen_joystick_type *joystick);

/* Close joystick */
void carmen_close_joystick(carmen_joystick_type *joystick);

/* returns translational and rotational velocities from joystick */
void carmen_joystick_control(carmen_joystick_type *joystick, double max_tv, 
			     double max_rv, double *tv, double *rv);

#ifdef __cplusplus
}
#endif

#endif
// @}
