#include <carmen/carmen.h>

#define     TV     1.2 
#define     RV      0.4

void odometry_handler(carmen_base_odometry_message *odometry)
{
  fprintf(stderr, "x,y,theta = %f, %f, %f\n", odometry->x, odometry->y,
	  odometry->theta);
}

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
    *tv = TV;
    *rv = 0;
    break;
  case 'u':
    *tv = TV;
    *rv = RV;
    break;
  case 'o':
    *tv = TV;
    *rv = -RV;
    break;
  case 'j':
    *tv = 0;
    *rv = RV;
    break;
  case 'l':
    *tv = 0;
    *rv = -RV;
    break;
  case ',':
    *tv = -TV;
    *rv = 0;
    break;
  case 'm':
    *tv = -TV;
    *rv = -RV;
    break;
  case '.':
    *tv = -TV;
    *rv = RV;
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
  double start, now;
  
  /* connect to IPC network */
  carmen_initialize_ipc(argv[0]);
  carmen_param_check_version(argv[0]);
  register_ipc_messages();
  carmen_base_subscribe_odometry_message(NULL, 
					 (carmen_handler_t)odometry_handler, 
					 CARMEN_SUBSCRIBE_LATEST);
  carmen_initialize_keyboard();

#define FORWARDTIME 2.5
#define TURNTIME 3.0
  
  while (0) {
    // march up and down
    fprintf(stderr, "FORWARD\n");
    now = start = carmen_get_time_ms();
    while (now - start < (FORWARDTIME)) {
      send_base_velocity_command(TV,0);
/*        usleep(5000); */
      now = carmen_get_time_ms();
    }

    fprintf(stderr, "BACKWARD\n");
    now = start = carmen_get_time_ms();
    while (now - start < (FORWARDTIME)) {
      send_base_velocity_command(-TV,0);
/*        usleep(5000); */
      now = carmen_get_time_ms();
    }

    fprintf(stderr, "TURN RIGHT\n");
    now = start = carmen_get_time_ms();
    while (now - start < (TURNTIME)) {
      send_base_velocity_command(0,-RV);
/*        usleep(5000); */
      now = carmen_get_time_ms();
    }

    fprintf(stderr, "TURN LEFT\n");
    now = start = carmen_get_time_ms();
    while (now - start < (TURNTIME)) {
      send_base_velocity_command(0,RV);
/*        usleep(5000); */
      now = carmen_get_time_ms();
    }
  }

  while (1) {
    fprintf(stderr, "FORWARD RIGHT\n");
    now = start = carmen_get_time_ms();
    while (now - start < (FORWARDTIME)) {
      send_base_velocity_command(TV,-RV);
      now = carmen_get_time_ms();
    }

    fprintf(stderr, "BACKWARD LEFT\n");
    now = start = carmen_get_time_ms();
    while (now - start < (FORWARDTIME)) {
      send_base_velocity_command(-TV,RV);
      now = carmen_get_time_ms();
    }

    fprintf(stderr, "FORWARD LEFT\n");
    now = start = carmen_get_time_ms();
    while (now - start < (FORWARDTIME)) {
      send_base_velocity_command(TV,RV);
      now = carmen_get_time_ms();
    }

    fprintf(stderr, "BACKWARD RIGHT\n");
    now = start = carmen_get_time_ms();
    while (now - start < (FORWARDTIME)) {
      send_base_velocity_command(-TV,-RV);
      now = carmen_get_time_ms();
    }
  }
}

