#include <carmen/carmen.h>
#include <carmen/serial.h>


#define    MOTOR_WHEEL_RADIUS         0.035
#define    WHEELBASE                  0.555
#define    MOTOR_WHEEL_CIRC           (2.0 * M_PI * MOTOR_WHEEL_RADIUS)
#define    VEL_PER_RPM                (MOTOR_WHEEL_CIRC / 60.0)
#define    MAX_RPM                    120.0
#define    MAX_VELOCITY               (MAX_RPM * VEL_PER_RPM)
#define    MAX_SPEED_PERC             75

#define    DEFAULT_TIMEOUT            30000

#define    COMMAND_ENERGIZE           0xc5
#define    COMMAND_SETDC              0xd0
#define    COMMAND_READ               0xd1
#define    COMMAND_WRITE              0xd2
#define    COMMAND_STORE              0xd3
#define    COMMAND_RESTORE            0xd4
#define    COMMAND_RESET              0xd5
#define    COMMAND_LOOKUP_WRITE       0xd6
#define    COMMAND_LOOKUP_READ        0xd7
#define    COMMAND_LOOKUP_RESTORE     0xd8

#define    VERBOSE


int fd1;
int fd2;
int test_speed = 50;

static carmen_base_odometry_message odometry;


int icon_wait_for_response(int fd, int timeout)
{
  unsigned char response[5];

  usleep(timeout);
  if(carmen_serial_numChars(fd) > 0) {
    carmen_serial_readn(fd, response, 1);
    if(response[0] == 0x06) {                      /* ACK */
#ifdef VERBOSE
      fprintf(stderr, "Return value: ACK\n");
#endif
      return 0;
    }
    else {                                         /* unknown response */
#ifdef VERBOSE
      fprintf(stderr, "Return value: 0x%02x (unknown)\n", response[0]);
#endif
      return -1;        
    }
  }
  else {                                           /* timeout */
#ifdef VERBOSE
    fprintf(stderr, "Command timed out.\n");
#endif
    return -1;
  }
}

int icon_command(int fd, int command, int device, unsigned char *payload,
		 int payload_length, int timeout)
{
  unsigned char *buffer, sum = 0;
  int i;

  buffer = (unsigned char *)calloc(4 + payload_length, 1);
  carmen_test_alloc(buffer);
  buffer[0] = (unsigned char)command;
  buffer[1] = (unsigned char)device;
  buffer[2] = (unsigned char)payload_length;
  memcpy(buffer + 3, payload, payload_length);
  for(i = 0; i < 3 + payload_length; i++)
    sum += buffer[i];
  buffer[3 + payload_length] = sum;

#ifdef VERBOSE
  fprintf(stderr, "Sending command: ");
  for(i = 0; i <= 3 + payload_length; i++)
    fprintf(stderr, "0x%02x ", buffer[i]);
  fprintf(stderr, "\n");
#endif

  carmen_serial_writen(fd, buffer, 4 + payload_length);
  free(buffer);

  return icon_wait_for_response(fd, timeout);
}

int icon_command_energize(int fd, int device)
{
  return icon_command(fd, COMMAND_ENERGIZE, device, NULL, 0, DEFAULT_TIMEOUT);
}

int icon_command_setdc(int fd, int device, int duty_cycle)
{
  unsigned char payload[2];
  int code;

  if(duty_cycle > 100)
    duty_cycle = 100;
  if(duty_cycle < -100)
    duty_cycle = -100;
  code = (int)(duty_cycle / 100.0 * 1023);

  payload[0] = (unsigned char)((code & 0xff00) >> 8);
  payload[1] = (unsigned char)(code & 0x00ff);

  return icon_command(fd, COMMAND_SETDC, device, payload, 2, DEFAULT_TIMEOUT);
}

int icon_command_stop(int fd, int device)
{
  return icon_command_setdc(fd, device, 0);
}

void shutdown_module(int x)
{
  if(x == SIGINT) {
    icon_command_stop(fd1, 1);
    icon_command_stop(fd2, 1);
    fprintf(stderr, "Disconnecting.\n");
    exit(0);
  }
}

static inline void rotate(double *x, double *y, double theta) {

  *x = (*x) * cos(theta) - (*y) * sin(theta);
  *y = (*x) * sin(theta) + (*y) * cos(theta);
}

static void update_odom() {

  double dt, dx, dy, dtheta;

  dt = carmen_get_time_ms() - odometry.timestamp;

  dtheta = odometry.rv * dt;
  dx = cos(odometry.theta) * odometry.tv * dt;
  dy = sin(odometry.theta) * odometry.tv * dt;
  rotate(&dx, &dy, dtheta);

  odometry.x += dx;
  odometry.y += dy;
  odometry.theta += dtheta;
  odometry.timestamp = carmen_get_time_ms();
}

