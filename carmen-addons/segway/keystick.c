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

float tv_list[] = {0.2, 0.4, 0.6, 0.8, 1.0};
float rv_list[] = {0.4, 0.6, 0.6, 0.6, 0.6};
int speed = 0;

void carmen_initialize_keyboard(void)
{
  struct termios term_struct;
  int flags;
  tcflag_t oflags;

  flags = fcntl((int)stdin, F_GETFL);           /* initialize asyncronous */
  fcntl((int)stdin, F_SETFL, flags | O_NONBLOCK);    /* keyboard input */
  tcgetattr(0, &term_struct);
  oflags = term_struct.c_oflag;
  cfmakeraw(&term_struct);
  term_struct.c_oflag = oflags;
  term_struct.c_lflag |= ISIG;
  tcsetattr(0, TCSANOW, &term_struct);
}

int carmen_read_char(char *c)
{
  long available;
  int i;
  
  ioctl(0, FIONREAD, &available);
  if(available > 0) {
    for(i = 0; i < available; i++)
      read(0, c, 1);
    return 1;
  }
  else
    return 0;
}

void send_base_velocity_command(double tv, double rv)
{
  IPC_RETURN_TYPE err;
  char *host;
  static carmen_base_velocity_message v;
  static int first = 1;

  if(first) {
    host = carmen_get_tenchar_host_name();
    strcpy(v.host, host);
    first = 0;
  }
  v.tv = tv;
  v.rv = rv;
  v.timestamp = carmen_get_time_ms();
  err = IPC_publishData(CARMEN_BASE_VELOCITY_NAME, &v);
  carmen_test_ipc(err, "Could not publish", CARMEN_BASE_VELOCITY_NAME);  
}

int carmen_keyboard_control(char c, double *tv, double *rv)
{
  int quit = 0;
  if(c >= 'A' && c <= 'Z')
    c += 32;
  switch(c) {
  case 'i':
    *tv = tv_list[speed];
    *rv = 0;
    break;
  case 'u':
    *tv = tv_list[speed];
    *rv = rv_list[speed];
    break;
  case 'o':
    *tv = tv_list[speed];
    *rv = -rv_list[speed];
    break;
  case 'j':
    *tv = 0;
    *rv = rv_list[speed];
    break;
  case 'l':
    *tv = 0;
    *rv = -rv_list[speed];
    break;
  case ',':
    *tv = -tv_list[speed];
    *rv = 0;
    break;
  case 'm':
    *tv = -tv_list[speed];
    *rv = -rv_list[speed];
    break;
  case '.':
    *tv = -tv_list[speed];
    *rv = rv_list[speed];
    break;
  case '1':
    speed = 0;
    fprintf(stderr, "TV = %.1f    RV = %.1f\n", 
	    tv_list[speed], rv_list[speed]);
    break;
  case '2':
    speed = 1;
    fprintf(stderr, "TV = %.1f    RV = %.1f\n", 
	    tv_list[speed], rv_list[speed]);
    break;
  case '3':
    speed = 2;
    fprintf(stderr, "TV = %.1f    RV = %.1f\n", 
	    tv_list[speed], rv_list[speed]);
    break;
  case '4':
    speed = 3;
    fprintf(stderr, "TV = %.1f    RV = %.1f\n", 
	    tv_list[speed], rv_list[speed]);
    break;
  case '5':
    speed = 4;
    fprintf(stderr, "TV = %.1f    RV = %.1f\n", 
	    tv_list[speed], rv_list[speed]);
    break;
  case 'q':
    quit = -1;
    break;
  default:
    *tv = 0;
    *rv = 0;
    break;
  }
  if(quit >= 0) {
    send_base_velocity_command(*tv, *rv);
    return 0;
  }
  return -1;
}

void register_ipc_messages(void)
{
  IPC_RETURN_TYPE err;
  
  /* define messages created by this module */
  err = IPC_defineMsg(CARMEN_BASE_ODOMETRY_NAME, IPC_VARIABLE_LENGTH,
                      CARMEN_BASE_ODOMETRY_FMT);
  carmen_test_ipc_exit(err, "Could not define", CARMEN_BASE_ODOMETRY_NAME);
  
  err = IPC_defineMsg(CARMEN_BASE_VELOCITY_NAME, IPC_VARIABLE_LENGTH,
                      CARMEN_BASE_VELOCITY_FMT);
  carmen_test_ipc_exit(err, "Could not define", CARMEN_BASE_VELOCITY_NAME);
}

int main(int argc __attribute__ ((unused)), char **argv)
{
  char c;
  double tv = 0, rv = 0;
  int err = 0;

  /* connect to IPC network */
  carmen_initialize_ipc(argv[0]);
  carmen_param_check_version(argv[0]);
  register_ipc_messages();
  carmen_initialize_keyboard();
  do {
    if(carmen_read_char(&c)) {
      err = carmen_keyboard_control(c, &tv, &rv);
    }
    else
      sleep_ipc(0.1);
  } while(!err);
  return 0;
}

