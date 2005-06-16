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

typedef struct {
  char *module_name;
  char *variable_name;
  double timestamp;
  char host[10];
} carmen_param_query_message;

#define CARMEN_PARAM_QUERY_FMT     "{string, string, double, [char:10]}"

#define CARMEN_PARAM_QUERY_ROBOT_NAME  "carmen_param_query_robot"

typedef struct {
  char *robot;
  carmen_param_status_t status;
  double timestamp;
  char host[10];
} carmen_param_response_robot_message;

#define CARMEN_PARAM_RESPONSE_ROBOT_NAME  "carmen_param_respond_robot"
#define CARMEN_PARAM_RESPONSE_ROBOT_FMT  "{string, int, double, [char:10]}"

#define CARMEN_PARAM_QUERY_MODULES_NAME   "carmen_param_query_modules"

typedef struct {
  char **modules;
  int num_modules;
  carmen_param_status_t status;
  double timestamp;
  char host[10];
} carmen_param_response_modules_message;

#define CARMEN_PARAM_RESPONSE_MODULES_NAME  "carmen_param_respond_modules"
#define CARMEN_PARAM_RESPONSE_MODULES_FMT  "{<string:2>, int, int, double, [char:10]}"

#define CARMEN_PARAM_QUERY_ALL_NAME    "carmen_param_query_all"

typedef struct {
  char *module_name;
  int list_length;
  char **variables;
  char **values;
  carmen_param_status_t status;
  double timestamp;
  char host[10];
} carmen_param_response_all_message;

#define CARMEN_PARAM_RESPONSE_ALL_NAME     "carmen_param_respond_all"
#define CARMEN_PARAM_RESPONSE_ALL_FMT "{string, int, <string:2>, <string:2>, int, double, [char:10]}"

#define CARMEN_PARAM_QUERY_INT_NAME    "carmen_param_query_int"

typedef struct {
  char *module_name;
  char *variable_name;
  int value;
  carmen_param_status_t status;
  double timestamp;
  char host[10];
} carmen_param_response_int_message;

#define CARMEN_PARAM_RESPONSE_INT_NAME    "carmen_param_respond_int"
#define CARMEN_PARAM_RESPONSE_INT_FMT     "{string, string, int, int, double, [char:10]}"

#define CARMEN_PARAM_QUERY_DOUBLE_NAME    "carmen_param_query_double"

typedef struct {
  char *module_name;
  char *variable_name;
  double value;
  carmen_param_status_t status;
  double timestamp;
  char host[10];
} carmen_param_response_double_message;

#define CARMEN_PARAM_RESPONSE_DOUBLE_NAME    "carmen_param_respond_double"
#define CARMEN_PARAM_RESPONSE_DOUBLE_FMT     "{string, string, double, int, double, [char:10]}"

#define CARMEN_PARAM_QUERY_ONOFF_NAME    "carmen_param_query_onoff"

typedef struct {
  char *module_name;
  char *variable_name;
  int value;
  carmen_param_status_t status;
  double timestamp;
  char host[10];
} carmen_param_response_onoff_message;

#define CARMEN_PARAM_RESPONSE_ONOFF_NAME    "carmen_param_respond_onoff"
#define CARMEN_PARAM_RESPONSE_ONOFF_FMT     "{string, string, int, int, double, [char:10]}"

#define CARMEN_PARAM_QUERY_STRING_NAME    "carmen_param_query_string"

typedef struct {
  char *module_name;
  char *variable_name;
  char *value;
  carmen_param_status_t status;
  double timestamp;
  char host[10];
} carmen_param_response_string_message;

#define CARMEN_PARAM_RESPONSE_STRING_NAME    "carmen_param_respond_string"
#define CARMEN_PARAM_RESPONSE_STRING_FMT     "{string, string, string, int, double, [char:10]}"


#define CARMEN_PARAM_QUERY_FILENAME_NAME    "carmen_param_query_filename"

typedef struct {
  char *module_name;
  char *variable_name;
  char *filename;
  carmen_param_status_t status;
  double timestamp;
  char host[10];
} carmen_param_response_filename_message;

#define CARMEN_PARAM_RESPONSE_FILENAME_NAME    "carmen_param_respond_filename"
#define CARMEN_PARAM_RESPONSE_FILENAME_FMT     "{string, string, string, int, double, [char:10]}"


typedef struct {
  char *module_name;
  char *variable_name;
  char *value;
  double timestamp;
  char host[10];
} carmen_param_set_message;

#define CARMEN_PARAM_SET_NAME    "carmen_param_set"
#define CARMEN_PARAM_SET_FMT     "{string, string, string, double, [char:10]}"

typedef carmen_param_response_string_message carmen_param_variable_change_message;

#define CARMEN_PARAM_VARIABLE_CHANGE_NAME    "carmen_param_variable_change"
#define CARMEN_PARAM_VARIABLE_CHANGE_FMT     "{string, string, string, int, double, [char:10]}"

typedef struct {
  double timestamp;
  char host[10];
} carmen_param_version_query_message;

#define CARMEN_PARAM_VERSION_QUERY_NAME "carmen_param_query_version"
#define CARMEN_PARAM_VERSION_QUERY_FMT  "{double, [char:10]}"

typedef struct {
  int major;
  int minor;
  int revision;
  double timestamp;
  char host[10];
} carmen_param_version_message;

#define CARMEN_PARAM_VERSION_NAME "carmen_param_version"
#define CARMEN_PARAM_VERSION_FMT  "{int, int, int, double, [char:10]}"

#ifdef __cplusplus
}
#endif

#endif
