#include <carmen/carmen.h>
#include <carmen/drive_low_level.h>
#include "orclib_v5/orc.h"
#include <sys/ioctl.h>
#include <limits.h>

#define ORC_MASTER 0
#define ORC_SLAVE 1
#define ORC_PAD 2

#define ORC_STATUS 0x2A

#define ORC_LEFT_SONAR_PING 4
#define ORC_RIGHT_SONAR_PING 6

#define ORC_LEFT_SONAR_ECHO 49
#define ORC_RIGHT_SONAR_ECHO 51

#define ORC_SONAR_PING 6
#define ORC_SONAR_ECHO 7

// Configuring IR sensor to use Sonar ports
#define ORC_LEFT_IR_PING 5
#define ORC_RIGHT_IR_PING 7

// Configure pins to digital
#define ORC_LEFT_IR_ECHO 4
#define ORC_RIGHT_IR_ECHO 6

// Sets modes for pins  
#define ORC_IR_PING 3 // Digital Out; 
#define ORC_IR_ECHO 1 // Digital In (Pull-Up)

#define ORC_LEFT_MOTOR 0
#define ORC_RIGHT_MOTOR 2

#define ORC_LEFT_MOTOR_ACTUAL_PWM 14
#define ORC_RIGHT_MOTOR_ACTUAL_PWM 19

#define ORC_LEFT_MOTOR_SLEW 15
#define ORC_RIGHT_MOTOR_SLEW 21

#define ORC_LEFT_PINMODE 4
#define ORC_RIGHT_PINMODE 5

#define ORC_LEFT_MOTOR_QUAD_PORT 16
#define ORC_RIGHT_MOTOR_QUAD_PORT 18

#define ORC_LEFT_ENCODER_STATE 4
#define ORC_RIGHT_ENCODER_STATE 5

#define ORC_QUAD_PHASE_FAST 14

#define ORC_LEFT_MOTOR_DIR 25
#define ORC_RIGHT_MOTOR_DIR 26
#define ORC_FORWARD 1
#define ORC_BACKWARD 2

#define ORC_BUMPER_PORT0 10
#define ORC_BUMPER_PORT1 11
#define ORC_BUMPER_PORT2 12
#define ORC_BUMPER_PORT3 13

#define ORC_MAX_ANGULAR_VEL 8.0 // Radians / seconds

#define ORC_MAX_PWM 250
#define ORC_FF_GAIN ((ORC_MAX_PWM / ORC_MAX_ANGULAR_VEL) * 0.9)
#define ORC_P_GAIN 20
#define ORC_D_GAIN 5

#define ORC_VEL_ACCEL_TEMP 0.9
#define ORC_VEL_DECEL_TEMP 0.4

#define ORC_LEFT_MOTOR_ENCODER 7
#define ORC_RIGHT_MOTOR_ENCODER 10

#define ORC_MASTER_TIME 4
#define ORC_SLAVE_TIME 48

#define ORC_WHEEL_DIAMETER .2525
#define ORC_WHEEL_BASE .37
#define ORC_ENCODER_RESOLUTION 4550
#define ORC_GEAR_RATIO 1 // 12.5 ??? 

#define ORC_DIGITAL_IN_PULL_UP 1
#define ORC_DIGITAL_IN 6

#define ORC_SERVO_CURRENT 35
#define ORC_SERVO_PWM_STATE 8
#define ORC_SERVO_PIN 5

static double acceleration;
static double deceleration;

static double x, y, theta;
static double left_velocity, right_velocity;
static double left_error_prev = 0.0, right_error_prev = 0.0;
static double left_desired_velocity = 0, right_desired_velocity = 0;
static double left_range, right_range;
static int initialized = 0;
static int sonar_on = 1;


// We ignore the d-term the first time we enter the control loop
// after receiving a new velocity command. This is because the
// d-term captures the derivative of the error, which is not 
// meaningful if the error is caused by the command that moved
// the desired velocity set point.
static int ignore_left_d_term = 1;
static int ignore_right_d_term = 1;

static int left_pwm = 0, right_pwm = 0;
static double time_since_last_command;
static double last_command_time;

static int bumpers[4];

static double left_displacement, right_displacement;
static double delta_slave_time;

static orc_t *s_orc;

int carmen_base_direct_initialize_robot(char *model, char *dev){

  // used only to make the code compile!!!
  model = model;

  // create a new orc object
  orc_comms_impl_t *impl = orc_rawprovider_create( dev );
  s_orc = orc_create( impl );
  return 0;

}

int carmen_base_direct_shutdown_robot(void){

  // destroy the orc
  orc_destroy( s_orc );
  return 0;
}

