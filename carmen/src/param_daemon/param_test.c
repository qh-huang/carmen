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

/* handler for C^c */
void 
shutdown_module(int x)
{
  if(x == SIGINT) {
    close_ipc();
    exit(1);
  }
}

void
handler(char *module, char *variable, char *value)
{
  fprintf(stderr, "%s_%s has changed, is now %s\n", module, variable, value);
}

int 
main(int argc __attribute__ ((unused)), char** argv)
{
  double length;
  char *dev;
  int num_items;

  carmen_param_t param_list[] = {
    {"robot", "length", CARMEN_PARAM_DOUBLE, &length, 1, handler},
    {"scout", "dev", CARMEN_PARAM_STRING, &dev, 1, handler},
  };

  carmen_initialize_ipc(argv[0]); 
  carmen_param_check_version(argv[0]);

  signal(SIGINT, shutdown_module);
  
  num_items = sizeof(param_list)/sizeof(param_list[0]);

  carmen_param_install_params(argc, argv, param_list, num_items);

  carmen_warn("robot_length is %.1f\n", length);
  carmen_warn("scout_dev is %s\n", dev);

  carmen_param_check_unhandled_commandline_args(argc, argv);

  IPC_dispatch();

  return 0;
}
