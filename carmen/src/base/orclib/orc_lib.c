#include <carmen/carmen.h>
#include <carmen/base_low_level.h>
#include <carmen/serial.h>
#include <sys/ioctl.h>
#include <limits.h>

#define ORC_MASTER 0
#define ORC_SLAVE 1

#define ORC_STATUS 0x2A

#define ORC_LEFT_SONAR_PING 4
#define ORC_RIGHT_SONAR_PING 6

#define ORC_LEFT_SONAR_ECHO 49
#define ORC_RIGHT_SONAR_ECHO 51

#define ORC_SONAR_PING 6
#define ORC_SONAR_ECHO 7

#define ORC_LEFT_MOTOR 0
#define ORC_RIGHT_MOTOR 2

#define ORC_LEFT_MOTOR_ACTUAL_PWM 14
#define ORC_RIGHT_MOTOR_ACTUAL_PWM 19

#define ORC_LEFT_PINMODE 4
#define ORC_RIGHT_PINMODE 5

#define ORC_LEFT_MOTOR_QUAD_PORT 16
#define ORC_RIGHT_MOTOR_QUAD_PORT 18

#define ORC_QUAD_PHASE_FAST 14

#define ORC_LEFT_MOTOR_DIR 25
#define ORC_RIGHT_MOTOR_DIR 26
#define ORC_FORWARD 1
#define ORC_BACKWARD 2

#define ORC_MAX_ANGULAR_VEL 8.0 // Radians / seconds

#define ORC_MAX_PWM 255
#define ORC_FF_GAIN ((ORC_MAX_PWM / ORC_MAX_ANGULAR_VEL) * 0.9)
#define ORC_P_GAIN 8
#define ORC_I_GAIN 3
#define ORC_D_GAIN 15

#define ORC_VEL_ACCEL_TEMP 0.9
#define ORC_VEL_DECEL_TEMP 0.4

#define ORC_LEFT_MOTOR_ENCODER 7
#define ORC_RIGHT_MOTOR_ENCODER 10

#define ORC_MASTER_TIME 4
#define ORC_SLAVE_TIME 48

#define ORC_WHEEL_DIAMETER .125
#define ORC_WHEEL_BASE .43
#define ORC_ENCODER_RESOLUTION 500
#define ORC_GEAR_RATIO 65.5

#define ORC_DIGITAL_IN_PULL_UP 1
#define ORC_DIGITAL_IN 6

#define ORC_SERVO_CURRENT 35
#define ORC_SERVO_PWM_STATE 8
#define ORC_SERVO_PIN 5

static double acceleration;
static double deceleration;

static double x, y, theta;
static double displacement, rotation;
static double left_velocity, right_velocity;
static double left_iTerm = 0.0, right_iTerm = 0.0;
static double left_vel_ramp = 0.0, right_vel_ramp = 0.0;
static double left_error_prev = 0.0, right_error_prev = 0.0;
static double left_desired_velocity = 0, right_desired_velocity = 0;
static double left_range, right_range;
static int initialized = 0;
static int sonar_on = 1;

static int left_pwm = 0, right_pwm = 0;
static short last_master_ticks;
static short last_slave_ticks;
static int left_last_tick, right_last_tick;
static double time_since_last_command;
static double last_command_time;

static double servo_state[4];
static double servo_current[2];

static unsigned char bumpers[4];
static int gripper_state = 0;
static int serial_fd = -1;

static void recover_failure(void)
{
  printf("Recovering from Serial Failure\n");
  initialized = 0;
  if (serial_fd >= 0) {
    close(serial_fd);
    serial_fd = -1;
  }
  while (serial_fd < 0) {
    sleep(1);
    carmen_warn("Trying to reconnect to base...\n");
    carmen_base_direct_initialize_robot(NULL,  NULL);
  }
}

unsigned char create_checksum(unsigned char *buffer, int size)
{
  unsigned char checksum = 0;
  int i;

  for (i = 0; i < size; i++)
    checksum = (checksum << 1) + buffer[i] + (checksum & 0x80 ? 1 : 0);

  return checksum;
}

