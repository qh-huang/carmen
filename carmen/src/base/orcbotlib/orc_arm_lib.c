#include <carmen/carmen.h>
#include "../arm_low_level.h"
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
#define ORC_ARM_P_GAIN 8
#define ORC_ARM_D_GAIN 15
#define ORC_ARM_I_GAIN 0 // 3
#define ORC_ARM_MIN_ANGULAR_VEL 0.00125 // Radians / second 
//#define ORC_ARM_FF_GAIN ((ORC_ARM_MAX_PWM / ORC_ARM_MAX_ANGULAR_VEL) * 0.9)
#define ORC_ARM_FF_GAIN ( (90 / ORC_ARM_MAX_ANGULAR_VEL ) * 0.9 );
#define ORC_ARM_THETA_P_GAIN 0.008
#define ORC_ARM_THETA_D_GAIN 0.028 // 0.018
#define ORC_ARM_THETA_I_GAIN 0.000 // 0.003
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

  // sets limits to default and velocities to zero
  int carmen_arm_direct_reset(void);
  
  // sets safe joint use limits -- input array of limits for each element
  void carmen_arm_direct_set_limits(double min_angle, double max_angle,
				  int min_pwm, int max_pwm);
  
  // this is OPEN loop, should be part of a larger control loop
  // sets desired joint angles and implements control for next time step
  void carmen_arm_direct_update_joints(double *joint_angles );

  // ----- gets ----- //
  void carmen_arm_direct_get_state(double *joint_angles, double *joint_currents,
				 double *joint_angular_vels, 
				 int *gripper_closed );

*/

/* 
   currently implemented:
   we assume an arm with three joints
   all joints are motors
   we don't allow 360 spins on the base (wire tangle)
*/



// ---- DEFINES ---- //
// for joint access in info arrays
#define ELBOW 0
#define SHOULDER 1
#define HIP 2

// constants
#define ORC_ARM_MAX_ANGULAR_VEL 0.0035 // Radians / second

// orc


// ---- STATIC VARIABLES ---- //
// basic information
static orc_t *s_orc;
static int active;
static int s_num_joints;
static carmen_arm_joint_t *s_joint_types;

// current joint information
static double s_arm_theta[3];
static double s_arm_angular_velocity[3];

// for the PID control
static double s_arm_iTerm[3];
static double s_arm_error_prev[3];

// limits of safe operation
static double s_min_angle[3];
static double s_max_angle[3];
static double s_min_pwm[3];
static double s_max_pwm[3];

// ---- INIT/QUIT/RESET ---- //
int carmen_arm_direct_initialize(orc_t *orc, int num_joints, 
				 carmen_arm_joint_t *joint_types ){
  s_active = 1;
  s_orc = orc;
  s_num_joints = num_joints;
  s_joint_types = joint_types;
  carmen_arm_direct_reset();
  return 0;
}

int carmen_arm_direct_shutdown(void){
  carmen_arm_direct_reset();
  s_active = 0;
}

int carmen_arm_direct_reset(void){

  // stop all motors
  orc_motor_set( s_orc, HIP, 0 );
  orc_motor_set( s_orc, ELBOW, 0 );
  orc_motor_set( s_orc, SHOULDER, 0 );

  // reset limits

}


// ---- SETS ---- //
void carmen_arm_direct_set_limits(double *min_angle, double *max_angle,
				  int *min_pwm, int *max_pwm){
  s_min_angle = min_angle;
  s_max_angle = max_angle;
  s_min_pwm = min_pwm;
  s_max_pwm = max_pwm;
}



// ---- GETS ---- //

// velezj: rssII Arm -- puts values from here into whoever's doing the querying
void carmen_arm_direct_get_state(double *joint_angles, double *joint_currents,
				 double *joint_angular_vels, int num_joint_angles)
{

  need to have to not complain

  // ??? where are these coming from ???
  num_joint_angles = num_joint_angles;
  joint_currents = joint_currents;

  can start at 0

  // ??? why did numbering start with 1 ??? also does the order matter here 
  // swapped the elbow and shoulder
  // put values into the output from what we have in here
  joint_angles[1] = s_arm_theta[ELBOW];
  joint_angles[2] = s_arm_theta[SHOULDER];
  joint_angles[3] = s_arm_theta[HIP];

  joint_angular_vels[1] = s_arm_angular_velocity[ELBOW];
  joint_angular_vels[2] = s_arm_angular_velocity[SHOULDER];
  joint_angular_vels[3] = s_arm_angular_velocity[HIP];
}


