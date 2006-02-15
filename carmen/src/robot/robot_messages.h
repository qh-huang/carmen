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

/** @addtogroup robot **/
// @{

/** \file robot_messages.h
 * \brief Definition of the messages for this module.
 *
 * This file specifies the messages for this modules used to transmit
 * data via ipc to other modules.
 **/


#ifndef CARMEN_ROBOT_MESSAGES_H
#define CARMEN_ROBOT_MESSAGES_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
  int num_sonars;
  double sensor_angle;          //width of sonar cone
  double *ranges;
  carmen_point_t *positions;
  carmen_point_t robot_pose;
  double tv, rv;
  double timestamp;
  char *host;
} carmen_robot_sonar_message;

#define      CARMEN_ROBOT_SONAR_NAME            "robot_sonar"
#define      CARMEN_ROBOT_SONAR_FMT             "{int,double,<double:1>,<{double,double,double}:1>,{double,double,double},double,double,double,string}"

typedef struct { 
  int num_bumpers;
  char *state;
  carmen_point_t robot_pose; //position of the center of the robot
  double tv, rv;
  double timestamp;
  char *host;
} carmen_robot_bumper_message;

#define CARMEN_ROBOT_BUMPER_NAME "carmen_robot_bumper"
#define CARMEN_ROBOT_BUMPER_FMT "{int, <char:1>, {double,double,double}, double, double, double, string}"
  
typedef struct {
  carmen_laser_laser_config_t config;
  int num_readings;
  float *range;
  char *tooclose;
  int num_remissions;
  float *remission;
  carmen_point_t laser_pose; //position of the center of the laser
  carmen_point_t robot_pose; //position of the center of the robot
  double tv, rv;
  double forward_safety_dist, side_safety_dist;
  double turn_axis;
  double timestamp;
  char *host;
} carmen_robot_laser_message;

#define      CARMEN_ROBOT_FRONTLASER_NAME       "carmen_robot_frontlaser"
#define      CARMEN_ROBOT_FRONTLASER_FMT        "{{int,double,double,double,double,double,int},int,<float:2>,<char:2>,int,<float:5>,{double,double,double},{double,double,double},double,double,double,double,double,double,string}"

#define      CARMEN_ROBOT_REARLASER_NAME        "carmen_robot_rearlaser"
#define      CARMEN_ROBOT_REARLASER_FMT         "{{int,double,double,double,double,double,int},int,<float:2>,<char:2>,int,<float:5>,double,double,double,double,double,double,double,double,double,double,double,double,string}"

typedef struct {
  double vector_distance;
  double vector_angle;
  double timestamp;
  char *host;
} carmen_robot_vector_status_message;

#define CARMEN_ROBOT_VECTOR_STATUS_NAME "carmen_robot_vector_status"
#define CARMEN_ROBOT_VECTOR_STATUS_FMT "{double, double, double, string}"
  
typedef struct {
  double tv, rv;
  double timestamp;
  char *host;
} carmen_robot_velocity_message;

#define      CARMEN_ROBOT_VELOCITY_NAME         "carmen_robot_vel"
#define      CARMEN_ROBOT_VELOCITY_FMT          "{double,double,double,string}"

typedef struct {
  int trajectory_length;
  carmen_traj_point_t *trajectory;
  carmen_traj_point_t robot_position;
  double timestamp;
  char *host;
} carmen_robot_follow_trajectory_message;

#define      CARMEN_ROBOT_FOLLOW_TRAJECTORY_NAME         "carmen_robot_follow_trajectory"
#define      CARMEN_ROBOT_FOLLOW_TRAJECTORY_FMT          "{int,<{double,double,double,double,double}:1>,{double,double,double,double,double},double,string}"

typedef struct {
  double distance;
  double theta;
  double timestamp;
  char *host;
} carmen_robot_vector_move_message;

#define      CARMEN_ROBOT_VECTOR_MOVE_NAME         "carmen_robot_vector_move"
#define      CARMEN_ROBOT_VECTOR_MOVE_FMT          "{double,double,double,string}"

#ifdef __cplusplus
}
#endif

#endif

// @}
