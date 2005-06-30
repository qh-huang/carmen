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

static void 
usage(char *progname, char *fmt, ...) 
{
  va_list args;

  if (fmt != NULL)
    {
      fprintf(stderr, "\n[31;1m");
      va_start(args, fmt);
      vfprintf(stderr, fmt, args);
      va_end(args);
      fprintf(stderr, "[0m\n\n");
    }
  else
    {
      fprintf(stderr, "\n");
    }

  if (strrchr(progname, '/') != NULL)
    {
      progname = strrchr(progname, '/');
      progname++;
    }

  fprintf(stderr, "Usage: %s [variable_name] [new_value]\n", progname);
  fprintf(stderr, "\nTo get module variables, use\n");
  fprintf(stderr, "       %s [module_name] [all]\n", progname);
  exit(-1);
}

/* handler for C^c */
int 
main(int argc, char** argv)
{
  int param_error;

  char *return_string = NULL;
  int list_length;
  char **variables, **values;
  int index;
  int max_variable_width;
  char buffer[255];

  if (argc < 2 || argc > 3)
    usage(argv[0], NULL);

  carmen_initialize_ipc(argv[0]);

  carmen_param_check_version(argv[0]);

  if (argc == 2) 
    {
      
      param_error = carmen_param_get_string(argv[1], &return_string);
      carmen_param_handle_error(param_error, usage, argv[0]);

      if (return_string) 
	{
	  printf("%s = %s\n", argv[1], return_string);
	  free(return_string);
	  exit(0);
	}
      else
	{
	  printf("Could not retrieve %s from paramServer.\n", argv[1]);
	  exit(0);
	}
    }

  if (argc == 3 && carmen_strcasecmp(argv[2], "all") == 0)
    {
      if (carmen_param_get_all(argv[1], &variables, &values, &list_length) < 0)
	{
	  IPC_perror("Error retrieving all variables of module");
	  exit(-1);
	}
      max_variable_width = 0;
      for (index = 0; index < list_length; index++)
	max_variable_width = carmen_fmax(max_variable_width, strlen(variables[index]));

      if (max_variable_width > 60)
	max_variable_width = 60;

      max_variable_width += 5;
      printf("\nVariable list for module [31;1m%s[0m\n", argv[1]);
      memset(buffer, '-', max_variable_width+15);
      buffer[max_variable_width+15] = '\0';
      printf("%s\n", buffer);
      for (index = 0; index < list_length; index++)
	{
	  printf("%s[%dC%s\n", variables[index],
		 max_variable_width - (int)strlen(variables[index]),
		 values[index]);
	  free(variables[index]);
	  free(values[index]);
	}
      free(variables);
      free(values);
      printf("\n");
    }
  else 
    {
      if (carmen_param_set_variable(argv[1], argv[2], &return_string) < 0) 
	{
	  IPC_perror("Error setting variable");
	  exit(-1);
	}
      
      printf("%s = %s\n", argv[1], return_string);
      free(return_string);
    }

  return 0;
}