// velezj: rssII Arm
static void command_angular_velocity( double desired_angular_velocity, double current_angular_velocity, int current_pwm, unsigned char WHICH_MOTOR ) {
  
  double ffTerm = 0.0, pTerm = 0.0, dTerm = 0.0, velError = 0.0;
  double * iTermPtr;
  double * velErrorPrevPtr;;
  int command_pwm;
  int motor;
  int max_pwm, min_pwm;

  // get base parameters for your joint motor
  switch( WHICH_MOTOR ){
    ORC_HIP_MOTOR : motor = HIP; break;
    ORC_SHOULDER_MOTOR : motor = SHOULDER; break;
    ORC_ELBOW_MOTOR : motor = ELBOW; break;
  }
  iTermPtr = &s_arm_iTerm[motor];
  velErrorPrevPtr = &s_arm_error_prev[motor];
  max_pwm = s_max_pwm[motor];
  min_pwm = s_min_pwm[motor];
 
  // if there is non-zero desired velocity
  if (fabs(desired_angular_velocity) > .0005) {

    // make the angular velocity between the max bounds
    if (desired_angular_velocity > ORC_ARM_MAX_ANGULAR_VEL)
      desired_angular_velocity = ORC_ARM_MAX_ANGULAR_VEL;
    if (desired_angular_velocity < -ORC_ARM_MAX_ANGULAR_VEL)
      desired_angular_velocity = -ORC_ARM_MAX_ANGULAR_VEL;

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

  } else {
    command_pwm = 0;
    *iTermPtr = 0.0;
  } // end if motion desired

  // debug
  if( command_pwm != 0 ) {
    printf( "[ %c ] p: %f  i: %f  d: %f   ve: %f\n", which, pTerm, *iTermPtr, dTerm, velError );  
    printf( "[ %c ] sending PWM: %d\n", which, command_pwm );
  }

  orc_motor_set(orc, motor, command_pwm);

}

// velezj: rssII Arm
void carmen_arm_control() {

  double shoulder_theta_delta = shoulder_desired_theta - shoulder_theta;
  double elbow_theta_delta = elbow_desired_theta - elbow_theta;
  
  double pTerm = 0.0, dTerm = 0.0;
  double *iTermPtr;

  // shoulder
  iTermPtr = &shoulder_theta_iTerm;
  //printf( "control: shoulder diff: %f    ( %f  -  %f )\n", shoulder_theta_delta, shoulder_desired_theta, shoulder_theta );
  if( fabs( shoulder_theta_delta ) > 0.01 ) {
    pTerm = shoulder_theta_delta * ORC_ARM_THETA_P_GAIN;
    *iTermPtr += ( shoulder_theta_delta * ORC_ARM_THETA_I_GAIN );
    dTerm = ( shoulder_theta_delta - shoulder_theta_error_prev ) * ORC_ARM_THETA_D_GAIN;
    shoulder_desired_angular_velocity = ( pTerm + *iTermPtr * dTerm );
    bound_value( &shoulder_desired_angular_velocity, -ORC_ARM_MAX_ANGULAR_VEL, ORC_ARM_MAX_ANGULAR_VEL );
    if( fabs( shoulder_desired_angular_velocity ) < ORC_ARM_MIN_ANGULAR_VEL ) {
      shoulder_desired_angular_velocity = sign( shoulder_desired_angular_velocity ) * ORC_ARM_MIN_ANGULAR_VEL;
    }

    printf( "contorl: shoulder: p: %f   i: %f   d: %f    av: %f\n", pTerm, *iTermPtr, dTerm, shoulder_desired_angular_velocity );
  } else {
    shoulder_desired_angular_velocity = 0.0;
    *iTermPtr = 0.0;
    //printf( "control: shoulder: done!\n" );
  }

  //elbow
  iTermPtr = &elbow_theta_iTerm;
  //printf( "control: elbow diff: %f    ( %f  -  %f )\n", elbow_theta_delta, elbow_desired_theta, elbow_theta );
  if( fabs( elbow_theta_delta ) > 0.01 ) {
    pTerm = elbow_theta_delta * ORC_ARM_THETA_P_GAIN;
    *iTermPtr += ( elbow_theta_delta * ORC_ARM_THETA_I_GAIN );
    dTerm = ( elbow_theta_delta - elbow_theta_error_prev ) * ORC_ARM_THETA_D_GAIN;
    elbow_desired_angular_velocity = ( pTerm + *iTermPtr * dTerm );
    bound_value( &elbow_desired_angular_velocity, -ORC_ARM_MAX_ANGULAR_VEL, ORC_ARM_MAX_ANGULAR_VEL );
    if( fabs( elbow_desired_angular_velocity ) < ORC_ARM_MIN_ANGULAR_VEL ) {
      elbow_desired_angular_velocity = sign( elbow_desired_angular_velocity ) * ORC_ARM_MIN_ANGULAR_VEL;
    }

    printf( "contorl: elbow: p: %f   i: %f   d: %f    av: %f\n", pTerm, *iTermPtr, dTerm, elbow_desired_angular_velocity );
  } else {
    elbow_desired_angular_velocity = 0.0;
    *iTermPtr = 0.0;
    //printf( "control: elbow: done!\n" );
  }

  command_angular_velocity( shoulder_desired_angular_velocity, shoulder_angular_velocity, shoulder_pwm, ORC_SHOULDER_MOTOR );
  command_angular_velocity( elbow_desired_angular_velocity, elbow_angular_velocity, elbow_pwm, ORC_ELBOW_MOTOR );
} 


// ---- HELPER FUNCTIONS ---- //

// velezj: for RssII Arm
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
void bound_value( double *vPtr, double min, double max ) {
  if( *vPtr < min )
    *vPtr = min;
  if( *vPtr > max )
    *vPtr = max;
}

// velezj: rssII Arm
double sign( double v ) {
  if( v < 0.0 )
    return -1.0;
  return 1.0;
}

// this reads relevant arm sensors and updates static values
void carmen_arm_direct_update_status(void)
{
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



