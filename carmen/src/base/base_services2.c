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
 * Public License along with Foobar; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place, 
 * Suite 330, Boston, MA  02111-1307 USA
 *
 ********************************************************/

#include <carmen/carmen.h>
#include <dlfcn.h>
#include <laser_main.h>
#include <robot_main.h>
#include "base_low_level.h"

typedef enum {NONE, RESET, SET_VELOCITY, STOP, SONAR_ON, SONAR_OFF} command_t;
#define COMMAND_STACK_CAPACITY 100;

#ifdef _REENTRANT
#include <pthread.h>
static pthread_t main_thread;
static pthread_mutex_t state_mutex;
#endif

static char* base_type = NULL;
static void* dl_handle = NULL;

typedef int (SONAR_ON_TYPE)(void);
typedef int (SONAR_OFF_TYPE)(void);
typedef int (RESET_TYPE)(void);
typedef int (INITIALIZE_ROBOT_TYPE)(char *dev, int go_backwards);
typedef int (SHUTDOWN_ROBOT_TYPE)(void);
typedef int (SET_ACCELERATION_TYPE)(double acceleration);
typedef int (SET_DECELERATION_TYPE)(double acceleration);
typedef int (SET_VELOCITY_TYPE)(double tv, double rv);
typedef int (UPDATE_STATUS_TYPE)(void);
typedef void (GET_STATE_TYPE)(carmen_base_odometry_message *odometry,
			      double *ranges);

static SONAR_ON_TYPE* base_sonar_on = NULL;
static SONAR_OFF_TYPE* base_sonar_off = NULL;
static RESET_TYPE* base_reset = NULL;
static INITIALIZE_ROBOT_TYPE* base_initialize_robot = NULL;
static SHUTDOWN_ROBOT_TYPE* base_shutdown_robot = NULL;
static SET_ACCELERATION_TYPE* base_set_acceleration = NULL;
static SET_DECELERATION_TYPE* base_set_deceleration = NULL;
static SET_VELOCITY_TYPE* base_set_velocity = NULL;
static UPDATE_STATUS_TYPE* base_update_status = NULL;
static GET_STATE_TYPE* base_get_state = NULL;

static int robot_is_ok = 0;
static double timeout = 2.0;
static double tv, rv;
static double deceleration;
static carmen_robot_config_t robot_config;
static carmen_base_odometry_message odometry;
static double reset_time = 0;

static int use_sonar = 1;
static carmen_base_sonar_message sonar;
static int sonar_state = 0;
static double *ranges;
static int num_sonar_ranges;

static command_t *command_stack;
static int command_stack_size;
static int command_stack_capacity;

/*
 * Everything that occurs in the following section (to the next commented
 * division) runs in the base thread, which is started separately. No
 * function in this section should contain IPC calls, but instead only
 * contain calls to the robot base.
 *
 */

static command_t
pop_command(void)
{
  command_t command;

  if (command_stack_size <= 0)
    return NONE;

  command = command_stack[0];

#ifdef _REENTRANT
  pthread_mutex_lock(&state_mutex);
#endif 

  if (command_stack_size > 1) 
    {
      memcpy(command_stack, command_stack+1, 
	     (command_stack_size-1)*sizeof(command_t));
      command_stack_size--;
    }
  
#ifdef _REENTRANT
  pthread_mutex_unlock(&state_mutex);
#endif 

  return command;
}

static void
push_command(command_t command) 
{
  if (command_stack_size == command_stack_capacity) 
    {
      carmen_die("We have fallen too far behind in commands being sent to\n"
		 "the robot, vs. commands being received via IPC.\n"
		 "We have to exit.\n");
    }

#ifdef _REENTRANT
  pthread_mutex_lock(&state_mutex);
#endif 

  command_stack[command_stack_size++] = command;

#ifdef _REENTRANT
  pthread_mutex_unlock(&state_mutex);
#endif 
}