void send_packet(unsigned char *byte, int length,  unsigned char where)
{
  static unsigned char *buffer;
  static unsigned char size;
  static int buffer_size = 0;
  int i;
  int num_written;

  if (buffer_size == 0) {
    buffer = (unsigned char *)calloc(sizeof(unsigned char), length+4);
    carmen_test_alloc(buffer);
    buffer_size = length+4;
  } else if (buffer_size < length+4) {
    buffer = (unsigned char *)realloc
      (buffer, sizeof(unsigned char)*(length+4));
    carmen_test_alloc(buffer);
    buffer_size = length+4;
  }

  size = length+4;

  buffer[0] = 0xED;
  buffer[1] = size;
  buffer[2] = (where << 6) | 0xF;
  for (i = 0; i < length; i++)
    buffer[i+3] = byte[i];
  buffer[length+3] = create_checksum(buffer, length+3);

  num_written = carmen_serial_writen(serial_fd, buffer, length+4);
  if (num_written < 0)
    recover_failure();
}

static void command_velocity(double desired_velocity, double current_velocity,
           int current_pwm, unsigned char WHICH_MOTOR)
{
  double desired_angular_velocity, current_angular_velocity;
  double desired_velocity_ramp = 0;
  double ffTerm = 0.0, pTerm = 0.0, dTerm = 0.0, velError = 0.0;
  double * iTermPtr;
  double * velErrorPrevPtr;
  double * velRampPtr;
  unsigned char command_pwm;
  unsigned char dir;
  unsigned char buffer[4];
  char which;

  if (WHICH_MOTOR == ORC_LEFT_MOTOR) {
    which = 'L';
    iTermPtr = &left_iTerm;
    velRampPtr = &left_vel_ramp;
    velErrorPrevPtr = &left_error_prev;
  }
  else {
    which = 'R';
    iTermPtr = &right_iTerm;
    velRampPtr = &right_vel_ramp;
    velErrorPrevPtr = &right_error_prev;
  }

  if (desired_velocity == 0) {
    *velRampPtr = 0;
  }
  else if (desired_velocity > *velRampPtr) {
    (*velRampPtr) += acceleration * time_since_last_command;
    printf("accel %f %f %f\n", acceleration * time_since_last_command,
	   acceleration, time_since_last_command);
    //    (*velRampPtr) += ORC_VEL_ACCEL_TEMP * time_since_last_command;
    //    printf("accel %f\n", ORC_VEL_ACCEL_TEMP * time_since_last_command);
    if (*velRampPtr > desired_velocity) {
      *velRampPtr = desired_velocity;
    }
  }
  else if (desired_velocity < *velRampPtr) {
    (*velRampPtr) -= deceleration * time_since_last_command;
    printf("decel %f %f %f\n", deceleration * time_since_last_command,
    	   deceleration, time_since_last_command);
    //    (*velRampPtr) -= ORC_VEL_DECEL_TEMP * time_since_last_command;
    //    printf("decel %f\n", ORC_VEL_DECEL_TEMP * time_since_last_command);
    if (*velRampPtr < desired_velocity) {
      *velRampPtr = desired_velocity;
    }
  }

  desired_velocity_ramp = *velRampPtr;

  desired_angular_velocity = desired_velocity_ramp / (ORC_WHEEL_DIAMETER/2.0);
  current_angular_velocity = current_velocity / (ORC_WHEEL_DIAMETER/2.0);


  if (fabs(desired_angular_velocity) > .001) {
      /* Nick, what did you mean to do here?  I don't understand these
         comparisons.  Do you mean < 3 and &&? */
    if (desired_angular_velocity > ORC_MAX_ANGULAR_VEL)
      desired_angular_velocity = ORC_MAX_ANGULAR_VEL;
    if (desired_angular_velocity < -ORC_MAX_ANGULAR_VEL)
      desired_angular_velocity = -ORC_MAX_ANGULAR_VEL;

    velError = (desired_angular_velocity - current_angular_velocity);
    ffTerm = desired_angular_velocity * ORC_FF_GAIN;
    pTerm = velError * ORC_P_GAIN;
    *iTermPtr += velError * ORC_I_GAIN;
    dTerm = (velError - *velErrorPrevPtr) * ORC_D_GAIN;

    current_pwm = (int)(ffTerm + pTerm + *iTermPtr + dTerm);

    *velErrorPrevPtr = velError;

    if (abs(current_pwm) > ORC_MAX_PWM)
      current_pwm = (current_pwm > 0 ? ORC_MAX_PWM : -ORC_MAX_PWM);
    if (WHICH_MOTOR == ORC_LEFT_MOTOR)
      current_pwm = -current_pwm;

    if (current_pwm < 0) {
      command_pwm = -current_pwm;
      dir = ORC_BACKWARD;
    } else {
      command_pwm = current_pwm;
      dir = ORC_FORWARD;
    }
  } else {
    dir = ORC_FORWARD;
    command_pwm = 0;
    *iTermPtr = 0.0;
  }

  buffer[0] = 0x4D;
  buffer[1] = WHICH_MOTOR;
  buffer[2] = dir;
  buffer[3] = command_pwm;
  send_packet(buffer, 4, ORC_SLAVE);
}

