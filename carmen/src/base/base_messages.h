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

#ifndef CARMEN_BASE_MESSAGES_H
#define CARMEN_BASE_MESSAGES_H

#ifdef __cplusplus
extern "C" {
#endif

// These messages are generic format messages

#define CARMEN_BASE_SERVO_ARM_QUERY_NAME "carmen_base_servo_arm_query"
typedef carmen_default_message carmen_base_servo_arm_query_message;
#define CARMEN_BASE_RESET_OCCURRED_NAME "carmen_base_reset_occurred"
typedef carmen_default_message carmen_base_reset_occurred_message;
#define CARMEN_BASE_RESET_COMMAND_NAME "carmen_base_reset_command"
typedef carmen_default_message carmen_base_reset_command_message;

typedef struct {
  double timestamp;
  char *host;
  double x, y, theta;
  double tv, rv;
  double acceleration;
} carmen_base_odometry_message;

#define      CARMEN_BASE_ODOMETRY_NAME       "carmen_base_odometry"
#define      CARMEN_BASE_ODOMETRY_FMT        "{double,string,double,double,double,double,double,double}"

typedef struct {
  double timestamp;
  char *host;
  double tv, rv;
} carmen_base_velocity_message;

#define      CARMEN_BASE_VELOCITY_NAME       "carmen_base_velocity"
#define      CARMEN_BASE_VELOCITY_FMT        "{double,string,double,double}"

typedef struct {
  double timestamp;
  char *host;
  double xv;
  double yv;
  double rv;
} carmen_base_holonomic_velocity_message;

#define      CARMEN_BASE_HOLONOMIC_VELOCITY_NAME       "carmen_base_holonomic_velocity"
#define      CARMEN_BASE_HOLONOMIC_VELOCITY_FMT        "{double,double,double,double,[char:10]}"

typedef struct {
  double timestamp;
  char *host;
  int num_sonars;
  double sensor_angle;                    //width of sonar cone
  double *range;
  carmen_point_p positions;
} carmen_base_sonar_message;

#define      CARMEN_BASE_SONAR_NAME          "carmen_base_sonar"
#define      CARMEN_BASE_SONAR_FMT           "{double,string,int,double,<double:3>,<{double,double,double}:3>}"

typedef struct {
  double timestamp;
  char *host;
  int rate;
  int num_sonars;
  int *order;
  carmen_point_t *sonar_offsets;
} carmen_base_sonar_conf_message;

#define      CARMEN_BASE_SONAR_CONF_NAME          "carmen_base_sonar_conf"
#define      CARMEN_BASE_SONAR_CONF_FMT           "{double,string,int,int,<int:3><{double,double,double}:3>}"

typedef struct {
  double timestamp;
  char *host;
  int num_bumpers;
  unsigned char *state;
} carmen_base_bumper_message;

#define      CARMEN_BASE_BUMPER_NAME          "carmen_base_bumper"
#define      CARMEN_BASE_BUMPER_FMT           "{double,string,int,<char:3>}"

typedef struct {
  double timestamp;
  char *host;
  int num_irs;
  double *range;
} carmen_base_ir_message;

#define      CARMEN_BASE_IR_NAME          "carmen_base_ir"
#define      CARMEN_BASE_IR_FMT           "{double,string,int,<double:3>}"

typedef struct {
  double timestamp;
  char *host;
  int history;
  int num_irs;
  int order[32];
} carmen_base_ir_conf_message;

#define      CARMEN_BASE_IR_CONF_NAME          "carmen_base_ir_conf"
#define      CARMEN_BASE_IR_CONF_FMT           "{double,string,int,int,[int:32]}"

typedef struct {
  double timestamp;
  char *host;
  unsigned char *data;
  int size;
} carmen_base_binary_data_message;

#define CARMEN_BASE_BINARY_COMMAND_NAME "carmen_base_binary_command"
#define CARMEN_BASE_BINARY_COMMAND_FMT "{double,string,<char:4>,int}"

#define CARMEN_BASE_BINARY_DATA_NAME "carmen_base_binary_data"
#define CARMEN_BASE_BINARY_DATA_FMT "{double,string,<char:4>,int}"

typedef struct {
  double timestamp;
  char *host;
  double *servos;
  int num_servos;
} carmen_base_servo_message;


#define CARMEN_BASE_SERVO_ARM_COMMAND_NAME  "carmen_base_servo_arm_command"
#define CARMEN_BASE_SERVO_ARM_COMMAND_FMT "{double,string,<double:4>,int}"


typedef struct {
  double timestamp;
  char *host;
  double *servos;
  int num_servos;
  double *servo_currents;
  int num_currents;
  int gripper;
} carmen_base_arm_state_message;

#define CARMEN_BASE_SERVO_ARM_STATE_NAME "carmen_base_arm_state"
#define CARMEN_BASE_SERVO_ARM_STATE_FMT "{double,string,<double:4>,int,<double:6>,int,int}"

#ifdef __cplusplus
}
#endif

#endif