static void 
command_robot(void)
{
  static double last_command = 0.0;
  static int moving = 0;
  static int timeout_watch = 0;
  static double current_acceleration = 0;
  
  command_t command;
  int err = 0;
  double time_elapsed = carmen_get_time() - last_command;
  double current_time; 

  while (!robot_is_ok) {
    err = base_reset();
    if (!err) {
      robot_is_ok = 1;
      reset_time = carmen_get_time();
    }
  }

  if (moving && current_acceleration == robot_config.acceleration)
    {
      base_set_deceleration(deceleration);
      current_acceleration = deceleration;
    }  

  while (command_stack_size > 0)
    {
      command = pop_command();
      switch (command) 
	{
	case NONE: 
	  break;
	case RESET:
	  err = base_reset();
	  if (err < 0) 
	    robot_is_ok = 0;
	  
	  reset_time = carmen_get_time();
	  break;
	case SET_VELOCITY:   
	  if(tv == 0 && rv == 0) 
	    {
	      if (moving) {
		err = base_set_deceleration(deceleration);
		if (err < 0)
		  robot_is_ok = 0;
		moving = 0;
	      }
	      carmen_warn("S");
	    }
	  else if (!moving) 
	    {
	      moving = 1;
	      current_acceleration = robot_config.acceleration;
	      err = base_set_acceleration(current_acceleration);
	      if (err < 0) 
		robot_is_ok = 0;
	      carmen_warn("V");
	    }
	  
	  err = base_set_velocity(tv, rv);
	  if (err < 0) 
	    robot_is_ok = 0;
	  timeout_watch = 1;
	  break;
	case SONAR_ON:
	  if (sonar_state == 0) {
	    num_sonar_ranges = base_sonar_on();
	    ranges = (double *)calloc(num_sonar_ranges, sizeof(double));
	    carmen_test_alloc(ranges);
	  }
	  if (num_sonar_ranges < 0)
	    err = num_sonar_ranges;
	  else {
	    err = 0;
	    sonar_state = 1;
	  }
	  break;
	case SONAR_OFF:
	  if (sonar_state == 1) {
	    err = base_sonar_off();
	    free(ranges);
	    ranges = NULL;
	    num_sonar_ranges = 0;
	    sonar_state = 0;
	  }
	  break;
	default:    
	  if (time_elapsed > timeout && timeout_watch) 
	    {
	      carmen_warn("T");
	      timeout = 1;
	      moving = 0;
	      err = base_set_velocity(0, 0);
	    }  
	  else
	    {
	      base_update_status();
	    }
	} /* End of switch(command) */
    } /* End of while (command_stack_size > 0) */

  current_time = carmen_get_time();

  odometry.timestamp = current_time;

  if (err < 0) {    
    robot_is_ok = 0;
    return;
  }

#ifdef _REENTRANT
  pthread_mutex_lock(&state_mutex);
#endif 

  base_get_state(&odometry, ranges);

  if (err < 0) {
    robot_is_ok = 0;
    return;
  }

#ifdef _REENTRANT
  pthread_mutex_unlock(&state_mutex);
  pthread_testcancel();
#endif 
}


#ifdef _REENTRANT
static void *
start_thread(void *data __attribute__ ((unused))) 
{
  sigset_t set;

  sigemptyset(&set);
  sigaddset(&set, SIGINT);
  sigprocmask(SIG_BLOCK, &set, NULL);

  while (1) {    
    command_robot();
    usleep(30000);
  }

  return NULL;
}  
#endif

/*
 * Everything that occurs above this point runs in the robot thread,
 * which is started separately. No function above this point should contain
 * IPC calls. 
 *
 * Everything that occurs below this point runs in the main thread, which
 * handles IPC, etc. No function below this point should contain calls to
 * the robot base.
 *
 */

static int 
initialize_robot(char *dev, int backwards_flag)
{
  int result;
  char *host;

  result = base_initialize_robot(dev, backwards_flag);
  if (result < 0) {
    robot_is_ok = 0;
    return -1;
  }

  if (use_sonar) 
    {      
      num_sonar_ranges = base_sonar_on();
      if(num_sonar_ranges < 0) {
	robot_is_ok = 0;
	return -1;
      }
      ranges = (double *)calloc(num_sonar_ranges, sizeof(double));
      carmen_test_alloc(ranges);
    }

  result = base_reset();
  if (result < 0) {
    robot_is_ok = 0;
    return -1;
  }

  reset_time = carmen_get_time();

  host = carmen_get_tenchar_host_name();
  strcpy(odometry.host, host);     

  command_stack_capacity = COMMAND_STACK_CAPACITY;
  command_stack = (command_t *)
    calloc(command_stack_capacity, sizeof(command_t));
  carmen_test_alloc(command_stack);
  command_stack_size = 0;
  
  robot_is_ok = 1;
  return 0;
}