static double delta_tick_to_metres(int delta_tick)
{
  double revolutions = (double)delta_tick/
    (double)(ORC_ENCODER_RESOLUTION*ORC_GEAR_RATIO);
  double radians = revolutions*2*M_PI;
  double metres = radians*(ORC_WHEEL_DIAMETER/2.0);

  return metres;
}

static double voltage_to_current(unsigned short voltage)
{
  double current;
  current = voltage*5.0/65536.0;
  // V=IR, R=0.18 ohm
  current = current/0.18;
  return current;
}

static int unpack_short (unsigned char *buffer, int offset)
{
  return ((buffer[offset]&0x00ff)<<8)+(buffer[offset+1]&0x00ff);
}

static void unpack_master_packet(unsigned char *buffer)
{
  int range;
  short time_ticks, delta_master_ticks;
  unsigned char command_buffer[2];
  short bumper_state;
  unsigned short servo_pwm;
  int i;
  static double last_ping_time = 0;
  static int last_ping = ORC_RIGHT_SONAR_PING;

  //carmen_warn("Got master packet\n");
 //printf("unpack_master_packet\n");

  range = unpack_short(buffer, ORC_LEFT_SONAR_ECHO);
  if (range == 0xffff)
    left_range = 0;
  else
    left_range = range/1000000.0*331.46/2.0;

  range = unpack_short(buffer, ORC_RIGHT_SONAR_ECHO);
  if (range == 0xffff)
    right_range = 0;
  else
    right_range = range/1000000.0*331.46/2.0;

  time_ticks = buffer[ORC_MASTER_TIME];
  delta_master_ticks = time_ticks - last_master_ticks;
  if (delta_master_ticks > SHRT_MAX/2)
    delta_master_ticks -= 2*SHRT_MAX;
  if (delta_master_ticks < -SHRT_MAX/2)
    delta_master_ticks += 2*SHRT_MAX;

  last_master_ticks = time_ticks;

  if (sonar_on && carmen_get_time() - last_ping_time > .05) {
    command_buffer[0] = 'R';
    if (last_ping == ORC_LEFT_SONAR_PING)
      command_buffer[1] = ORC_RIGHT_SONAR_PING;
    else
      command_buffer[1] = ORC_LEFT_SONAR_PING;
    send_packet(command_buffer, 2, ORC_MASTER);
    last_ping = command_buffer[1];
    last_ping_time = carmen_get_time();
  }

  bumper_state = unpack_short(buffer, ORC_DIGITAL_IN);

  bumpers[0] = bumper_state >> 8 & 1;
  bumpers[1] = bumper_state >> 9 & 1;
  bumpers[2] = bumper_state >> 10 & 1;
  bumpers[3] = bumper_state >> 11 & 1;
  gripper_state = bumper_state >> 12 & 1;

 
  for (i = 0; i < 4; i++) {
    servo_pwm = unpack_short(buffer, ORC_SERVO_PWM_STATE+i*2);
    servo_state[i] = servo_pwm;
  }

}

