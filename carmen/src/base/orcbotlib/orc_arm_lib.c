#include <carmen/carmen.h>
#include "../arm_low_level.h"
#include "orc_arm_constants.h"
#include <sys/ioctl.h>
#include <limits.h>

/*

// velezj: For RssII Arm
#define ORC_SHOULDER_MOTOR 1
#define ORC_SHOULDER_MOTOR_ACTUAL_PWM 17
#define ORC_SHOULDER_MOTOR_DIR 25
#define ORC_SHOULDER_MOTOR_QUAD_PORT 16
#define ORC_SHOULDER_MOTOR_ENCODER 7
#define ORC_SHOULDER_MOTOR_DIRECTION_SIGN 1
#define ORC_ELBOW_MOTOR 3
#define ORC_ELBOW_MOTOR_ACTUAL_PWM 23
#define ORC_ELBOW_MOTOR_DIR 26
#define ORC_ELBOW_MOTOR_QUAD_PORT 18
#define ORC_ELBOW_MOTOR_ENCODER 10
#define ORC_ELBOW_MOTOR_DIRECTION_SIGN -1
#define ORC_ARM_GEAR_REDUCTION ( 65.5 * 5.0 )
#define ORC_ARM_TICKS_PER_RADIAN ( ORC_ENCODER_RESOLUTION * ORC_ARM_GEAR_REDUCTION / ( 2.0 * M_PI ) )
#define ORC_ELBOW_MAX_PWM 40 // 27
#define ORC_ELBOW_MIN_PWM 35 // 22
#define ORC_SHOULDER_MAX_PWM 50 // 37
#define ORC_SHOULDER_MIN_PWM 42 // 30


//#define ORC_ARM_FF_GAIN ((ORC_ARM_MAX_PWM / ORC_ARM_MAX_ANGULAR_VEL) * 0.9)

#define ORC_ARM_GRIPPER_SERVO 0 // 0
#define ORC_ARM_GRIPPER_MIN_PWM 8600
#define ORC_ARM_GRIPPER_PWM_PER_RADIAN 2214.2 

// velezj: rssII Arm -- used by all functions
static double shoulder_desired_theta = 0.0, elbow_desired_theta = 0.0;
static double shoulder_theta_iTerm = 0.0, elbow_theta_iTerm = 0.0;
static double shoulder_theta_error_prev = 0.0, elbow_theta_error_prev = 0.0;
static double shoulder_iTerm = 0.0, elbow_iTerm = 0.0;
static double shoulder_error_prev = 0.0, elbow_error_prev = 0.0;
static double shoulder_desired_angular_velocity = 0.0, elbow_desired_angular_velocity = 0.0;

static int shoulder_pwm, elbow_pwm;
static int shoulder_last_tick = 0, elbow_last_tick = 0;

// copied from arm_low_level.h ... stuff that should get implemented

  // passes in an orc pointer and tell arm how many joints
  int carmen_arm_direct_initialize(orc_t *orc, int num_joints, 
				   carmen_arm_joint_t *joint_types );
  int carmen_arm_direct_shutdown(void);

  // ----- sets ----- //

  // sets error and velocities to zero
  int carmen_arm_direct_reset(void);
  
  // sets safe joint use limits -- input array of limits for each element
  void carmen_arm_direct_set_limits(double min_angle, double max_angle,
				  int min_pwm, int max_pwm);
  


  // ----- gets ----- //


*/

/* 
   currently implemented:
   we assume an arm with three joints 
   all joints are motors -- not using joint types
   we don't allow 360 spins on the base (wire tangle)
*/

// ---- STATIC VARIABLES ---- //
// basic information
static orc_t *s_orc;
static int active;
static int s_num_joints;
static carmen_arm_joint_t *s_joint_types;
static double s_time;   

// current joint data
static double *s_arm_theta;
static double *s_arm_tick;
static double *s_arm_angular_velocity;
static double *s_arm_iTerm;
static double *s_arm_error_prev;
static double *s_arm_current;
 
// limits of safe operation
static double *s_min_angle;
static double *s_max_angle;
static int *s_min_pwm;
static int *s_max_pwm;

// 