static void
initialize_sonar_message(carmen_base_sonar_message *sonar)
{
  char *offset_string;
  int num_sonars;
  double sensor_angle;
  int err;

  carmen_param_set_module("robot");
  carmen_param_allow_unfound_variables(0);

  err = carmen_param_get_int("num_sonars", &num_sonars);
  if (err < 0)
    carmen_die("%s", carmen_param_get_error());

  err = carmen_param_get_double("sensor_angle", &sensor_angle);
  if (err < 0)
    carmen_die("%s", carmen_param_get_error());

  err = carmen_param_get_string("sonar_offsets", &offset_string);
  if (err < 0)
    carmen_die("%s", carmen_param_get_error());
  
#ifdef _REENTRANT
  pthread_mutex_lock(&state_mutex);
#endif 

  sonar->range = (double *)calloc(num_sonars, sizeof(double));
  carmen_test_alloc(sonar->range);
  sonar->sonar_offsets = (carmen_point_p)
    calloc(num_sonars, sizeof(carmen_point_t));
  carmen_test_alloc(sonar->sonar_offsets);
  
  carmen_parse_sonar_offsets(offset_string, sonar->sonar_offsets, num_sonars);
  free(offset_string);
  
  sonar->sensor_angle = sensor_angle;
  sonar->timestamp = 0.0;
  strncpy(sonar->host, odometry.host, 10);    

#ifdef _REENTRANT
  pthread_mutex_unlock(&state_mutex);
#endif 
}

static void
handle_sonar_change(char *module __attribute__ ((unused)), 
		    char *variable __attribute__ ((unused)), 
		    char *value __attribute__ ((unused)))
{
  if (use_sonar && !sonar_state) 
    {
      initialize_sonar_message(&sonar);
      push_command(SONAR_ON);
    } 
  else if (sonar_state) 
    {
#ifdef _REENTRANT
      pthread_mutex_lock(&state_mutex);
#endif 
      if (sonar.range) 
	free(sonar.range);
      if (sonar.sonar_offsets)
	free(sonar.sonar_offsets);
#ifdef _REENTRANT
      pthread_mutex_unlock(&state_mutex);
#endif 
      push_command(SONAR_OFF);
    } /* end of if (use_sonar && !sonar_state) ... else if (sonar_state) */
}

static int
read_parameters(int argc, char **argv, char **dev_name, int *backwards)
{
  int num_items;

  carmen_param_t param_list[] = {
    {"base", "dev", CARMEN_PARAM_STRING, dev_name, 0, NULL},
    {"robot", "backwards", CARMEN_PARAM_ONOFF, backwards, 0, NULL},
    {"robot", "use_sonar", CARMEN_PARAM_ONOFF, &use_sonar, 1, 
     handle_sonar_change}, 
    {"robot", "acceleration", CARMEN_PARAM_DOUBLE, 
     &(robot_config.acceleration), 1, NULL},
    {"robot", "deceleration", CARMEN_PARAM_DOUBLE, 
     &(deceleration), 1, NULL}};

  num_items = sizeof(param_list)/sizeof(param_list[0]);
  carmen_param_install_params(argc, argv, param_list, num_items);

  if (use_sonar)
    initialize_sonar_message(&sonar);
  else
    memset(&sonar, 0, sizeof(carmen_base_sonar_message));

  if (robot_config.acceleration > deceleration) 
    carmen_die("ERROR: robot_deceleration must be greater or equal than "
	       "robot_acceleration\n");
  
  return 0;
}

static void 
velocity_handler(MSG_INSTANCE msgRef, BYTE_ARRAY callData,
		 void *clientData __attribute__ ((unused)))
{
  IPC_RETURN_TYPE err;
  carmen_base_velocity_message vel;

  FORMATTER_PTR formatter;
  
  formatter = IPC_msgInstanceFormatter(msgRef);
  err = IPC_unmarshallData(formatter, callData, &vel,
                     sizeof(carmen_base_velocity_message));
  IPC_freeByteArray(callData);

  carmen_test_ipc_return(err, "Could not unmarshall", 
			 IPC_msgInstanceName(msgRef));

  tv = vel.tv;
  rv = vel.rv;
  push_command(SET_VELOCITY);
}

