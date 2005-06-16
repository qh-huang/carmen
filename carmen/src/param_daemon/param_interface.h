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

#ifndef CARMEN_PARAMETER_INTERFACE_H
#define CARMEN_PARAMETER_INTERFACE_H

#include <carmen/param_messages.h>


#ifdef __cplusplus
extern "C" {
#endif

#define carmen_param_handle_error(error, usage, progname) {if ((error) < 0) usage(progname, carmen_param_get_error());}

typedef enum {CARMEN_PARAM_INT, CARMEN_PARAM_DOUBLE, CARMEN_PARAM_ONOFF, CARMEN_PARAM_STRING,
	      CARMEN_PARAM_FILE, CARMEN_PARAM_DIR} carmen_param_type_t;

typedef void (*carmen_param_change_handler_t)(char *module, char *variable, char *value);

typedef struct {
  char *module;
  char *variable;
  carmen_param_type_t type;
  void *user_variable;
  int subscribe;
  carmen_param_change_handler_t handler;
} carmen_param_t, *carmen_param_p;

char *carmen_param_get_robot(void);
int carmen_param_get_modules(char ***modules, int *num_modules);
char *carmen_param_get_module(void);
int carmen_param_get_paramserver_host(char **hostname);
int carmen_param_get_all(char *module, char ***variables, char ***values, 
			 int *list_length);
int carmen_param_get_int(char *variable, int *return_value);
int carmen_param_get_double(char *variable, double *return_value);
int carmen_param_get_onoff(char *variable, int *return_value);
int carmen_param_get_string(char *variable, char **return_value);
int carmen_param_get_filename(char *variable, char  **return_value);

void carmen_param_set_module(char *new_module_name);
int carmen_param_set_variable(char *variable, char *new_value, 
			      char **return_value);
int carmen_param_set_int(char *variable, int new_value, int *return_value);
int carmen_param_set_double(char *variable, double  new_value, 
			    double *return_value);
int carmen_param_set_onoff(char *variable, int new_value, int *return_value);
int carmen_param_set_string(char *variable, char *new_value, 
			    char **return_value);
int carmen_param_set_filename(char *variable, char *new_value,
			      char **return_value);

char *carmen_param_get_error(void);
void carmen_param_allow_unfound_variables(int new_value);
int carmen_param_are_unfound_variables_allowed(void);

void carmen_param_set_usage_line(char *fmt, ...);
void carmen_param_usage(char *progname, carmen_param_p param_list, 
			int num_items, char *fmt, ...);

int carmen_param_install_params(int argc, char *argv[], 
				 carmen_param_p param_list, 
				 int num_items);

void carmen_param_check_unhandled_commandline_args(int argc, char *argv[]);

void carmen_param_subscribe_int(char *module, char *variable, 
				int *variable_address, 
				carmen_param_change_handler_t handler);
void carmen_param_subscribe_double(char *module, char *variable, 
				   double *variable_address, 
				   carmen_param_change_handler_t handler);
void carmen_param_subscribe_onoff(char *module, char *variable, 
				  int *variable_address, 
				  carmen_param_change_handler_t handler);
void carmen_param_subscribe_string(char *module, char *variable, 
				   char **variable_address, 
				   carmen_param_change_handler_t handler);
void carmen_param_subscribe_file(char *module, char *variable, 
				 char **variable_address, 
				 carmen_param_change_handler_t handler);
void carmen_param_subscribe_dir(char *module, char *variable, 
				char **variable_address, 
				carmen_param_change_handler_t handler);

int carmen_param_check_version(char *prog_name);

#ifdef __cplusplus
}
#endif

#endif
