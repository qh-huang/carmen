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

#ifndef CARMEN_LOCALIZE_MESSAGES_H
#define CARMEN_LOCALIZE_MESSAGES_H

#ifdef __cplusplus
extern "C" {
#endif

/* Initialization message for localize */

#define CARMEN_INITIALIZE_UNIFORM     1
#define CARMEN_INITIALIZE_GAUSSIAN    2

typedef struct {
  double timestamp;
  char *host;
  int distribution;
  int num_modes;
  carmen_point_t *mean, *std;
} carmen_localize_initialize_message;

#define CARMEN_LOCALIZE_INITIALIZE_NAME  "carmen_localize_initialize"
#define CARMEN_LOCALIZE_INITIALIZE_FMT   "{double,string,int,int,<{double,double,double}:4>,<{double,double,double}:4>}"

  /* initialize by map placename */

typedef struct {
  double timestamp;
  char *host;
  char *placename;
} carmen_localize_initialize_placename_message;

#define CARMEN_LOCALIZE_INITIALIZE_PLACENAME_NAME "carmen_localize_initialize_placename"
#define CARMEN_LOCALIZE_INITIALIZE_PLACENAME_FMT "{double,string,string}"

/* Contains the mean and standard deviation of the position of the robot */

typedef struct {
  double timestamp;
  char *host;
  carmen_point_t globalpos, globalpos_std;
  carmen_point_t odometrypos;
  double globalpos_xy_cov;
  int converged;
} carmen_localize_globalpos_message;

#define CARMEN_LOCALIZE_GLOBALPOS_NAME "carmen_localize_globalpos"
#define CARMEN_LOCALIZE_GLOBALPOS_FMT  "{double,string,{double,double,double},{double,double,double},{double,double,double},double,int}"

/* particle message */

typedef struct {
  float x, y, theta, weight;
} carmen_localize_particle_ipc_t, *carmen_localize_particle_ipc_p;

typedef struct {
  double timestamp;
  char *host;
  int num_particles;
  carmen_localize_particle_ipc_p particles;
  carmen_point_t globalpos, globalpos_std;
  double globalpos_xy_cov;
} carmen_localize_particle_message;

#define CARMEN_LOCALIZE_PARTICLE_NAME "carmen_localize_particle"
#define CARMEN_LOCALIZE_PARTICLE_FMT  "{double,string,int,<{float,float,float,float}:3>,{double,double,double},{double,double,double},double}"

/* sensor message in localize coordinates */

typedef struct {
  double timestamp;
  char *host;
  carmen_laser_laser_config_t config;
  int num_readings, laser_skip;
  float *range;
  char *mask;
  carmen_point_t pose;
  int num_laser;
} carmen_localize_sensor_message;

#define CARMEN_LOCALIZE_SENSOR_NAME "carmen_localize_sensor"
#define CARMEN_LOCALIZE_SENSOR_FMT  "{double,string,{int,double,double,double,double,double,int},int,int,<float:4>,<char:4>,{double,double,double},int}"

typedef struct {
  double timestamp;
  char *host;
  int global;
} carmen_localize_map_query_message;
  
#define CARMEN_LOCALIZE_MAP_QUERY_NAME "carmen_localize_map_query"
#define CARMEN_LOCALIZE_MAP_QUERY_FMT "{double,string,int}"
  
typedef struct {
  double timestamp;
  char *host;
  unsigned char *data;    
  int size;
  carmen_map_config_t config;
  int compressed;
  int global;
} carmen_localize_map_message;  
  
#define CARMEN_LOCALIZE_MAP_NAME "carmen_localize_map"
#define CARMEN_LOCALIZE_MAP_FMT  "{double,string,<char:4>,int,{int,int,double,string},int,int}"

#define CARMEN_LOCALIZE_GLOBALPOS_QUERY_NAME "carmen_localize_globalpos_query"
typedef carmen_default_message carmen_localize_globalpos_query_message;

#ifdef __cplusplus
}
#endif

#endif