static void 
reset_handler(MSG_INSTANCE msgRef, BYTE_ARRAY callData,
	      void *clientData __attribute__ ((unused)))
{
  IPC_RETURN_TYPE err;
  carmen_base_reset_message msg;

  FORMATTER_PTR formatter;
  
  formatter = IPC_msgInstanceFormatter(msgRef);
  err = IPC_unmarshallData(formatter, callData, &msg,
			   sizeof(carmen_base_reset_message));
  IPC_freeByteArray(callData);

  carmen_test_ipc_return(err, "Could not unmarshall", 
			 IPC_msgInstanceName(msgRef));

  push_command(RESET);
}

int 
carmen_base_initialize_ipc(void)
{
  IPC_RETURN_TYPE err;

  /* define messages created by base */
  err = IPC_defineMsg(CARMEN_BASE_ODOMETRY_NAME, IPC_VARIABLE_LENGTH,
                      CARMEN_BASE_ODOMETRY_FMT);
  carmen_test_ipc_exit(err, "Could not define", CARMEN_BASE_ODOMETRY_NAME);

  err = IPC_defineMsg(CARMEN_BASE_SONAR_NAME, IPC_VARIABLE_LENGTH,
                      CARMEN_BASE_SONAR_FMT);
  carmen_test_ipc_exit(err, "Could not define IPC message", 
		       CARMEN_BASE_SONAR_NAME);

  err = IPC_defineMsg(CARMEN_BASE_VELOCITY_NAME, IPC_VARIABLE_LENGTH,
                      CARMEN_BASE_VELOCITY_FMT);
  carmen_test_ipc_exit(err, "Could not define", CARMEN_BASE_VELOCITY_NAME);

  err = IPC_defineMsg(CARMEN_BASE_RESET_NAME, IPC_VARIABLE_LENGTH,
                      CARMEN_BASE_RESET_FMT);
  carmen_test_ipc_exit(err, "Could not define", CARMEN_BASE_RESET_NAME);

  err = IPC_defineMsg(CARMEN_BASE_RESET_COMMAND_NAME, IPC_VARIABLE_LENGTH,
                      CARMEN_BASE_RESET_COMMAND_FMT);
  carmen_test_ipc_exit(err, "Could not define", 
		       CARMEN_BASE_RESET_COMMAND_NAME);

  /* setup incoming message handlers */

  err = IPC_subscribe(CARMEN_BASE_VELOCITY_NAME, velocity_handler, NULL);
  carmen_test_ipc_exit(err, "Could not subscribe", CARMEN_BASE_VELOCITY_NAME);
  IPC_setMsgQueueLength(CARMEN_BASE_VELOCITY_NAME, 1);

  err = IPC_subscribe(CARMEN_BASE_RESET_COMMAND_NAME, reset_handler, NULL);
  carmen_test_ipc_exit(err, "Could not subscribe", 
		       CARMEN_BASE_RESET_COMMAND_NAME);
  IPC_setMsgQueueLength(CARMEN_BASE_RESET_COMMAND_NAME, 1);

  return IPC_No_Error;
}

int 
carmen_base_start(int argc, char **argv)
{
  int backwards;
  char *dev_name;
  
  if (read_parameters(argc, argv, &dev_name, &backwards) < 0)
    return -1;

  if (carmen_base_initialize_ipc() < 0) 
    {
      carmen_warn("\nError: Could not initialize IPC.\n");
      return -1;
    }

  if(initialize_robot(dev_name, backwards) < 0)
    {
      carmen_warn("\nError: Could not connect to robot. "
		  "Did you remember to turn the base on?\n");
      return -1;
    }

  strcpy(odometry.host, carmen_get_tenchar_host_name());

#ifdef _REENTRANT
  pthread_mutex_init(&state_mutex, NULL);
  pthread_create(&main_thread, NULL, start_thread, NULL);
#endif
  return 0;
}