void carmen_base_command_velocity(double desired_velocity, 
				  double current_velocity,
				  unsigned char WHICH_MOTOR)
{
  double desired_angular_velocity, current_angular_velocity;
  double pTerm = 0.0, dTerm = 0.0, velError = 0.0;
  double *velErrorPrevPtr;
  double pGain = ORC_P_GAIN;

  int current_pwm;
  int new_pwm;

  unsigned char command_pwm;
  unsigned char dir;
  int *ignore_d_term;

  if (WHICH_MOTOR == ORC_LEFT_MOTOR) {
    velErrorPrevPtr = &left_error_prev;
    current_pwm = left_pwm;
    ignore_d_term = &ignore_left_d_term;
    pGain = pGain * 1.2;
  } else {
    velErrorPrevPtr = &right_error_prev;
    current_pwm = right_pwm;
    ignore_d_term = &ignore_right_d_term;
  }

  desired_angular_velocity = desired_velocity / (ORC_WHEEL_DIAMETER/2.0);
  current_angular_velocity = current_velocity / (ORC_WHEEL_DIAMETER/2.0);

  if (fabs(desired_angular_velocity) > .001) {
    if (desired_angular_velocity > ORC_MAX_ANGULAR_VEL)
      desired_angular_velocity = ORC_MAX_ANGULAR_VEL;
    if (desired_angular_velocity < -ORC_MAX_ANGULAR_VEL)
      desired_angular_velocity = -ORC_MAX_ANGULAR_VEL;

    velError = (desired_angular_velocity - current_angular_velocity);
    pTerm = velError * pGain;

    if (!*ignore_d_term)
      dTerm = (velError - *velErrorPrevPtr) * ORC_D_GAIN;
    else {
      dTerm = 0;
      *ignore_d_term = 0;
    }

    new_pwm = current_pwm + carmen_round(pTerm + dTerm);

    if(0)
    carmen_warn("%s %f %f : %f %f : %d %d %d\n", 
		(WHICH_MOTOR == ORC_LEFT_MOTOR ? "left" : "right"),
		desired_angular_velocity,
		current_angular_velocity, pTerm, dTerm,
		current_pwm, carmen_round(pTerm+dTerm), new_pwm);

    *velErrorPrevPtr = velError;

    if (abs(new_pwm) > ORC_MAX_PWM)
      new_pwm = (new_pwm > 0 ? ORC_MAX_PWM : -ORC_MAX_PWM);

    if (WHICH_MOTOR == ORC_RIGHT_MOTOR) {
      if (new_pwm < 0) 
	dir = ORC_FORWARD;
      else 
	dir = ORC_BACKWARD;
      //      carmen_warn("left %s\n", (dir == 1) ? "forward" : "backward");
    } else {
      if (new_pwm < 0) 
	dir = ORC_BACKWARD;
      else 
	dir = ORC_FORWARD;
      //      carmen_warn("right %s\n", (dir == 1) ? "forward" : "backward");
    }
    command_pwm = abs(new_pwm);
  } else {
    dir = ORC_FORWARD;
    command_pwm = 0;
  }

  if (0)
  carmen_warn("Tried to send %d %d %f %f %d\n", command_pwm, new_pwm,
	      desired_velocity, current_velocity, WHICH_MOTOR);

  orc_motor_set( s_orc, WHICH_MOTOR, command_pwm );
}

/*
static double delta_tick_to_metres(int delta_tick)
{
  double revolutions = (double)delta_tick/
    (double)(ORC_ENCODER_RESOLUTION*ORC_GEAR_RATIO);
  double radians = revolutions*2*M_PI;
  double metres = radians*(ORC_WHEEL_DIAMETER/2.0);

  return metres;
}
*/

int carmen_base_direct_sonar_on(void)
{
  printf("carmen_base_direct_sonar_on\n");
  sonar_on = 1;

  return 2;
}

int carmen_base_direct_sonar_off(void)
{
  sonar_on = 0;
  return 2;
}


int carmen_base_direct_reset(void)
{
  printf("carmen_base_direct_reset\n");
  x = 0;
  y = 0;
  theta = 0;

  initialized = 0;
  return 0;
}
int carmen_base_direct_set_acceleration(double new_acceleration)
{
  acceleration = new_acceleration;
  //  printf("carmen_base_direct_set_acceleration: accel=%f\n", new_acceleration);

  return 0;
}

int carmen_base_direct_set_deceleration(double new_deceleration)
{
  deceleration = new_deceleration;
  //  printf("carmen_base_direct_set_deceleration: decel=%f\n", new_deceleration);

  return 0;
}

int carmen_base_direct_set_velocity(double new_tv, double new_rv)
{
  left_desired_velocity = new_tv;
  right_desired_velocity = new_tv;

  right_desired_velocity += new_rv/2;
  left_desired_velocity -= new_rv/2;

  carmen_base_command_velocity(left_desired_velocity, left_velocity,
       ORC_LEFT_MOTOR);
  carmen_base_command_velocity(right_desired_velocity, right_velocity,
       ORC_RIGHT_MOTOR);

  time_since_last_command = carmen_get_time() - last_command_time;
  last_command_time = carmen_get_time();

  printf("carmen_base_direct_set_velocity: tv=%.2f, rv=%.2f\n", 
	 new_tv, new_rv);
  double last_update_time;
  carmen_base_direct_update_status( &last_update_time );

  return 0;
}

int carmen_base_direct_update_status(double* packet_timestamp)
{
  // fill in the time stamp
  *packet_timestamp = carmen_get_time();
  
  // sonar is not implemented in orc 5 so we don't deal with this
  left_range = 0.0;
  right_range = 0.0;

  // ir deleted since not being used

  // gripper moved to the arm section

  // bumpers
  bumpers[0] = orc_digital_read( s_orc, ORC_BUMPER_PORT0 );
  bumpers[1] = orc_digital_read( s_orc, ORC_BUMPER_PORT1 );
  bumpers[2] = orc_digital_read( s_orc, ORC_BUMPER_PORT2 );
  bumpers[3] = orc_digital_read( s_orc, ORC_BUMPER_PORT3 );

  // motor
  

  return 0;
}

int carmen_base_query_low_level(double *left_disp, double *right_disp,
				double *delta_time)
{
  *left_disp = left_displacement;
  *right_disp = right_displacement;
  *delta_time = delta_slave_time;
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
  *rv_p = atan2(right_velocity-left_velocity, ORC_WHEEL_BASE);  
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

  return 4;
}