// ---- INIT/QUIT/RESET ---- //
int carmen_arm_direct_initialize(carmen_base_model_t *base, int num_joints, 
				 carmen_arm_joint_t *joint_types ){
 
  // create an orc
  s_orc = orc_create( base );

  // initialize our static variable
  *s_joint_types = calloc( num_joints, sizeof( double ) );
  carmen_test_alloc( s_joint_types );s
  *s_arm_theta = calloc( num_joints, sizeof( double ) );
  carmen_test_alloc( s_arm_theta );
  *s_arm_tick = calloc( num_joints, sizeof( double ) );
  carmen_test_alloc( s_arm_tick );
  *s_arm_angular_velocity = calloc( num_joints, sizeof( double ) );
  carmen_test_alloc( s_arm_angular_velocity );
  *s_arm_iTerm = calloc( num_joints, sizeof( double ) );
  carmen_test_alloc( s_arm_iTerm );
  *s_arm_error_prev = calloc( num_joints, sizeof( double ) );
  carmen_test_alloc( s_arm_error_prev );
  *s_arm_current = calloc( num_joints, sizeof( double ) );
  carmen_test_alloc( s_arm_current );
  *s_min_angle = calloc( num_joints, sizeof( double ) );
  carmen_test_alloc( s_min_angle );
  *s_max_angle = calloc( num_joints, sizeof( double ) );
  carmen_test_alloc( s_max_angle );
  *s_min_pwm = calloc( num_joints, sizeof( int ) );
  carmen_test_alloc( s_min_pwm );
  *s_max_pwm = calloc( num_joints, sizeof( int ) );
  carmen_test_alloc( s_max_pwm );
  
  // set them to initial values
  s_active = 1;
  s_time = carmen_get_time();
  s_orc = (orc_t)base;
  s_num_joints = num_joints;
  memcpy( s_joint_types, joint_types, sizeof( int ) );
  memcpy( s_min_angle, MIN_THETA, sizeof( double ) );
  memcpy( s_max_angle, MAX_THETA, sizeof( double ) );
  memcpy( s_min_pwm, MIN_PWM, sizeof( int ) );
  memcpy( s_max_pwm, MAX_PWM, sizeof( int ) );
  carmen_arm_direct_reset();

  return 0;
}

int carmen_arm_direct_shutdown(void){
  carmen_arm_direct_reset();

  // free our static variables
  free( s_joint_types );
  free( s_arm_theta );
  free( s_arm_tick );
  free( s_arm_angular_velocity );
  free( s_arm_iTerm );
  free( s_arm_error_prev );
  free( s_arm_current );
  free( s_min_angle );  
  free( s_max_angle );
  free( s_min_pwm );
  free( s_max_pwm );
  
  // destroy the orc
  orc_destroy( s_orc );
  s_active = 0;
}

int carmen_arm_direct_reset(void){

  // stop all motors and reset the error, tick count to zero
  for( int i = 0; i < s_num_joints; ++i ){
    orc_motor_set( s_orc, MOTOR_PORTMAP[i], 0 );
    s_arm_tick[i] = 0;
    s_arm_iTerm[i] = 0;
    s_arm_error_prev[i] = 0;
  }
}


// ---- SETS ---- //
void carmen_arm_direct_set_limits(double *min_angle, double *max_angle,
				  int *min_pwm, int *max_pwm){

  memcpy( s_min_angle, min_angle, sizeof( double ) );
  memcpy( s_max_angle, max_angle, sizeof( double ) );
  memcpy( s_min_pwm, min_pwm, sizeof( int ) );
  memcpy( s_max_pwm, max_pwm, sizeof( int ) );
}

// needs to be made 360 safe!!!!

// this is OPEN loop, should be part of a larger control loop
// sets desired joint angles and implements control for next time step
// most of this code is adapted from the RSS II arm by velezj
void carmen_arm_direct_update_joints( double *desired_angles ){

  double desired_vel[num_joints];
  double pTerm = 0.0, dTerm = 0.0; double *iTermPtr;

  // update to our current position
  update_internal_data();

  // determine the desired angular velocities 
  for( int i = 0; i < num_joints; ++i ){

    // get the desired angular change
    double theta_delta = desired_angles[i] - s_arm_theta[i];
    double desired_angular_velocity = 0;

    // compute and make sure velocites are within limits
    iTermPtr = &s_arm_iTerm[i];
    if( fabs( theta_delta ) > 0.01 ) {
      pTerm = theta_delta * ORC_ARM_THETA_P_GAIN;
      *iTermPtr += ( theta_delta * ORC_ARM_THETA_I_GAIN );
      dTerm = ( theta_delta - s_theta_error_prev[i] ) * ORC_ARM_THETA_D_GAIN;
      desired_angular_velocity = ( pTerm + *iTermPtr * dTerm );
      bound_value( &desired_angular_velocity, -ORC_ARM_MAX_ANGULAR_VEL, ORC_ARM_MAX_ANGULAR_VEL );
      if( fabs( desired_angular_velocity ) < ORC_ARM_MIN_ANGULAR_VEL ) {
	desired_angular_velocity = sign( desired_angular_velocity ) * ORC_ARM_MIN_ANGULAR_VEL;
      }
      printf( "contorl: shoulder: p: %f   i: %f   d: %f    av: %f\n", pTerm, *iTermPtr, dTerm, shoulder_desired_angular_velocity );
    } else {
      desired_angular_velocity = 0.0;
      *iTermPtr = 0.0;   
    }

    // set the values into our array
    desired_vel[i] = desired_angular_velocity;
  }

  // actually command the velocities
  for( int i = 0; i < num_joints, ++i ){
    command_angular_velocity( desired_vel[i], s_angular_velocity[i], PWM?????, MOTOR_PORTMAP[i] );
  }
}