int 
carmen_base_run(void) 
{
  IPC_RETURN_TYPE err;
  int index;
  static carmen_base_odometry_message published_odometry;
  static carmen_base_reset_message reset;  

  if (reset_time > reset.timestamp) {
    reset.timestamp = reset_time;
    err = IPC_publishData(CARMEN_BASE_RESET_NAME, &reset);
    carmen_test_ipc_exit(err, "Could not publish", CARMEN_BASE_RESET_NAME);  
    return 1;
  }
     
#ifdef _REENTRANT
  pthread_mutex_lock(&state_mutex);
#endif 

  memcpy(&published_odometry, &odometry, sizeof(carmen_base_odometry_message));

  if (use_sonar && sonar_state) 
    {
      if (num_sonar_ranges < sonar.num_sonars)
	sonar.num_sonars = num_sonar_ranges;
      for (index = 0; index < sonar.num_sonars; index++) 
	{
	  sonar.range[index] = ranges[index];
	}
    }

#ifdef _REENTRANT
  pthread_mutex_unlock(&state_mutex);
#endif 

  err = IPC_publishData(CARMEN_BASE_ODOMETRY_NAME, &odometry);
  carmen_test_ipc_exit(err, "Could not publish", 
		       CARMEN_BASE_ODOMETRY_NAME);
  
  if (use_sonar)
    {
      err = IPC_publishData(CARMEN_BASE_SONAR_NAME, &sonar);
      carmen_test_ipc_exit(err, "Could not publish", 
			   CARMEN_BASE_SONAR_NAME);
    }
  
#ifndef _REENTRANT
  command_robot();
#endif
  
  return 1;
}

void 
carmen_base_shutdown(int x)
{

#ifdef _REENTRANT
  pthread_cancel(main_thread);
#endif

  if(x == SIGINT) 
    {
      carmen_verbose("\nShutting down robot...");
      sleep(1);
      if (use_sonar && sonar_state)
	base_sonar_off();
      carmen_verbose("done.\n");
    }
}

void 
carmen_base_emergency_crash(int x __attribute__ ((unused)))
{
  base_set_velocity(0.0, 0.0);
}

static int
load_shared_object_library()
{
  char lib_name[128];

  sprintf((char*)&lib_name, "./lib%s.so%c", base_type, 0x00);
  dl_handle = dlopen((char*)&lib_name, RTLD_NOW);
  if (dl_handle != NULL)
    return 0;

  sprintf((char*)&lib_name, "../lib/lib%s.so%c", base_type, 0x00);
  dl_handle = dlopen((char*)&lib_name, RTLD_NOW);
  if (dl_handle != NULL)
    return 0;

  sprintf((char*)&lib_name, "../../lib/lib%s.so%c", base_type, 0x00);
  dl_handle = dlopen((char*)&lib_name, RTLD_NOW);
  if (dl_handle != NULL)
    return 0;

  carmen_warn("Can't load shared object library: %s\n", dlerror());
  return -1;
}