static void unpack_slave_packet(unsigned char *buffer)
{
  short left_tick, right_tick, time_ticks, delta_slave_ticks;
  double left_displacement, right_displacement;
  int left_delta_tick, right_delta_tick;
  double delta_slave_time;
  int left_dir, right_dir;
  unsigned char left_pinmode, right_pinmode;
  unsigned short voltage;
  static double start;

  //carmen_warn("Got slave packet\n");
  //printf("unpack_slave_packet\n");

  if (!initialized) {
    start = carmen_get_time();
    initialized = 1;
    x = 0;
    y = 0;
    theta = 0;

    left_last_tick = unpack_short(buffer, ORC_LEFT_MOTOR_ENCODER);
    right_last_tick = ((buffer[ORC_RIGHT_MOTOR_ENCODER]&0x00ff)<<8)+
      (buffer[ORC_RIGHT_MOTOR_ENCODER+1]&0x00ff);

    last_slave_ticks = ((buffer[ORC_SLAVE_TIME]&0x00ff)<<8)+
      (buffer[ORC_SLAVE_TIME+1]&0x00ff);
    left_pwm = buffer[ORC_LEFT_MOTOR_ACTUAL_PWM];
    right_pwm = buffer[ORC_RIGHT_MOTOR_ACTUAL_PWM];

    left_dir = ((buffer[ORC_LEFT_MOTOR_DIR] & 0x0f) >> 2) & 0x03;
    right_dir = ((buffer[ORC_RIGHT_MOTOR_DIR] & 0x0f) >> 2) & 0x03;

    if (left_dir == ORC_FORWARD)
      left_pwm = -left_pwm;
    if (right_dir == ORC_BACKWARD)
      right_pwm = -right_pwm;

    return;
  }

  voltage = unpack_short(buffer, ORC_SERVO_CURRENT);
  servo_current[0] = voltage_to_current(voltage);
  voltage = unpack_short(buffer, ORC_SERVO_CURRENT+2);
  servo_current[1] = voltage_to_current(voltage);

  left_pinmode = buffer[ORC_LEFT_PINMODE];
  right_pinmode = buffer[ORC_RIGHT_PINMODE];

  left_pwm = buffer[ORC_LEFT_MOTOR_ACTUAL_PWM];
  right_pwm = buffer[ORC_RIGHT_MOTOR_ACTUAL_PWM];

  left_dir = ((buffer[ORC_LEFT_MOTOR_DIR] & 0x0f) >> 2) & 0x03;
  right_dir = ((buffer[ORC_RIGHT_MOTOR_DIR] & 0x0f) >> 2) & 0x03;

  if (left_dir == ORC_FORWARD)
    left_pwm = -left_pwm;
  if (right_dir == ORC_BACKWARD)
    right_pwm = -right_pwm;

  left_tick = ((buffer[ORC_LEFT_MOTOR_ENCODER]&0x00ff)<<8)+
    (buffer[ORC_LEFT_MOTOR_ENCODER+1]&0x00ff);

  left_delta_tick = left_tick - left_last_tick;
  if (left_delta_tick > SHRT_MAX/2)
    left_delta_tick = left_delta_tick - 2*SHRT_MAX;
  if (left_delta_tick < -SHRT_MAX/2)
    left_delta_tick = left_delta_tick + 2*SHRT_MAX;

  right_tick = ((buffer[ORC_RIGHT_MOTOR_ENCODER]&0x00ff)<<8)+
    (buffer[ORC_RIGHT_MOTOR_ENCODER+1]&0x00ff);
  right_delta_tick = right_tick - right_last_tick;
  if (right_delta_tick > SHRT_MAX/2)
    right_delta_tick = right_delta_tick - 2*SHRT_MAX;
  if (right_delta_tick < -SHRT_MAX/2)
    right_delta_tick = right_delta_tick + 2*SHRT_MAX;

  left_last_tick = left_tick;
  right_last_tick = right_tick;

  right_delta_tick = -right_delta_tick;

  left_displacement = delta_tick_to_metres(left_delta_tick);
  right_displacement = delta_tick_to_metres(right_delta_tick);

  time_ticks = ((buffer[ORC_SLAVE_TIME]&0x00ff)<<8)+
    (buffer[ORC_SLAVE_TIME+1]&0x00ff);

  delta_slave_ticks = time_ticks - last_slave_ticks;
  last_slave_ticks = time_ticks;

  if (delta_slave_ticks > SHRT_MAX/2)
    delta_slave_ticks -= 2*SHRT_MAX;
  if (delta_slave_ticks < -SHRT_MAX/2)
    delta_slave_ticks += 2*SHRT_MAX;

  delta_slave_time = delta_slave_ticks/4000.0;

  displacement = (left_displacement+right_displacement)/2;
  rotation = atan2(right_displacement-left_displacement, ORC_WHEEL_BASE);

  left_velocity = left_displacement/delta_slave_time;
  right_velocity = right_displacement/delta_slave_time;

  x = x+cos(theta);
  y = y+sin(theta);
  theta = carmen_normalize_theta(theta+rotation);

  time_since_last_command = carmen_get_time() - last_command_time;
  last_command_time = carmen_get_time();
  command_velocity(left_desired_velocity, left_velocity, left_pwm,
       ORC_LEFT_MOTOR);
  command_velocity(right_desired_velocity, right_velocity, right_pwm,
       ORC_RIGHT_MOTOR);
}

