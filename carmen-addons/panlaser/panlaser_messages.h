/*********************************************************
 *
 * This source code is part of the Carnegie Mellon Robot
 * Navigation Toolkit (CARMEN)
 *
 * CARMEN Copyright (c) 2002 Michael Montemerlo, Nicholas
 * Roy, and Sebastian Thrun
 *
 * This module (panlaser) Copyright (c) 2003 Brian Gerkey
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

#ifndef CARMEN_PANLASER_MESSAGES_H
#define CARMEN_PANLASER_MESSAGES_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
  double x;
  double y;
  double z;
  double roll;
  double pitch;
  double yaw;
} carmen_pose_t, *carmen_pose_p;

typedef struct {
  carmen_pose_t pose;
  int num_readings;
  float *range;
  double timestamp;
  char host[10];
} carmen_panlaser_scan_message;

#define CARMEN_PANLASER_SCAN_NAME    "carmen_panlaser_scan"
#define CARMEN_PANLASER_SCAN_FMT     "{<{double,double,double,double,double,double}:1>,int,<float:1>,double,[char:10]}"


#ifdef __cplusplus
}
#endif

#endif