// ---- GETS ---- //
void carmen_arm_direct_get_state(double *joint_angles, double *joint_currents,
				 double *joint_angular_vels, int *gripper_closed ){

  update_internal_data();

  // put values into the output from what we have in here
   for( int i = 0; i < s_num_joints; ++i ){
     joint_angles[i] = s_arm_theta[i]; 
     joint_currents[i] = s_arm_currents[i];
     joint_angular_vels[i] = s_arm_angular_velocity[i];
   }
 
  // for now, since the gripper's not being used
  *gripper_closed = 0;
}


// velezj: rssII Arm
static void command_angular_velocity( double desired_angular_velocity, double current_angular_velocity, 
				      int current_pwm, int joint ) {
  
  double ffTerm = 0.0, pTerm = 0.0, dTerm = 0.0, velError = 0.0;
  double * iTermPtr;
  double * velErrorPrevPtr;;
  int command_pwm;
  int max_pwm, min_pwm;

  // get base parameters for your joint motor
  iTermPtr = &s_arm_iTerm[joint];
  velErrorPrevPtr = &s_arm_error_prev[joint];
  max_pwm = s_max_pwm[joint];
  min_pwm = s_min_pwm[joint];
 
  // if there is non-zero desired velocity
  if (fabs(desired_angular_velocity) > .0005) {

    // compute PID terms
    velError = (desired_angular_velocity - current_angular_velocity);
    ffTerm = desired_angular_velocity * ORC_ARM_FF_GAIN;
    pTerm = velError * ORC_ARM_P_GAIN;
    *iTermPtr += velError * ORC_ARM_I_GAIN;
    dTerm = (velError - *velErrorPrevPtr) * ORC_ARM_D_GAIN;
    *velErrorPrevPtr = velError;

    // set the pwm between the desired bounds
    current_pwm = (int)(ffTerm + pTerm + *iTermPtr + dTerm);
    if (abs(current_pwm) > max_pwm)
      current_pwm = (current_pwm > 0 ? max_pwm : -max_pwm);
    if( abs( current_pwm) < min_pwm )
      current_pwm = ( current_pwm > 0 ? min_pwm : -min_pwm );

    command_pwm = abs( current_pwm );

  } else {
    command_pwm = 0;
    *iTermPtr = 0.0;
  } // end if motion desired

  // debug
  if( command_pwm != 0 ) {
    printf( "[ %c ] p: %f  i: %f  d: %f   ve: %f\n", which, pTerm, *iTermPtr, dTerm, velError );  
    printf( "[ %c ] sending PWM: %d\n", which, command_pwm );
  }

  orc_motor_set(s_orc, MOTOR_PORTMAP[joint], command_pwm);
}


// ---- HELPER FUNCTIONS ---- //

// this reads relevant arm sensors and updates static values
static void update_internal_data(void)
{

  static double s_arm_theta[3];
  static double s_arm_angular_velocity[3];
  static double s_arm_current[3];

 double curr_time = carment_get_time();

  // updates arm angles, velocities, and currents
  for( int i = 0; i < s_num_joints; ++i ){
    int port = MOTOR_PORTMAP[i];
    int curr_tick_count = orc_quadphase_read( s_orc, ENCODER_PORT[i] );
    int prev_tick_count = s_arm_tick[i];

    // compute the change in angle since the last update
    double delta_theta = compute_delta_theta( curr_tick_count, prev_tick_count,
					      TICKS_PER_RADIAN[i] );

    // set variables
    s_arm_theta[i] = carmen_normalize_theta( delta_theta + s_arm_theta[i] );
    s_arm_angular_velocity[i] = delta_theta / ( curr_time - s_time );
    s_arm_current[i] = orc_analog_read( s_orc, 16 + port );  // motor ports are 16 + reg
  }			   
				   
  s_time = curr_time;  
}

// we assume the direction is the smallest route around the circle
static double compute_delta_theta( int curr, int prev, double ticks_per_radian )
{

  // compute the number of ticks traversed short and long way around
  // and pick the path with the minimal distance
  abs_reg = abs( curr - prev );
  abs_opp = 65536 - abs_reg;
  actual_delta = min( abs_reg, abs_opp );
  double theta = (double)actual_delta * ticks_per_radian;

  // give the angle the correct sign -- ccw is positive
  if( ( curr > prev && actual_delta == abs_reg ) || 
      ( curr < prev && actual_delta == abs_opp ) )
    {
      theta = -theta;
    }
  
  return theta;
}

