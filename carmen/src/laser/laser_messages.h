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

#ifndef CARMEN_LASER_MESSAGES_H
#define CARMEN_LASER_MESSAGES_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
  int num_readings;
  float *range;
  double timestamp;
  char host[10];
} carmen_laser_laser_message;

#define      CARMEN_LASER_FRONTLASER_NAME       "carmen_laser_frontlaser"
#define      CARMEN_LASER_FRONTLASER_FMT        "{int,<float:1>,double,[char:10]}"

#define      CARMEN_LASER_REARLASER_NAME        "carmen_laser_rearlaser"
#define      CARMEN_LASER_REARLASER_FMT         "{int,<float:1>,double,[char:10]}"

#define      CARMEN_LASER_LASER3_NAME           "carmen_laser_laser3"
#define      CARMEN_LASER_LASER3_FMT            "{int,<float:1>,double,[char:10]}"

#define      CARMEN_LASER_LASER4_NAME           "carmen_laser_laser4"
#define      CARMEN_LASER_LASER4_FMT            "{int,<float:1>,double,[char:10]}"

typedef struct {
  int frontlaser_stalled;
  int rearlaser_stalled;
  int laser3_stalled;
  int laser4_stalled;
} carmen_laser_alive_message;

#define      CARMEN_LASER_ALIVE_NAME            "carmen_laser_alive"
#define      CARMEN_LASER_ALIVE_FMT             "{int,int,int,int}"


#define      CARMEN_LASER_REMISSION_INVALID_VALUE         -1

typedef struct {
  int num_readings;
  float *range;
  float *remission;
  double timestamp;
  char host[10];
} carmen_laser_remission_message;

#define      CARMEN_LASER_FRONTLASER_REMISSION_NAME       "carmen_laser_frontlaser_remission"
#define      CARMEN_LASER_FRONTLASER_REMISSION_FMT        "{int,<float:1>,<float:1>,double,[char:10]}"

#define      CARMEN_LASER_REARLASER_REMISSION_NAME        "carmen_laser_rearlaser_remission"
#define      CARMEN_LASER_REARLASER_REMISSION_FMT         "{int,<float:1>,<float:1>,double,[char:10]}"

#define      CARMEN_LASER_LASER3_REMISSION_NAME           "carmen_laser_laser3_remission"
#define      CARMEN_LASER_LASER3_REMISSION_FMT            "{int,<float:1>,<float:1>,double,[char:10]}"

#define      CARMEN_LASER_LASER4_REMISSION_NAME           "carmen_laser_laser4_remission"
#define      CARMEN_LASER_LASER4_REMISSION_FMT            "{int,<float:1>,<float:1>,double,[char:10]}"

// *** REI - END *** //


#ifdef __cplusplus
}
#endif

#endif
