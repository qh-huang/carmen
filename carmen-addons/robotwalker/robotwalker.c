#include <carmen/carmen.h>
#include <carmen/serial.h>

#define    METRES_PER_INCH            0.0254 
#define    METRES_PER_WALKER          (METRES_PER_INCH/10.0) 
#define    WHEELBASE                  (13.4 * METRES_PER_INCH)
#define    ROT_VEL_FACT_RAD           (WHEELBASE/METRES_PER_WALKER)
#define    MAX_VELOCITY               (1.0 / METRES_PER_WALKER)

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

//#define    VERBOSE

int fd1;
int fd2;

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

  //time_of_last_command = carmen_get_time_ms();

  left_vel = right_vel = v.tv;
  //assuming v.rv is in radians per second
  left_vel -= 0.5 * v.rv * ROT_VEL_FACT_RAD;
  right_vel += 0.5 * v.rv * ROT_VEL_FACT_RAD;

  left_vel = carmen_clamp(-MAX_VELOCITY, left_vel, MAX_VELOCITY);
  right_vel = carmen_clamp(-MAX_VELOCITY, right_vel, MAX_VELOCITY);

  // make sure fd1 is left wheel motor and fd2 is right wheel motor !!!
  if(icon_command_setdc(fd1, 1, 100.0 * left_vel / MAX_VELOCITY) < 0)
    carmen_die("Error: could not set velocity on motor 1.\n");
  if(icon_command_setdc(fd2, 1, 100.0 * right_vel / MAX_VELOCITY) < 0)
    carmen_die("Error: could not set velocity on motor 2.\n");
}

static void init_motors() {

 if(carmen_serial_connect(&fd1, "/dev/ttyS0") < 0)
    carmen_die("Error: could not open serial port /dev/ttyS0.\n");
  carmen_serial_configure(fd1, 38400, "N");
  fprintf(stderr, "Connected to motor 1.\n");

  if(carmen_serial_connect(&fd2, "/dev/ttyS1") < 0)
    carmen_die("Error: could not open serial port /dev/ttyS1.\n");
  carmen_serial_configure(fd2, 38400, "N");
  fprintf(stderr, "Connected to motor 2.\n");

  signal(SIGINT, shutdown_module);

  if(icon_command_energize(fd1, 1) < 0)
    carmen_die("Error: could not energize motor 1.\n");
  if(icon_command_energize(fd2, 1) < 0)
    carmen_die("Error: could not energize motor 2.\n");
}

static void init_ipc() {

  IPC_RETURN_TYPE err;

  err = IPC_subscribe(CARMEN_ROBOT_VELOCITY_NAME, velocity_handler, NULL);
  carmen_test_ipc_exit(err, "Could not subscribe", CARMEN_ROBOT_VELOCITY_NAME);
  IPC_setMsgQueueLength(CARMEN_ROBOT_VELOCITY_NAME, 1);
}
    
int main(int argc __attribute__ ((unused)), char **argv)
{
  //int speed = 50;  //percent

  carmen_initialize_ipc(argv[0]);
  carmen_param_check_version(argv[0]);

  init_motors();
  init_ipc();

  while(1) {
    sleep_ipc(0.01);

    /*
    if(icon_command_setdc(fd1, 1, speed) < 0)
      carmen_die("Error: could not set velocity on motor 1.\n");
    if(icon_command_setdc(fd2, 1, speed) < 0)
      carmen_die("Error: could not set velocity on motor 2.\n");
    sleep(4);
    icon_command_stop(fd1, 1);
    icon_command_stop(fd2, 1);
    sleep(4);
    */
  }

  return 0;
}