static void velocity_handler(MSG_INSTANCE msgRef, BYTE_ARRAY callData,
			     void *clientData __attribute__ ((unused))) {

  carmen_robot_velocity_message v;
  FORMATTER_PTR formatter;
  IPC_RETURN_TYPE err;
  double left_vel, right_vel;

  formatter = IPC_msgInstanceFormatter(msgRef);
  err = IPC_unmarshallData(formatter, callData, &v,
                           sizeof(carmen_robot_velocity_message));  
  IPC_freeByteArray(callData);
  carmen_test_ipc_return(err, "Could not unmarshall", 
                         IPC_msgInstanceName(msgRef));

  left_vel = right_vel = v.tv;
  //assuming v.rv is in radians per second
  left_vel -= 0.5 * v.rv * WHEELBASE;
  right_vel += 0.5 * v.rv * WHEELBASE;

  left_vel = carmen_clamp(-MAX_VELOCITY, left_vel, MAX_VELOCITY);
  right_vel = carmen_clamp(-MAX_VELOCITY, right_vel, MAX_VELOCITY);

  // make sure fd1 is left wheel motor and fd2 is right wheel motor !!!
  if(icon_command_setdc(fd1, 1, -75.0 * left_vel / MAX_VELOCITY) < 0)
    fprintf(stderr, "Error: could not set velocity on motor 1.\n");
  if(icon_command_setdc(fd2, 1, -75.0 * right_vel / MAX_VELOCITY) < 0)
    fprintf(stderr, "Error: could not set velocity on motor 2.\n");

  update_odom();
  odometry.tv = v.tv;
  odometry.rv = v.rv;
}

static void init_odom() {

  odometry.x = 0.0;
  odometry.y = 0.0;
  odometry.theta = 0.0;
  odometry.tv = 0.0;
  odometry.rv = 0.0;
  odometry.timestamp = 0.0;
  strcpy(odometry.host, carmen_get_tenchar_host_name());
}

static void init_motors(char **argv) {

  while(carmen_serial_connect(&fd1, argv[1]) < 0)
    fprintf(stderr, "Error: could not open serial port %s.\n", argv[1]);
  carmen_serial_configure(fd1, 38400, "N");
  fprintf(stderr, "Connected to motor 1.\n");

  while(carmen_serial_connect(&fd2, argv[2]) < 0)
    fprintf(stderr, "Error: could not open serial port %s.\n", argv[2]);
  carmen_serial_configure(fd2, 38400, "N");
  fprintf(stderr, "Connected to motor 2.\n");

  signal(SIGINT, shutdown_module);

  while(icon_command_energize(fd1, 1) < 0)
    fprintf(stderr, "Error: could not energize motor 1.\n");
  while(icon_command_energize(fd2, 1) < 0)
    fprintf(stderr, "Error: could not energize motor 2.\n");
}

static void init_ipc() {

  IPC_RETURN_TYPE err;

  err = IPC_defineMsg(CARMEN_BASE_ODOMETRY_NAME, IPC_VARIABLE_LENGTH,
                      CARMEN_BASE_ODOMETRY_FMT);
  carmen_test_ipc_exit(err, "Could not define", CARMEN_BASE_ODOMETRY_NAME);

  err = IPC_defineMsg(CARMEN_BASE_VELOCITY_NAME, IPC_VARIABLE_LENGTH,
                      CARMEN_BASE_VELOCITY_FMT);
  carmen_test_ipc_exit(err, "Could not define", CARMEN_BASE_VELOCITY_NAME);

  err = IPC_subscribe(CARMEN_ROBOT_VELOCITY_NAME, velocity_handler, NULL);
  carmen_test_ipc_exit(err, "Could not subscribe", CARMEN_ROBOT_VELOCITY_NAME);
  IPC_setMsgQueueLength(CARMEN_ROBOT_VELOCITY_NAME, 1);
}

static void test_motors() {

  if(icon_command_setdc(fd1, 1, test_speed) < 0)
    fprintf(stderr, "Error: could not set velocity on motor 1.\n");
  sleep(2);
  icon_command_stop(fd1, 1);
  if(icon_command_setdc(fd2, 1, test_speed) < 0)
    fprintf(stderr, "Error: could not set velocity on motor 2.\n");
  sleep(2);
  icon_command_stop(fd2, 1);
  sleep(2);
  if(icon_command_setdc(fd1, 1, test_speed) < 0)
    fprintf(stderr, "Error: could not set velocity on motor 1.\n");
  if(icon_command_setdc(fd2, 1, test_speed) < 0)
    fprintf(stderr, "Error: could not set velocity on motor 2.\n");
  sleep(4);
  icon_command_stop(fd1, 1);
  icon_command_stop(fd2, 1);
  sleep(4);  
}
    
int main(int argc, char **argv)
{
  IPC_RETURN_TYPE err;
  double last_published_update = 0.0;
  double update_delay = .03;

  if (argc < 3) {
    fprintf(stderr, "usage: robotwalker <left motor device> <right motor device> [test_speed]\n");
    exit(1);
  }
  
  if (argc >= 4)
    test_speed = atoi(argv[3]);
  
  carmen_initialize_ipc(argv[0]);
  carmen_param_check_version(argv[0]);

  init_motors(argv);
  init_ipc();
  init_odom();

  while(1) {

    sleep_ipc(0.01);

    if (carmen_get_time_ms() - last_published_update > update_delay) {
      update_odom();
      err = IPC_publishData(CARMEN_BASE_ODOMETRY_NAME, &odometry);
      carmen_test_ipc_exit(err, "Could not publish", CARMEN_BASE_ODOMETRY_NAME);
      last_published_update = odometry.timestamp;
    }

    test_motors();
  }

  return 0;
}