static int min( int a, int b ){
  return ( a < b ) ? a : b ;
}

static double sign( double v ) {
  if( v < 0.0 )
    return -1.0;
  return 1.0;
}

static void bound_value( double *vPtr, double min, double max ) {
  if( *vPtr < min )
    *vPtr = min;
  if( *vPtr > max )
    *vPtr = max;
}

/*

  unsigned char *buffer;
  double start_time = carmen_get_time();

  if (!initialized) {
    start = carmen_get_time();
    initialized = 1;
    // velezj: RssII Arm
    shoulder_theta = 0.0;
    elbow_theta = 0.0;
    shoulder_last_tick = orc_quadphase_read(orc, ORC_SHOULDER_MOTOR_ENCODER);
    elbow_last_tick = orc_quadphase_read(orc, ORC_ELBOW_MOTOR_ENCODER);

    shoulder_pwm = orc_analog_read(orc, ORC_SHOULDER_MOTOR_ACTUAL_PWM);
    elbow_pwm = orc_analog_read(orc, ORC_SHOULDER_MOTOR_ACTUAL_PWM);

    return;
  }

  shoulder_pwm = orc_analog_read(orc, ORC_SHOULDER_MOTOR_ACTUAL_PWM);
  elbow_pwm = orc_analog_read(orc, ORC_ELBOW_MOTOR_ACTUAL_PWM);

  shoulder_tick = orc_quadphase_read(orc, ORC_SHOULDER_MOTOR_ENCODER);
  elbow_tick = orc_quadphase_read(orc, ORC_ELBOW_MOTOR_ENCODER);
  
  // velezj: For RssII Arm

  shoulder_delta_tick = shoulder_tick - shoulder_last_tick;
  if (shoulder_delta_tick > SHRT_MAX/2)
    shoulder_delta_tick = shoulder_delta_tick - 2*SHRT_MAX;
  if (shoulder_delta_tick < -SHRT_MAX/2)
    shoulder_delta_tick = shoulder_delta_tick + 2*SHRT_MAX;

  elbow_delta_tick = elbow_tick - elbow_last_tick;
  if (elbow_delta_tick > SHRT_MAX/2)
    elbow_delta_tick = elbow_delta_tick - 2*SHRT_MAX;
  if (elbow_delta_tick < -SHRT_MAX/2)
    elbow_delta_tick = elbow_delta_tick + 2*SHRT_MAX;

  shoulder_last_tick = shoulder_tick;
  elbow_last_tick = elbow_tick;

  shoulder_angle_change = delta_ticks_to_angle(shoulder_delta_tick);
  elbow_angle_change = delta_ticks_to_angle(elbow_delta_tick);
  if( fabs( shoulder_desired_theta - shoulder_theta ) > 0.01 ) {
    printf( "shoulder_delta_theta: %f  (%d)\n", shoulder_angle_change, shoulder_delta_tick );
  }
  if( fabs( elbow_desired_theta - elbow_theta ) > 0.01 ) {
    printf( "elbow_delta_theta: %f  (%d)\n", elbow_angle_change, elbow_delta_tick );
  }

  // velezj: For RssII Arm
  shoulder_theta = shoulder_theta + shoulder_angle_change;
  elbow_theta = elbow_theta + elbow_angle_change;
  // since elbow relative to shoulder and when the shoulder moves to elbow angle
  // in fact does NOT change with it, so we must update our elbow angle even when
  // only the shoulder moves
  //elbow_theta -= shoulder_angle_change; 
  shoulder_angular_velocity = shoulder_angle_change / delta_slave_time;
  elbow_angular_velocity = elbow_angle_change / delta_slave_time;
  if( fabs( shoulder_desired_theta - shoulder_theta ) > 0.01 || fabs( elbow_desired_theta - elbow_theta ) > 0.01 ) {
    printf( "shoulder_theta: %f  (av = %f)\n", shoulder_theta, shoulder_angular_velocity );
    printf( "elbow_theta:    %f  (av = %f)\n", elbow_theta, elbow_angular_velocity  );
  }

  free(buffer);

  if(0)
  carmen_warn("time to update: %.2f\n", carmen_get_time() -
	      start_time);
  return 0;
}

static double delta_ticks_to_angle( int delta_ticks ) 
{
  double radians = (double)delta_ticks / (double)ORC_ARM_TICKS_PER_RADIAN;
  return radians;
}

static double voltage_to_current(unsigned short voltage)
{
  double current;
  current = voltage*5.0/65536.0;
  // V=IR, R=0.18 ohm
  current = current/0.18;
  return current;
}


// velezj: rssII Arm


// velezj: rssII Arm

*/