static int
init_shared_object_library()
{
  char sym_name[128];

  if (load_shared_object_library() < 0)
    return -1;

  sprintf((char*)&sym_name, "carmen_sonar_on");
  base_sonar_on = (SONAR_ON_TYPE*)dlsym(dl_handle, (char*)&sym_name);
  if (base_sonar_on == NULL) {
    carmen_warn("Can't find function %s: %s\n", sym_name, dlerror());
    dlclose(dl_handle);
    return -1;
  }

  sprintf((char*)&sym_name, "carmen_sonar_off");
  base_sonar_off = (SONAR_OFF_TYPE*)dlsym(dl_handle, (char*)&sym_name);
  if (base_sonar_off == NULL) {
    carmen_warn("Can't find function %s: %s\n", sym_name, dlerror());
    dlclose(dl_handle);
    return -1;
  }

  sprintf((char*)&sym_name, "carmen_reset");
  base_reset = (RESET_TYPE*)dlsym(dl_handle, (char*)&sym_name);
  if (base_reset == NULL) {
    carmen_warn("Can't find function %s: %s\n", sym_name, dlerror());
    dlclose(dl_handle);
    return -1;
  }

  sprintf((char*)&sym_name, "carmen_initialize_robot");
  base_initialize_robot = (INITIALIZE_ROBOT_TYPE*)
    dlsym(dl_handle, (char*)&sym_name);
  if (base_initialize_robot == NULL) {
    carmen_warn("Can't find function %s: %s\n", sym_name, dlerror());
    dlclose(dl_handle);
    return -1;
  }

  sprintf((char*)&sym_name, "carmen_shutdown_robot");
  base_shutdown_robot = (SHUTDOWN_ROBOT_TYPE*)
    dlsym(dl_handle, (char*)&sym_name);
  if (base_shutdown_robot == NULL) {
    carmen_warn("Can't find function %s: %s\n", sym_name, dlerror());
    dlclose(dl_handle);
    return -1;
  }

  sprintf((char*)&sym_name, "carmen_set_acceleration");
  base_set_acceleration = (SET_ACCELERATION_TYPE*)
    dlsym(dl_handle, (char*)&sym_name);
  if (base_set_acceleration == NULL) {
    carmen_warn("Can't find function %s: %s\n", sym_name, dlerror());
    dlclose(dl_handle);
    return -1;
  }

  sprintf((char*)&sym_name, "carmen_set_deceleration");
  base_set_deceleration = (SET_DECELERATION_TYPE*)
    dlsym(dl_handle, (char*)&sym_name);
  if (base_set_deceleration == NULL) {
    carmen_warn("Can't find function %s: %s\n", sym_name, dlerror());
    dlclose(dl_handle);
    return -1;
  }

  sprintf((char*)&sym_name, "carmen_set_velocity");
  base_set_velocity = (SET_VELOCITY_TYPE*)dlsym(dl_handle, (char*)&sym_name);
  if (base_set_velocity == NULL) {
    carmen_warn("Can't find function %s: %s\n", sym_name, dlerror());
    dlclose(dl_handle);
    return -1;
  }

  sprintf((char*)&sym_name, "carmen_update_status");
  base_update_status = (UPDATE_STATUS_TYPE*)dlsym(dl_handle, (char*)&sym_name);
  if (base_update_status == NULL) {
    carmen_warn("Can't find function %s: %s\n", sym_name, dlerror());
    dlclose(dl_handle);
    return -1;
  }

  sprintf((char*)&sym_name, "carmen_get_state");
  base_get_state = (GET_STATE_TYPE*)dlsym(dl_handle, (char*)&sym_name);
  if (base_get_state == NULL) {
    carmen_warn("Can't find function %s: %s\n", sym_name, dlerror());
    dlclose(dl_handle);
    return -1;
  }

  return 0;
}

void
close_shared_object_library()
{
  if (dl_handle != NULL) {
    dlclose(dl_handle);
    dl_handle = NULL;
  }
}

static int  
read_base_services_parameters() 
{
  int error;

  carmen_param_set_module("base");
  error = carmen_param_get_string("type", &base_type);
  if (error < 0)
    carmen_die("Error in getting base type from parameter server : %s\n"
	       "Are you sure there's a definition for base_type in the "
	       "parameter server?\n", carmen_param_get_error());

  if (base_type == NULL)
    carmen_die("Error in getting base type from parameter server : "
	       "returned value is NULL\n"
	       "Are you sure there's a definition for base_type in the "
	       "parameter server?\n");

  return init_shared_object_library();
}

static void
shutdown_base(int signo)
{
    carmen_base_shutdown(signo);
    carmen_laser_shutdown(signo);
    carmen_robot_shutdown(signo);

    close_shared_object_library();
    exit(-1);
}

int 
main(int argc, char **argv)
{
  carmen_ipc_initialize(argc, argv);
  carmen_param_check_version(argv[0]);

  if (read_base_services_parameters(argc, argv) < 0)
    return -1;

  signal(SIGINT, shutdown_base);
  signal(SIGSEGV, carmen_base_emergency_crash);

  if (carmen_base_start(argc, argv) < 0)
    exit(-1);

  if (carmen_laser_start(argc, argv) < 0) {
    carmen_base_shutdown(SIGTERM);
    exit(-1);
  }
  
  if (carmen_robot_start(argc, argv) < 0)
    exit(-1);

  while(1) {
    fprintf(stderr, ".");
    carmen_ipc_sleep(0.01);

    carmen_base_run();
    carmen_laser_run();
    carmen_robot_run();
  }

  close_shared_object_library();
  return 0;
}