static unsigned char *check_packet_length(unsigned char *buffer,
            int *buffer_length,
            int packet_length)
{
  if (*buffer_length >= packet_length)
    return buffer;

  if (buffer_length == 0)
    buffer = (unsigned char *)calloc(sizeof(unsigned char), packet_length);
  else
    buffer = (unsigned char *)realloc
      (buffer, packet_length*sizeof(unsigned char));
  carmen_test_alloc(buffer);
  *buffer_length = packet_length;

  return buffer;
}

static void get_response(void)
{
  static unsigned char byte, routing;
  static unsigned char *buffer = NULL;
  static unsigned char *data_ptr;
  static int buffer_length = 0;
  unsigned char checksum;

  int num_ready_chars;
  int packet_length;
  struct timeval timeout;
  fd_set rfds;
  int num_ready;

  int received_a_packet = 0;

  timeout.tv_sec = 0;
  timeout.tv_usec = 50000;

  while (1) {
    num_ready_chars = carmen_serial_numChars(serial_fd);

    if (num_ready_chars == -1) {
      recover_failure();
      return;
    }
    if (num_ready_chars == 0) {
      if (received_a_packet)
	return;
      FD_ZERO(&rfds);
      FD_SET(0, &rfds);
      num_ready = select(1, &rfds, NULL, NULL, &timeout);
      if (num_ready == 1)
	continue;
      else {
	return;
      }
    }

    if (carmen_serial_readn(serial_fd, &byte, 1) < 0) {
      recover_failure();
      return;
    }
    if (byte == 0xED) {
      if (carmen_serial_readn(serial_fd, &byte, 1) < 0) {
	recover_failure();
	return;
      }
      packet_length = byte;

      buffer = check_packet_length(buffer, &buffer_length, packet_length);
      buffer[0] = 0xED;
      buffer[1] = byte;
      if (carmen_serial_readn(serial_fd, &byte, 1) < 0) {
	recover_failure();
	return;
      }
      buffer[2] = byte;
      data_ptr = buffer+3;
      routing = byte >> 6;

      if (carmen_serial_readn(serial_fd, data_ptr, packet_length-3) < 0) {
	recover_failure();
	return;
      }

      checksum = create_checksum(buffer, packet_length-1);

      if (checksum != buffer[packet_length-1]) {
        carmen_warn("Corrupted data from serial line. "
                    "Dropping status packet.\n");
        return;
      }

      if (data_ptr[0] == ORC_STATUS && routing == ORC_MASTER) {
	unpack_master_packet(buffer);
	received_a_packet = 1;
      }
      else if (data_ptr[0] == ORC_STATUS && routing == ORC_SLAVE) {
	unpack_slave_packet(buffer);
	received_a_packet = 1;
      }
    }
  }
}

int carmen_base_direct_sonar_on(void)
{
  printf("carmen_base_direct_sonar_on\n");
  sonar_on = 1;

  return 2;
}

int
carmen_base_direct_sonar_off(void)
{
  sonar_on = 0;
  return 2;
}


int carmen_base_direct_reset(void)
{
  printf("carmen_base_direct_reset\n");
  initialized = 0;
  return 0;
}


