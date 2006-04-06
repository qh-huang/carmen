 /*********************************************************
 *
 * This source code is part of the Carnegie Mellon Robot
 * Navigation Toolkit (CARMEN)
 *
 * CARMEN Copyright (c) 2002 Michael Montemerlo, Nicholas
 * Roy, Sebastian Thrun, Dirk Haehnel, Cyrill Stachniss,
 * and Jared Glover
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


/** @addtogroup pantilt **/
// @{

/** \file pantilt_messages.h
 * \brief Definition of the messages for this module.
 *
 * This file specifies the messages for this modules used to transmit
 * data via ipc to other modules.
 **/

#ifndef CARMEN_PANTILT_MESSAGES_H
#define CARMEN_PANTILT_MESSAGES_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
  double  pan;
  double  tilt;
  double  timestamp;
  char*   host;
} carmen_pantilt_status_message;

#define      CARMEN_PANTILT_STATUS_MESSAGE_NAME  "pantilt_status_message"
#define      CARMEN_PANTILT_STATUS_MESSAGE_FMT   "{double, double, double, string}"


typedef struct {
  double  pan;
  double  tilt;
  double  timestamp;
  char*   host;
} carmen_pantilt_move_message;

#define      CARMEN_PANTILT_MOVE_MESSAGE_NAME  "pantilt_move_message"
#define      CARMEN_PANTILT_MOVE_MESSAGE_FMT   "{double,double,double,string}"

typedef struct {
  double pan;
  double timestamp;
  char*  host;
} carmen_pantilt_move_pan_message;

#define      CARMEN_PANTILT_MOVE_PAN_MESSAGE_NAME  "pantilt_move_pan_message"
#define      CARMEN_PANTILT_MOVE_PAN_MESSAGE_FMT   "{double,double,string}"

typedef struct {
  double  tilt;
  double  timestamp;
  char*   host;
 } carmen_pantilt_move_tilt_message;

#define      CARMEN_PANTILT_MOVE_TILT_MESSAGE_NAME  "pantilt_move_tilt_message"
#define      CARMEN_PANTILT_MOVE_TILT_MESSAGE_FMT   "{double,double,string}"

#ifdef __cplusplus
}
#endif

#endif

// @}
