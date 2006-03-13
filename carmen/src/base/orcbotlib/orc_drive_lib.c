#include <carmen/carmen.h>
#include <carmen/drive_low_level.h>
#include "orclib_v5/orc.h"
#include "orc_drive_constants.h"
#include <sys/ioctl.h>
#include <limits.h>

// Note: the following drive_low_level functions are not implemented 
//    
//





static double left_error_prev = 0.0, right_error_prev = 0.0;
static double left_desired_velocity = 0, right_desired_velocity = 0;





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

static int s_bumpers[4];



// basic
static orc_t *s_orc;
static int s_initialized = 0;

// sensor control -- none of these are currently used
// sonars were used in RSS code but acceleration never was
static int s_sonar_on = 1;   
static double s_acceleration;
static double s_deceleration;

// comm control -- also not used
static int s_serial_fd = -1;

// state data
static double s_x, s_y, s_theta;
static double s_left_range, s_right_range;
static double s_left_displacement, s_right_displacement;
static double s_time, s_delta_time;
static int s_left_tick, s_right_tick;
static double s_left_velocity, s_right_velocity;

// ----- HELPER FUNCTIONS ----- //
static double delta_tick_to_metres(int delta_tick);
static int compute_delta_ticks_abs( int curr, int prev );
static int min( int a, int b );

// ---- START/END/RESET ---- //
int carmen_base_direct_initialize_robot(char *model, char *dev){

  // used only to make the code compile!!!
  model = model;

  // create a new orc object
  orc_comms_impl_t *impl = orc_rawprovider_create( dev );
  s_orc = orc_create( impl );
  s_initialized = 1;

  // reset all variables to defaults
  carmen_base_direct_reset();
  return 0;

}

int carmen_base_direct_shutdown_robot(void){

  // destroy the orc
  orc_destroy( s_orc );
  s_initialized = 1;
  return 0;
}

int carmen_base_direct_reset(void)
{
  // zero position state
  printf("carmen_base_direct_reset\n");
  s_x = 0;
  s_y = 0;
  s_theta = 0;
  s_left_tick = orc_quadphase_read( s_orc, ORC_LEFT_ENCODER_PORT );
  s_right_tick = orc_quadphase_read( s_orc, ORC_RIGHT_ENCODER_PORT );

  // stop moving
  orc_motor_set_signed( s_orc, ORC_LEFT_MOTOR, 0 );
  orc_motor_set_signed( s_orc, ORC_RIGHT_MOTOR, 0 );
  return 0;
}

// ---- BASE MOVEMENT COMMANDS ---- //
void carmen_base_command_velocity(double desired_velocity, 
				  double current_velocity,
				  int WHICH_MOTOR)
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

  orc_motor_set_signed( s_orc, WHICH_MOTOR, command_pwm );
}

// input the velocity in terms of translational and rotational components
int carmen_base_direct_set_velocity(double new_tv, double new_rv) //
{
  left_desired_velocity = new_tv;
  right_desired_velocity = new_tv;

  right_desired_velocity += new_rv/2;
  left_desired_velocity -= new_rv/2;

  carmen_base_command_velocity(left_desired_velocity, s_left_velocity,
       ORC_LEFT_MOTOR);
  carmen_base_command_velocity(right_desired_velocity, s_right_velocity,
       ORC_RIGHT_MOTOR);

  time_since_last_command = carmen_get_time() - last_command_time;
  last_command_time = carmen_get_time();

  printf("carmen_base_direct_set_velocity: tv=%.2f, rv=%.2f\n", 
	 new_tv, new_rv);
  double last_update_time;
  carmen_base_direct_update_status( &last_update_time );

  return 0;
}

// ----- GETTING STATE ----- //

// updates the internal state variables
int carmen_base_direct_update_status(double* packet_timestamp) //
{
  // fill in the time stamp and update internal time
  double curr_time = carmen_get_time();
  s_delta_time = curr_time - s_time;
  s_time = curr_time;
  *packet_timestamp = curr_time;
  
  // sonar is not implemented in orc 5 so we don't deal with this
  s_left_range = 0.0;
  s_right_range = 0.0;

  // ir deleted since not being used

  // gripper moved to the arm section

  // bumpers
  s_bumpers[0] = orc_digital_read( s_orc, ORC_BUMPER_PORT0 );
  s_bumpers[1] = orc_digital_read( s_orc, ORC_BUMPER_PORT1 );
  s_bumpers[2] = orc_digital_read( s_orc, ORC_BUMPER_PORT2 );
  s_bumpers[3] = orc_digital_read( s_orc, ORC_BUMPER_PORT3 );

  // read encoders
  int curr_left_tick = orc_quadphase_read( s_orc, ORC_LEFT_ENCODER_PORT );
  int curr_right_tick = orc_quadphase_read( s_orc, ORC_RIGHT_ENCODER_PORT );
  int left_delta = compute_delta_ticks_abs( curr_left_tick, s_left_tick );
  int right_delta = compute_delta_ticks_abs( curr_right_tick, s_right_tick );
  s_left_displacement = delta_tick_to_metres( left_delta );
  s_right_displacement = delta_tick_to_metres( right_delta ); 
  s_left_tick = curr_left_tick;
  s_right_tick = curr_right_tick;

  // set velocity information
  s_left_velocity = s_left_displacement / s_delta_time;
  s_right_velocity = s_right_displacement / s_delta_time;



  return 0;
}

