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

#ifndef CARMEN_MAP_MESSAGES_H
#define CARMEN_MAP_MESSAGES_H

#include <carmen/map.h>


#ifdef __cplusplus
extern "C" {
#endif

#define CARMEN_HMAP_REQUEST_NAME         "carmen_hmap_request"
typedef carmen_default_message carmen_hmap_request_message;
#define CARMEN_GRIDMAP_REQUEST_NAME      "carmen_gridmap_request"
typedef carmen_default_message carmen_gridmap_request_message;
#define CARMEN_PLACELIST_REQUEST_NAME    "carmen_placelist_request"
typedef carmen_default_message carmen_placelist_request_message;
#define CARMEN_OFFLIMITS_REQUEST_NAME    "carmen_offlimits_request"
typedef carmen_default_message carmen_offlimits_request_message;

typedef struct {
  double timestamp;
  char *host;
  carmen_hmap_t hmap;
} carmen_hmap_message;

#define CARMEN_MAP_HMAP_NAME "carmen_hmap_message"
#define CARMEN_MAP_HMAP_FMT  "{double,string,{int, <string:1>, int, <{int, int, <int:2>, int, <{double, double, double}:4>}:3>}}"

typedef struct {
  double timestamp;
  char *host;

  unsigned char * map;
  int size;
  int compressed;
  carmen_map_config_t config;
  
  char *err_mesg;
} carmen_grid_map_message;

#define CARMEN_MAP_GRIDMAP_NAME    "carmen_grid_map_message"
#define CARMEN_MAP_GRIDMAP_UPDATE_NAME    "carmen_grid_map_update_message"
#define CARMEN_MAP_GRIDMAP_FMT     "{double,string, <char:4>, int, int, {int, int, double, string}, string}"



typedef struct {
  double timestamp;
  char *host;
  char *name;
} carmen_named_gridmap_request;

#define CARMEN_NAMED_GRIDMAP_REQUEST_NAME    "carmen_named_gridmap_request"
#define CARMEN_NAMED_GRIDMAP_REQUEST_FMT     "{double,string,string}"

typedef struct {  
  double timestamp;
  char *host;
  carmen_place_p places;
  int num_places;
} carmen_map_placelist_message;

#define CARMEN_MAP_PLACELIST_NAME    "carmen_placelist_message"
#define CARMEN_MAP_PLACELIST_FMT     "{double,string,<{int,int,[char:22],double,double,double,double,double,double}:4>, int}"

typedef struct {
  double timestamp;
  char *host;
  char *name;
} carmen_map_named_placelist_request;

#define CARMEN_NAMED_PLACELIST_REQUEST_NAME    "carmen_named_placelist_request"
#define CARMEN_NAMED_PLACELIST_REQUEST_FMT     "{double,string,string}"

typedef struct {  
  double timestamp;
  char *host;
  carmen_offlimits_p offlimits_list;
  int list_length;
} carmen_map_offlimits_message;

#define CARMEN_MAP_OFFLIMITS_NAME    "carmen_offlimits_message"
#define CARMEN_MAP_OFFLIMITS_FMT     "{double,string,<{int,int,int,int,int}:4>, int}"


typedef struct {
  double timestamp;
  char *host;
  char *name;
} carmen_map_named_offlimits_request;

#define CARMEN_NAMED_OFFLIMITS_REQUEST_NAME    "carmen_named_offlimits_request"
#define CARMEN_NAMED_OFFLIMITS_REQUEST_FMT     "{double,string,string}"

typedef struct {
  double timestamp;
  char *host;
  char *zone_name;
} carmen_map_zone_message;

#define CARMEN_MAP_ZONE_NAME                   "carmen_map_zone_message"
#define CARMEN_MAP_ZONE_FMT                    "{double,string,string}"

typedef struct {
  double timestamp;
  char *host;
  char *zone_name;
} carmen_map_change_map_zone_request;

#define CARMEN_CHANGE_MAP_ZONE_REQUEST_NAME    "carmen_change_map_zone_request"
#define CARMEN_CHANGE_MAP_ZONE_REQUEST_FMT     "{double,string,string}"

typedef struct {
  double timestamp;
  char *host;
  char *err_msg;
} carmen_map_change_map_zone_response;

#define CARMEN_CHANGE_MAP_ZONE_RESPONSE_NAME   "carmen_change_map_zone_response"
#define CARMEN_CHANGE_MAP_ZONE_RESPONSE_FMT    "{double,string,string}"


#ifdef __cplusplus
}
#endif

#endif
