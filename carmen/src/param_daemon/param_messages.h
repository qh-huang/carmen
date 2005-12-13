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

#ifndef CARMEN_PARAM_MESSAGES_H
#define CARMEN_PARAM_MESSAGES_H

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {CARMEN_PARAM_OK, CARMEN_PARAM_NOT_FOUND, CARMEN_PARAM_NOT_INT, 
	      CARMEN_PARAM_NOT_DOUBLE, CARMEN_PARAM_NOT_ONOFF,
	      CARMEN_PARAM_NOT_FILE, CARMEN_PARAM_FILE_ERR} carmen_param_status_t;

#define CARMEN_PARAM_QUERY_ALL_NAME       "carmen_param_query_all"
#define CARMEN_PARAM_QUERY_INT_NAME       "carmen_param_query_int"
#define CARMEN_PARAM_QUERY_DOUBLE_NAME    "carmen_param_query_double"
#define CARMEN_PARAM_QUERY_ONOFF_NAME     "carmen_param_query_onoff"
#define CARMEN_PARAM_QUERY_STRING_NAME    "carmen_param_query_string"
#define CARMEN_PARAM_VERSION_QUERY_NAME   "carmen_param_query_version"
typedef carmen_default_message carmen_param_query_version_message;

typedef struct {
  double timestamp;
  char *host;
  char *module_name;
  char *variable_name;
} carmen_param_query_message;

#define CARMEN_PARAM_QUERY_ROBOT_NAME  "carmen_param_query_robot"
#define CARMEN_PARAM_QUERY_FMT     "{double,string,string,string}"

typedef struct {
  double timestamp;
  char *host;
  char *robot;
  carmen_param_status_t status;
} carmen_param_response_robot_message;

#define CARMEN_PARAM_RESPONSE_ROBOT_NAME  "carmen_param_respond_robot"
#define CARMEN_PARAM_RESPONSE_ROBOT_FMT  "{double,string,string,int}"

#define CARMEN_PARAM_QUERY_MODULES_NAME   "carmen_param_query_modules"

typedef struct {
  double timestamp;
  char *host;
  char **modules;
  int num_modules;
  carmen_param_status_t status;
} carmen_param_response_modules_message;

#define CARMEN_PARAM_RESPONSE_MODULES_NAME  "carmen_param_respond_modules"
#define CARMEN_PARAM_RESPONSE_MODULES_FMT  "{double,string,<string:4>, int, int}"

typedef struct {
  double timestamp;
  char *host;
  char *module_name;
  int list_length;
  char **variables;
  char **values;
  carmen_param_status_t status;
} carmen_param_response_all_message;

#define CARMEN_PARAM_RESPONSE_ALL_NAME     "carmen_param_respond_all"
#define CARMEN_PARAM_RESPONSE_ALL_FMT "{double, string, string, int, <string:4>, <string:4>, int}"

typedef struct {
  double timestamp;
  char *host;
  char *module_name;
  char *variable_name;
  int value;
  carmen_param_status_t status;
} carmen_param_response_int_message;

#define CARMEN_PARAM_RESPONSE_INT_NAME    "carmen_param_respond_int"
#define CARMEN_PARAM_RESPONSE_INT_FMT     "{double, string, string, string, int, int}"

typedef struct {
  double timestamp;
  char *host;
  char *module_name;
  char *variable_name;
  double value;
  carmen_param_status_t status;
} carmen_param_response_double_message;

#define CARMEN_PARAM_RESPONSE_DOUBLE_NAME    "carmen_param_respond_double"
#define CARMEN_PARAM_RESPONSE_DOUBLE_FMT     "{double, string, string, string, double, int}"

typedef struct {
  double timestamp;
  char *host;
  char *module_name;
  char *variable_name;
  int value;
  carmen_param_status_t status;
} carmen_param_response_onoff_message;

#define CARMEN_PARAM_RESPONSE_ONOFF_NAME    "carmen_param_respond_onoff"
#define CARMEN_PARAM_RESPONSE_ONOFF_FMT     "{double, string, string, string, int, int}"

typedef struct {
  double timestamp;
  char *host;
  char *module_name;
  char *variable_name;
  char *value;
  carmen_param_status_t status;
} carmen_param_response_string_message;

#define CARMEN_PARAM_RESPONSE_STRING_NAME    "carmen_param_respond_string"
#define CARMEN_PARAM_RESPONSE_STRING_FMT     "{double, string, string, string, string, int}"


typedef struct {
  double timestamp;
  char *host;
  char *module_name;
  char *variable_name;
  char *value;
} carmen_param_set_message;

#define CARMEN_PARAM_SET_NAME    "carmen_param_set"
#define CARMEN_PARAM_SET_FMT     "{double, string, string, string, string}"

typedef carmen_param_response_string_message carmen_param_variable_change_message;

#define CARMEN_PARAM_VARIABLE_CHANGE_NAME    "carmen_param_variable_change"
#define CARMEN_PARAM_VARIABLE_CHANGE_FMT     "{double, string, string, string, string, int"

typedef struct {
  double timestamp;
  char *host;
  int major;
  int minor;
  int revision;
} carmen_param_version_message;

#define CARMEN_PARAM_VERSION_NAME "carmen_param_version"
#define CARMEN_PARAM_VERSION_FMT  "{double, string, int, int, int}"

#ifdef __cplusplus
}
#endif

#endif