int carmen_base_direct_initialize_robot(char *model __attribute__ ((unused)),
          char *dev __attribute__ ((unused)))
{
  int result;
  unsigned char buffer[5];

  result = carmen_serial_connect(&serial_fd, dev);
  if(result == -1) {
    serial_fd = -1;
    return -1;
  }

  carmen_serial_configure(serial_fd, 115200, "8N1");

  carmen_serial_ClearInputBuffer(serial_fd);

  buffer[0] = 'C';
  buffer[1] = 0; // Motor Encoder 0
  buffer[2] = 14; // Quad Phase Fast
  send_packet(buffer, 3, ORC_SLAVE);

  buffer[1] = 1; // Motor Encoder 0
  send_packet(buffer, 3, ORC_SLAVE);

  buffer[1] = 2; // Motor Encoder 1
  send_packet(buffer, 3, ORC_SLAVE);
  buffer[1] = 3; // Motor Encoder 1
  send_packet(buffer, 3, ORC_SLAVE);

  //Edsinger: added servo 
  buffer[1] = 0; // Servo0
  buffer[2] = ORC_SERVO_PIN; //Servo
  send_packet(buffer, 3, ORC_MASTER);

  buffer[1] = 1; // Servo1
  buffer[2] = ORC_SERVO_PIN; //Servo
  send_packet(buffer, 3, ORC_MASTER);

  buffer[1] = 2; // Servo2
  buffer[2] = ORC_SERVO_PIN; //Servo
  send_packet(buffer, 3, ORC_MASTER);

  buffer[1] = 3; // Servo3
  buffer[2] = ORC_SERVO_PIN; //Servo
  send_packet(buffer, 3, ORC_MASTER);


  buffer[1] = 4; // Sonar 0
  buffer[2] = ORC_SONAR_PING;
  send_packet(buffer, 3, ORC_MASTER);

  buffer[1] = 6; // Sonar 1
  buffer[2] = ORC_SONAR_PING;
  send_packet(buffer, 3, ORC_MASTER);

  buffer[1] = 5; // Sonar 0
  buffer[2] = ORC_SONAR_ECHO;
  send_packet(buffer, 3, ORC_MASTER);

  buffer[1] = 7; // Sonar 1
  buffer[2] = ORC_SONAR_ECHO;
  send_packet(buffer, 3, ORC_MASTER);

  buffer[1] = 8; // Bumper
  buffer[2] = ORC_DIGITAL_IN_PULL_UP;
  send_packet(buffer, 3, ORC_MASTER);

  buffer[1] = 9; // Bumper
  buffer[2] = ORC_DIGITAL_IN_PULL_UP;
  send_packet(buffer, 3, ORC_MASTER);

  buffer[1] = 10; // Bumper
  buffer[2] = ORC_DIGITAL_IN_PULL_UP;
  send_packet(buffer, 3, ORC_MASTER);

  buffer[1] = 11; // Bumper
  buffer[2] = ORC_DIGITAL_IN_PULL_UP;
  send_packet(buffer, 3, ORC_MASTER);

  buffer[1] = 12; // Gripper
  buffer[2] = ORC_DIGITAL_IN_PULL_UP;
  send_packet(buffer, 3, ORC_MASTER);

  buffer[0] = 0x4D;
  buffer[1] = ORC_LEFT_MOTOR;
  buffer[2] = 0;
  buffer[3] = 0;
  send_packet(buffer, 4, ORC_SLAVE);

  buffer[1] = ORC_RIGHT_MOTOR;
  send_packet(buffer, 4, ORC_SLAVE);
  printf("carmen_base_direct_initialize_robot\n");

  return 0;
}

int carmen_base_direct_shutdown_robot(void)
{
  close(serial_fd);

  return 0;
}

int carmen_base_direct_set_acceleration(double new_acceleration)
{
  acceleration = new_acceleration;
  printf("carmen_base_direct_set_acceleration: accel=%f\n", new_acceleration);

  return 0;
}

int carmen_base_direct_set_deceleration(double new_deceleration)
{
  deceleration = new_deceleration;
  printf("carmen_base_direct_set_deceleration: decel=%f\n", new_deceleration);

  return 0;
}

int carmen_base_direct_set_velocity(double new_tv, double new_rv)
{
  left_desired_velocity = new_tv;
  right_desired_velocity = new_tv;

  right_desired_velocity += new_rv/2;
  left_desired_velocity -= new_rv/2;

  command_velocity(left_desired_velocity, left_velocity, left_pwm,
       ORC_LEFT_MOTOR);
  command_velocity(right_desired_velocity, right_velocity, right_pwm,
       ORC_RIGHT_MOTOR);
  printf("carmen_base_direct_set_velocity: tv=%f, rv=%f\n", new_tv, new_rv);

  return 0;
}