int carmen_base_direct_get_integrated_state(double *x_p, double *y_p,
					    double *theta_p, double *tv_p,
					    double *rv_p) //
{
  *x_p = s_x;
  *y_p = s_y;
  *theta_p = s_theta;
  *tv_p = (s_left_velocity+s_right_velocity)/2;
  *rv_p = atan2(s_right_velocity-s_left_velocity, ORC_WHEEL_BASE);  
  return 0;
}

int carmen_base_query_low_level(double *left_disp, double *right_disp,
				double *delta_time)
{
  // update internal variables
  int err;
  err = carmen_base_direct_update_status(NULL);
  if (err < 0)
    return -1;

  // return displacement information
  *left_disp = s_left_displacement;
  *right_disp = s_right_displacement;
  *delta_time = s_delta_time;
  return 0;
}


// this contains hard coded values from RSS; not updated since
// current orc 5 robot does not use sonars
int carmen_base_direct_get_sonars(double *ranges, carmen_point_t *positions,
				  int num_sonars) //
{
  if (num_sonars >= 1) {
    ranges[0] = s_left_range;
    positions[0].x = .05;
    positions[0].y = -.10;
    positions[0].theta = -M_PI/2;
  }
  if (num_sonars >= 2) {
    ranges[1] = s_right_range;
    positions[1].x = .05;
    positions[1].y = .10;
    positions[1].theta = M_PI/2;
  }

  return 0;
}

int carmen_base_direct_get_bumpers(unsigned char *bumpers_p, int num_bumpers) //
{
  if (num_bumpers >= 1)
    bumpers_p[0] = s_bumpers[0];
  if (num_bumpers >= 2)
    bumpers_p[1] = s_bumpers[1];
  if (num_bumpers >= 3)
    bumpers_p[2] = s_bumpers[2];
  if (num_bumpers >= 4)
    bumpers_p[3] = s_bumpers[3];

  return 4;
}

// ----- HELPER FUNCTIONS ----- //
static double delta_tick_to_metres(int delta_tick)
{
  double revolutions = (double)delta_tick/
    (double)(ORC_ENCODER_RESOLUTION*ORC_GEAR_RATIO);
  double radians = revolutions*2*M_PI;
  double metres = radians*(ORC_WHEEL_DIAMETER/2.0);

  return metres;
}

// we assume the direction is the smallest route around the circle
static int compute_delta_ticks_abs( int curr, int prev )
{

  // compute the number of ticks traversed short and long way around
  // and pick the path with the minimal distance
  int abs_reg = abs( curr - prev );
  int abs_opp = 65536 - abs_reg;
  int actual_delta = min( abs_reg, abs_opp );

  // give the angle the correct sign -- ccw is positive
  if( ( curr > prev && actual_delta == abs_reg ) || 
      ( curr < prev && actual_delta == abs_opp ) )
    {
      actual_delta = -actual_delta;
    }
  
  return actual_delta;
}

static int min( int a, int b )
{
  return ( a < b ) ? a : b ;
}

/* --------------------------------------------------------------

             FUNCTIONS BELOW ARE NOT YET IMPLEMENTED
	        DO NOT APPLY YET... OR EVER?? 
 	  -- some not even implemented in the RSS code

   --------------------------------------------------------------*/

// ---- SETTING CONTROLS ---- //
int carmen_base_direct_sonar_on(void) //
{
  printf("carmen_base_direct_sonar_on\n");
  s_sonar_on = 1;

  return 2;
}

int carmen_base_direct_sonar_off(void) //
{
  s_sonar_on = 0;
  return 2;
}


int carmen_base_direct_set_acceleration(double new_acceleration) //
{
  s_acceleration = new_acceleration;
  return 0;
}

int carmen_base_direct_set_deceleration(double new_deceleration) //
{
  s_deceleration = new_deceleration;
  return 0;
}

// ----- GETTING STATE INFORMATION ----- //
int carmen_base_direct_get_state(double *disp_p __attribute__ ((unused)), 
				 double *rot_p __attribute__ ((unused)),
				 double *tv_p __attribute__ ((unused)), 
				 double *rv_p __attribute__ ((unused)))//
{

  carmen_die("Turn base_hardware_integrator on\n");

  return 0;
}

int carmen_base_query_encoders(double *disp_p __attribute__ ((unused)),
			       double *rot_p __attribute__ ((unused)),
			       double *tv_p __attribute__ ((unused)),
			       double *rv_p __attribute__ ((unused))) //
{
  carmen_die("Turn base_hardware_integrator on\n");

  return 0;
}


// ----- LOW LEVEL COMMUNICATION ----- //
int carmen_base_direct_send_binary_data(unsigned char *data, int size)//
{
  return carmen_serial_writen(s_serial_fd, data, size);
  return 0;
}

int  carmen_base_direct_get_binary_data(unsigned char **data
					__attribute__ ((unused)), 
					int *size __attribute__ ((unused)))//
{

  return 0;
}

