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

typedef enum {SICK_LMS, SICK_PLS, HOKUYO_URG, SIMULATED_LASER, UMKNOWN_PROXIMITY_SENSOR}   
carmen_laser_laser_type_t;

typedef enum {OFF, DIRECT, NORMALIZED}          
carmen_laser_remission_type_t;


typedef struct {
  carmen_laser_laser_type_t  laser_type;  /* what kind of laser is this */
  double start_angle;                     /* angle of the first beam relative */
                                          /* to to the center of the laser */
  double fov;                             /* field of view of the laser */
  double angular_resolution;              /* angular resolution of the laser */
  double maximum_range;                   /* the maximum valid range of a measurement  */
  double accuracy;                        /* error in the range measurements*/

  carmen_laser_remission_type_t remission_mode;  /* if and what kind of remission values are used */

} carmen_laser_laser_config_t;



typedef struct {
  carmen_laser_laser_config_t config;
  int num_readings;
  float *range;
  int num_remissions;
  float *remission;
  double timestamp;
  char *host;
} carmen_laser_laser_message;

#define      CARMEN_LASER_FRONTLASER_NAME       "carmen_laser_frontlaser"
#define      CARMEN_LASER_FRONTLASER_FMT        "{{int,double,double,double,double,double,int},int,<float:2>,int,<float:4>,double,string}"

#define      CARMEN_LASER_REARLASER_NAME        "carmen_laser_rearlaser"
#define      CARMEN_LASER_REARLASER_FMT         "{{int,double,double,double,double,double,int},int,<float:2>,int,<float:4>,double,string}"

#define      CARMEN_LASER_LASER3_NAME           "carmen_laser_laser3"
#define      CARMEN_LASER_LASER3_FMT            "{{int,double,double,double,double,double,int},int,<float:2>,int,<float:4>,double,string}"

#define      CARMEN_LASER_LASER4_NAME           "carmen_laser_laser4"
#define      CARMEN_LASER_LASER4_FMT            "{{int,double,double,double,double,double,int},int,<float:2>,int,<float:4>,double,string}"

typedef struct {
  int frontlaser_stalled;
  int rearlaser_stalled;
  int laser3_stalled;
  int laser4_stalled;
} carmen_laser_alive_message;

#define      CARMEN_LASER_ALIVE_NAME            "carmen_laser_alive"
#define      CARMEN_LASER_ALIVE_FMT             "{int,int,int,int}"


#ifdef __cplusplus
}
#endif

#endif