int carmen_base_direct_update_status(void)
{
  unsigned char byte;

  byte = ORC_STATUS;

  send_packet(&byte, 1, ORC_MASTER);
  send_packet(&byte, 1, ORC_SLAVE);

  get_response();

  return 0;
}

int carmen_base_direct_get_state(double *disp_p, double *rot_p,
         double *tv_p, double *rv_p)
{

  *disp_p = displacement;
  *rot_p = rotation;
  *tv_p = (left_velocity+right_velocity)/2;
  *rv_p = atan2(right_velocity-left_velocity, ORC_WHEEL_BASE/2);

    //carmen_warn("%f %f %f %f\n", *disp_p, *rot_p, *tv_p, *rv_p);

  return 0;
}


int carmen_base_direct_get_integrated_state(double *x_p, double *y_p,
              double *theta_p, double *tv_p,
              double *rv_p)
{
  *x_p = x;
  *y_p = y;
  *theta_p = theta;
  *tv_p = (left_velocity+right_velocity)/2;
  *rv_p = atan2(right_velocity-left_velocity, ORC_WHEEL_BASE/2);

  return 0;
}

int carmen_base_direct_get_sonars(double *ranges, carmen_point_t *positions,
          int num_sonars)
{
  if (num_sonars >= 1) {
    ranges[0] = left_range;
    positions[0].x = .05;
    positions[0].y = -.10;
    positions[0].theta = -M_PI/2;
  }
  if (num_sonars >= 2) {
    ranges[1] = right_range;
    positions[1].x = .05;
    positions[1].y = .10;
    positions[1].theta = M_PI/2;
  }

  return 0;
}

int carmen_base_direct_get_bumpers(unsigned char *bumpers_p, int num_bumpers)
{
  if (num_bumpers >= 1)
    bumpers_p[0] = bumpers[0];
  if (num_bumpers >= 2)
    bumpers_p[1] = bumpers[1];
  if (num_bumpers >= 3)
    bumpers_p[2] = bumpers[2];
  if (num_bumpers >= 4)
    bumpers_p[3] = bumpers[3];

  /*
  printf("carmen_base_direct_get_bumpers");
  if (bumpers[0])
    printf(" 0 ");
  else
    printf(" 0*");

  if (bumpers[1])
    printf(" 1 ");
  else
    printf(" 1*");

  if (bumpers[2])
    printf(" 2 ");
  else
    printf(" 2*");

  if (bumpers[3])
    printf(" 3 ");
  else
    printf(" 3*");

  printf("\n");
  */

  return 4;
}

int carmen_base_direct_send_binary_data(unsigned char *data, int size)
{
  return carmen_serial_writen(serial_fd, data, size);
}


void carmen_base_direct_arm_set(double servos[], int num_servos)
{
  unsigned char buffer[4];
  unsigned short pwm;
  int i;

  
if (num_servos > 4) {
    carmen_warn("orc_lib.c only supports 4 servos (%d sent)\n"
                "Returning only 4\n", num_servos);
    num_servos = 4;
  }

 for (i=0;i<num_servos;i++)
   { 
     int nservo=(int)servos[i];
    pwm = nservo;
    buffer[0] = 0x53;
    buffer[1] = i;
    buffer[2] = pwm >> 8;
    buffer[3] = pwm & 0x00ff;
    send_packet(buffer, 4, ORC_MASTER);
  }
}


void carmen_base_direct_arm_get(double servos[], int num_servos,
                                double *currents, int *gripper)
{
  int i;

  
  if (num_servos > 4) {
    carmen_warn("orc_lib.c only supports 4 servos (%d requested)\n"
                "Returning only 4\n", num_servos);
    num_servos = 4;
  }
 
  for (i = 0; i < num_servos; i++) {
    servos[i] = servo_state[i];
  }
  if (currents && num_servos > 0) {
    currents[0] = servo_current[0];
    if (num_servos > 1)
      currents[1] = servo_current[1];
  }
  if (gripper)
    *gripper = gripper_state;
}
