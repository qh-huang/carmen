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

#include <carmen/carmen.h>
#include <laser_main.h>
#include <robot_main.h>
#include <dlfcn.h>

static char* base_type = NULL;
static void* dl_handle = NULL;

typedef int (START_FUNC_TYPE)(int argc, char **argv);
typedef void(RUN_FUNC_TYPE)(void);
typedef int (SHUTDOWN_FUNC_TYPE)(int signo);
typedef void(EMERGENCY_CRASH_FUNC_TYPE)(int signo);

static START_FUNC_TYPE*           base_start = NULL;
static RUN_FUNC_TYPE*             base_run = NULL;
static SHUTDOWN_FUNC_TYPE*        base_shutdown = NULL;
static EMERGENCY_CRASH_FUNC_TYPE* base_emergency_crash = NULL;

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
  
  carmen_warn("Can't load shared object library lib%s.so in:\n"
	      "./\n../lib\n../../lib/\n\n"
	      "Are you sure you configured carmen to support this base "
	      "type?\n\n", base_type);
  return -1;
}

static int
init_shared_object_library()
{
  char sym_name[128];

  base_start = NULL;
  base_run = NULL;
  base_shutdown = NULL;

  if (load_shared_object_library() < 0)
    return -1;

  sprintf((char*)&sym_name, "carmen_%s_start%c", base_type, 0x00);
  base_start = (START_FUNC_TYPE*)dlsym(dl_handle, (char*)&sym_name);
  if (base_start == NULL) {
    fprintf(stderr, "Can't find function %s: %s\n", sym_name, dlerror());
    dlclose(dl_handle);
    return -1;
  }

  sprintf((char*)&sym_name, "carmen_%s_run%c", base_type, 0x00);
  base_run = (RUN_FUNC_TYPE*)dlsym(dl_handle, (char*)&sym_name);
  if (base_run == NULL) {
    fprintf(stderr, "Can't find function %s: %s\n", sym_name, dlerror());
    dlclose(dl_handle);
    return -1;
  }

  sprintf((char*)&sym_name, "carmen_%s_shutdown%c", base_type, 0x00);
  base_shutdown = (SHUTDOWN_FUNC_TYPE*)dlsym(dl_handle, (char*)&sym_name);
  if (base_shutdown == NULL) {
    fprintf(stderr, "Can't find function %s: %s\n", sym_name, dlerror());
    dlclose(dl_handle);
    return -1;
  }

  sprintf((char*)&sym_name, "carmen_%s_emergency_crash%c", base_type, 0x00);
  base_emergency_crash = (EMERGENCY_CRASH_FUNC_TYPE*)
    dlsym(dl_handle, (char*)&sym_name);
  if (base_emergency_crash == NULL) {
    fprintf(stderr, "Can't find function %s: %s\n", sym_name, dlerror());
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
	       "Are you sure there's a definition for base_type in "
	       "the parameter server?\n", carmen_param_get_error());

  if (base_type == NULL)
    carmen_die("Error in getting base type from parameter server : "
	       "returned value is NULL\nAre you sure there's a definition "
	       "for base_type in the parameter server?\n");

  return init_shared_object_library();
}

static void
shutdown_base(int signo)
{
    base_shutdown(signo);
    carmen_laser_shutdown(signo);
    carmen_robot_shutdown(signo);

    close_shared_object_library();
    exit(-1);
}

int 
main(int argc __attribute__ ((unused)), char **argv __attribute__ ((unused)))
{
  carmen_initialize_ipc(argv[0]);
  carmen_param_check_version(argv[0]);

  if (read_base_services_parameters(argc, argv) < 0)
    return -1;

  signal(SIGINT, shutdown_base);
  signal(SIGSEGV, base_emergency_crash);

  if (base_start(argc, argv) < 0)
    exit(-1);

  if (carmen_laser_start(argc, argv) < 0) {
    base_shutdown(SIGTERM);
    exit(-1);
  }
  
  if (carmen_robot_start(argc, argv) < 0)
    exit(-1);

  while(1) {
    fprintf(stderr, ".");
    sleep_ipc(0.01);

    base_run();
    carmen_laser_run();
    carmen_robot_run();
  }

  close_shared_object_library();
  return 0;
}


