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

#include <sys/ioctl.h>
#include <values.h>
#include "cerebellum_com.h"

#undef _REENTRANT

#ifdef _REENTRANT
#include <pthread.h>
static pthread_t main_thread;
static int thread_is_running = 0;
#endif


// Normalize angle to domain -pi, pi
//
#define NORMALIZE(z) atan2(sin(z), cos(z))

// might need to define a longer delay to wait for acks
#define TROGDOR_DELAY_US 10000

/************************************************************************/
/* Physical constants, in meters, radians, seconds (unless otherwise noted)
 * */
//#define TROGDOR_AXLE_LENGTH    0.317
//#define TROGDOR_WHEEL_DIAM     0.10795  /* 4.25 inches */
//#define TROGDOR_WHEEL_CIRCUM   (TROGDOR_WHEEL_DIAM * M_PI)
//#define TROGDOR_TICKS_PER_REV  11600.0
//#define TROGDOR_M_PER_TICK     (TROGDOR_WHEEL_CIRCUM / TROGDOR_TICKS_PER_REV)
/* the internal PID loop runs every 1.55ms (we think) */
//#define TROGDOR_PID_FREQUENCY  (1/1.55e-3)
//#define TROGDOR_MPS_PER_TICK   (TROGDOR_M_PER_TICK * TROGDOR_PID_FREQUENCY)

/* assuming that the counts can use the full space of a signed 32-bit int
 * */
//#define TROGDOR_MAX_TICS 2147483648U

/* for safety */
//#define TROGDOR_MAX_WHEELSPEED   4.0


#define        CEREBELLUM_TIMEOUT          (2.0)
#define        METERS_PER_INCH        (0.0254)
#define        INCH_PER_METRE         (39.370)

/* the internal PID loop runs every 1.55ms (we think) */
#define        OBOT_LOOP_FREQUENCY  (1/1.55e-3)

#define        OBOT_WHEEL_CIRCUMFERENCE  (4.25 * METERS_PER_INCH * 3.1415926535)
#define        OBOT_TICKS_PER_REV       (11600.0)
#define        OBOT_WHEELBASE        (.317)


#define        METERS_PER_OBOT_TICK (OBOT_WHEEL_CIRCUMFERENCE / OBOT_TICKS_PER_REV)
#define        TICKS_PER_MPS (1.0/(METERS_PER_OBOT_TICK * OBOT_LOOP_FREQUENCY))

#define        ROT_VEL_FACT_RAD (OBOT_WHEELBASE*TICKS_PER_MPS/2.0)

#define        MAXV             (4.5*TICKS_PER_MPS)

//this is in the current iteration of the motor driver
#define        MAX_CEREBELLUM_ACC 10

#define        MIN_OBOT_TICKS (4.0)
#define        MAX_READINGS_TO_DROP (10)

static char *dev_name;
static int State[4];
static int set_velocity = 0;
static int fire_gun = 0;
static int command_vl = 0, command_vr = 0;
static double last_command = 0;
static double last_update = 0;
static int timeout = 0;
static int moving = 0;
static double current_acceleration;
static double stopping_acceleration;
static carmen_robot_config_t robot_config;
static carmen_base_odometry_message odometry;
static int use_sonar = 0;

static double x, y, theta;
static double voltage;
static double voltage_time_elapsed = 0;

static int temperature_fault;
static int left_temperature,right_temperature;
static double temperature_time_elapsed = 0;

static int
initialize_robot(char *dev)
{
  int result;
  double acc;

  carmen_terminal_cbreak(0);

  result = carmen_cerebellum_connect_robot(dev);
  //  if(result != 0)
  //  return -1;

  acc = robot_config.acceleration*TICKS_PER_MPS;

  // SNEAKY HACK
  result = carmen_cerebellum_ac(25);
  current_acceleration = robot_config.acceleration;

  //  printf("TROGDOR_M_PER_TICK: %lf\n", TROGDOR_M_PER_TICK);
  printf("METERS_PER_OBOT_TICK: %lf\n", METERS_PER_OBOT_TICK);
  //printf("TROGDOR_MPS_PER_TICK: %lf\n",TROGDOR_MPS_PER_TICK);
  printf("1/TICKS_PER_MPS: %lf\n",1.0/TICKS_PER_MPS);

  if(result != 0)
    return 1;

  last_update = carmen_get_time_ms();

  return 1;
}

static void 
reset_status(void)
{
  char *host;
  odometry.x = 0.0;
  odometry.y = 0.0;
  odometry.theta = 0.0;
  odometry.tv = 0.0;
  odometry.rv = 0.0;
  odometry.timestamp = 0.0;

  host = carmen_get_tenchar_host_name();
  strcpy(odometry.host, host);   
}

static int
update_status(void)  /* This function takes approximately 60 ms to run */
{
  double vl, vr;
  static double last_published_update;
  static int initialized = 0;
  static short last_left_tick = 0, last_right_tick = 0;
  static int transient_dropped = 0;
  
  short left_delta_tick, right_delta_tick;
  double left_delta, right_delta;

  double delta_angle;
  double delta_distance;
  double max_ticks;

  //make sure that this measurement doesn't come from some time in the future
  if (last_published_update >= odometry.timestamp)
    {
      return 0; 
      carmen_warn("Invalid.\n");
    }

  //sometimes the robot returns 0 for left tic or right tic
  //this is an error, if that is the case, the reading is ignored

  if(State[0] == 0 || State[1] == 0)
    {
      carmen_warn("0 odometry returned, ignoring it.\n");
      return(0);
    }

  vl = ((double)State[2]) * TICKS_PER_MPS;
  vr = ((double)State[3]) * TICKS_PER_MPS;

  odometry.tv = 0.5 * (vl + vr);
  odometry.rv = (vr - vl) * ROT_VEL_FACT_RAD;
  
  if (!initialized)
    {
      last_left_tick = State[0];
      last_right_tick = State[1];
      initialized = 1;
      
      x = y = theta = 0;

      return 0;
    }

  //printf("SHRT_MIN %d SHRT_MAX %d\n",SHRT_MIN,SHRT_MAX);


  /* update odometry message */

  left_delta_tick = State[0] - last_left_tick;

  //this will turn a rollover into a small number
  if (left_delta_tick > SHRT_MAX/2)
    left_delta_tick += SHRT_MIN;
  if (left_delta_tick < -SHRT_MAX/2)
    left_delta_tick -= SHRT_MIN;
  left_delta = left_delta_tick*METERS_PER_OBOT_TICK;

  right_delta_tick = State[1] - last_right_tick;

  //these ifs will turn a rollover into a small number
  if (right_delta_tick > SHRT_MAX/2)
    right_delta_tick += SHRT_MIN;
  if (right_delta_tick < -SHRT_MAX/2)
    right_delta_tick -= SHRT_MIN;
  right_delta = right_delta_tick*METERS_PER_OBOT_TICK;

  //no wheel should go faster than 3.0 m/s
  max_ticks = 3.0 /( METERS_PER_OBOT_TICK / (odometry.timestamp-last_published_update)) ;

  //  printf("maxticks: %lf, time elapsed: %lf\n", max_ticks, (odometry.timestamp-last_published_update));

  if(abs(left_delta_tick) > max_ticks || abs(right_delta_tick) > max_ticks)
    {
      if( transient_dropped < MAX_READINGS_TO_DROP )
	{
	  printf("CBldt: %d rdt: %d\n",left_delta_tick,right_delta_tick); 
	  printf("delta tick unreal trying a drop\n");
	  transient_dropped++;
	  return(1);
	}
      else
	{
	  printf("accepting large tick reading\n");
	  transient_dropped =0;
	}
    }


  if (fabs(right_delta - left_delta) < .001) 
    delta_angle = 0;
  else
    {
      delta_angle = (left_delta-right_delta)/OBOT_WHEELBASE;
    }

  if(delta_angle > 0.5)
    {
      if( transient_dropped < MAX_READINGS_TO_DROP)
	{
	  printf("CB:delta_angle: %lf, lt: %d rt: %d, olt: %d ort: %d",
		 delta_angle,State[0],State[1],last_left_tick,last_right_tick);
	  printf("dropping one reading\n");
	  transient_dropped++;
	  return(1);
	}
      else
	{
	  printf("accepting large delta reading\n");
	  transient_dropped = 0;
	}


    }



  delta_distance = (right_delta + left_delta) / 2.0;
  
  
  x += delta_distance * cos(theta);
  y += delta_distance * sin(theta);
  theta += delta_angle;
  
  theta = NORMALIZE(theta);
  
  last_left_tick = State[0];
  last_right_tick = State[1];
  
  last_published_update = odometry.timestamp;
  
  odometry.x = x;
  odometry.y = y;
  odometry.theta = theta;


  //the transient dropped flag lets us drop one and only one anomalous reading
  if( transient_dropped > 0)
    transient_dropped--;


  //  printf("x: %lf, y: %lf, theta: %lf \n", x,y,theta);

  return 1;
}

static void 
command_robot(void)
{
  double time_elapsed = carmen_get_time_ms() - last_command;
  double current_time; 
  int acc;
  int error = 0;

  if(fire_gun)
    {
      //carmen_cerebellum_fire();
      fire_gun = 0;
    }

  if (set_velocity)
    {
      if(command_vl == 0 && command_vr == 0) 
	{
	  moving = 0;
	  //	  printf("carmen says S\n");
	  carmen_warn("S");
	}
      else 
	{
	  //	  printf("if not moving setting acceleration1\n");
	  if (!moving)
	    {
	      acc = robot_config.acceleration*TICKS_PER_MPS;
	      acc = MAX_CEREBELLUM_ACC;
	      do 
		{
		  error = carmen_cerebellum_ac(acc);
		  if (error < 0)
		    carmen_cerebellum_reconnect_robot(dev_name);
		} 
	      while (error < 0);
	      current_acceleration = robot_config.acceleration;
	    }
	  //	  printf("if not moving setting acceleration2\n");

	  carmen_warn("V");
	  moving = 1;
	}

      set_velocity = 0;
      timeout = 0;
      do 
	{
	  //	  printf("setting velocity1\n");

	  error = carmen_cerebellum_set_velocity(command_vl, command_vr);
	  if (error < 0)
	    carmen_cerebellum_reconnect_robot(dev_name);
	  //	  printf("setting velocity2\n");

	} 
      while (error < 0);

      last_command = carmen_get_time_ms();
    }
  else if(time_elapsed > CEREBELLUM_TIMEOUT && !timeout) 
    {
      carmen_warn("T");
      command_vl = 0;
      command_vr = 0;
      timeout = 1;
      moving = 0;
      do
	{
	  //	  printf("timeout1\n");

	  error=carmen_cerebellum_set_velocity(0, 0);      
	  if (error < 0)
	    carmen_cerebellum_reconnect_robot(dev_name);
	  //	  printf("timeout2\n");

	} 
      while (error < 0);
    }  
  else if (moving && current_acceleration == robot_config.acceleration)
    {
      acc = stopping_acceleration*TICKS_PER_MPS;
      do 
	{
	  //	  printf("stopping1\n");

	  error=carmen_cerebellum_ac(acc);
	  if (error < 0)
	    carmen_cerebellum_reconnect_robot(dev_name);
	  //	  printf("stopping2\n");

	} 
      while (error < 0);
      current_acceleration = stopping_acceleration;
    }

  do
    {
      //printf("getting state1\n");
      error=carmen_cerebellum_get_state(State+0, State+1, State+2, State+3);
      if (error < 0)
	carmen_cerebellum_reconnect_robot(dev_name);
      //printf("getting state2\n");

    } 
  while (error < 0);

  //increment the voltage check timer, check it every 3 seconds
  voltage_time_elapsed += time_elapsed;
  if( voltage_time_elapsed > 3000)
    {
      printf("V: ");
      error = carmen_cerebellum_get_voltage(&voltage);
      if( error < 0)
	{
	  printf("error, voltage command didn't work\n");
	}
      else
	printf("voltage: %lf\n",voltage);

      voltage_time_elapsed=0;
    }

  //increment the temperature check timer, check it every 3 seconds
  temperature_time_elapsed += time_elapsed;
  if( temperature_time_elapsed > 6000)
    {
      printf("C: ");
      error = carmen_cerebellum_get_temperatures(&temperature_fault,
						&left_temperature,
						&right_temperature);
      if( error < 0)
	{
	  printf("error, temperature command didn't work\n");
	}
      else
	printf("temp: fault:%d L:%d R:%d\n", temperature_fault,
	       left_temperature,right_temperature);

      temperature_time_elapsed=0;
    }
 

  
  current_time = carmen_get_time_ms();
  last_update = current_time;
  odometry.timestamp = current_time;

  if(use_sonar) 
    {
      carmen_warn("Sonar not supported by Cerebellum module\n");
      use_sonar = 0;
      carmen_param_set_variable("robot_use_sonar", "off", NULL);
    }
}

static int
read_cerebellum_parameters(int argc, char **argv)
{
  int num_items;

  carmen_param_t param_list[] = {
    {"cerebellum", "dev", CARMEN_PARAM_STRING, &dev_name, 0, NULL},
    {"robot", "use_sonar", CARMEN_PARAM_ONOFF, &use_sonar, 1, NULL}, 
    {"robot", "max_t_vel", CARMEN_PARAM_DOUBLE, &(robot_config.max_t_vel), 
     1, NULL},
    {"robot", "max_r_vel", CARMEN_PARAM_DOUBLE, &(robot_config.max_r_vel), 
     1, NULL},
    {"robot", "acceleration", CARMEN_PARAM_DOUBLE, 
     &(robot_config.acceleration), 1, NULL},
    {"robot", "deceleration", CARMEN_PARAM_DOUBLE, 
     &(stopping_acceleration), 1, NULL}};

  num_items = sizeof(param_list)/sizeof(param_list[0]);
  carmen_param_install_params(argc, argv, param_list, num_items);

  if(use_sonar)
    {
      carmen_warn("Sonar not supported by Cerebellum module\n");
      use_sonar = 0;
      carmen_param_set_variable("robot_use_sonar", "off", NULL);
    }

  if (robot_config.acceleration > stopping_acceleration) 
    carmen_die("ERROR: robot_deceleration must be greater or equal "
	       "than robot_acceleration\n");
  
  return 0;
}

static void 
velocity_handler(MSG_INSTANCE msgRef, BYTE_ARRAY callData,
		 void *clientData __attribute__ ((unused)))
{
  IPC_RETURN_TYPE err;
  carmen_base_velocity_message vel;
  double vl, vr;

  FORMATTER_PTR formatter;
  
  formatter = IPC_msgInstanceFormatter(msgRef);
  err = IPC_unmarshallData(formatter, callData, &vel,
                     sizeof(carmen_base_velocity_message));
  IPC_freeByteArray(callData);

  carmen_test_ipc_return(err, "Could not unmarshall", 
			 IPC_msgInstanceName(msgRef));

  vl = vel.tv * TICKS_PER_MPS;
  vr = vel.tv * TICKS_PER_MPS;  

  vl -= 0.5 * vel.rv * ROT_VEL_FACT_RAD;
  vr += 0.5 * vel.rv * ROT_VEL_FACT_RAD;

  //  printf("Velocities: trans: %f rot: %f\r\n",vel.tv,vel.rv);
  //printf("After conv: left %f right %f\r\n",vl, vr);

  if(vl > MAXV)
    vl = MAXV;
  else if(vl < -MAXV)
    vl = -MAXV;
  if(vr > MAXV)
      vr = MAXV;
  else if(vr < -MAXV)
    vr = -MAXV;

  //printf("After maxing: %f %f\r\n",vl,vr);

  //we put in a minimum ticks per loop due to a bad motor controller
  //note that if we set one to the minimum we should increase
  //the other speed in proportion
  if( vl> 0.0 && vl < MIN_OBOT_TICKS)
    {
      vr = vr/vl * MIN_OBOT_TICKS;
      vl = MIN_OBOT_TICKS;
    }
  if( vl< 0.0 && vl > -MIN_OBOT_TICKS)
    {
      vr = vr/(fabs(vl)) * MIN_OBOT_TICKS;
      vl =-MIN_OBOT_TICKS;
    }

  if( vr> 0.0 && vr < MIN_OBOT_TICKS)
    {
      vl = vl/vr * MIN_OBOT_TICKS;      
      vr = MIN_OBOT_TICKS;
    }

  if( vr< 0.0 && vr > -MIN_OBOT_TICKS)
    {
      vl = vl/(fabs(vr)) * MIN_OBOT_TICKS;      

      vr =-MIN_OBOT_TICKS;

    }
  command_vl = (int)vl;
  command_vr = (int)vr;

  //printf("To_int: %d %d\r\n",command_vl, command_vr);

  set_velocity = 1;
}

/*
static void 
gun_handler(MSG_INSTANCE msgRef, BYTE_ARRAY callData,
		 void *clientData __attribute__ ((unused)))
{
  IPC_RETURN_TYPE err;
  carmen_robot_cereb_fire_message v;

  FORMATTER_PTR formatter;
  
  formatter = IPC_msgInstanceFormatter(msgRef);
  err = IPC_unmarshallData(formatter, callData, &v,
                     sizeof(carmen_robot_cereb_fire_message));
  IPC_freeByteArray(callData);

  carmen_test_ipc_return(err, "Could not unmarshall", 
			 IPC_msgInstanceName(msgRef));

  printf("firing\r\n");

  fire_gun = 1;
}
*/
int 
carmen_cerebellum_initialize_ipc(void)
{
  IPC_RETURN_TYPE err;

  /* define messages created by cerebellum */
  err = IPC_defineMsg(CARMEN_BASE_ODOMETRY_NAME, IPC_VARIABLE_LENGTH,
                      CARMEN_BASE_ODOMETRY_FMT);
  carmen_test_ipc_exit(err, "Could not define", CARMEN_BASE_ODOMETRY_NAME);

  err = IPC_defineMsg(CARMEN_BASE_VELOCITY_NAME, IPC_VARIABLE_LENGTH,
                      CARMEN_BASE_VELOCITY_FMT);
  carmen_test_ipc_exit(err, "Could not define", CARMEN_BASE_VELOCITY_NAME);

  /* setup incoming message handlers */

  err = IPC_subscribe(CARMEN_BASE_VELOCITY_NAME, velocity_handler, NULL);
  carmen_test_ipc_exit(err, "Could not subscribe", CARMEN_BASE_VELOCITY_NAME);
  IPC_setMsgQueueLength(CARMEN_BASE_VELOCITY_NAME, 1);

  //  err = IPC_subscribe(CARMEN_ROBOT_CEREB_FIRE_NAME, gun_handler, NULL);
  //carmen_test_ipc_exit(err, "Could not subscribe", CARMEN_ROBOT_CEREB_FIRE_NAME);
  //IPC_setMsgQueueLength(CARMEN_ROBOT_CEREB_FIRE_NAME, 1);

  return IPC_No_Error;
}

#ifdef _REENTRANT
static void *
start_thread(void *data __attribute__ ((unused))) 
{
  sigset_t set;

  sigemptyset(&set);
  sigaddset(&set, SIGINT);
  sigaddset(&set, SIGPIPE);
  sigprocmask(SIG_BLOCK, &set, NULL);

  while (1) {    
    command_robot();
    usleep(30000);
  }
  return(NULL);
}  
#endif


int 
carmen_cerebellum_start(int argc, char **argv)
{
  if (read_cerebellum_parameters(argc, argv) < 0)
    return -1;

  if (carmen_cerebellum_initialize_ipc() < 0) 
    {
      carmen_warn("\nError: Could not initialize IPC.\n");
      return -1;
    }

  if(initialize_robot(dev_name) < 0)
    {
      carmen_warn("\nError: Could not connect to robot. "
		  "Did you remember to turn the base on?\n");
      return -1;
    }

  reset_status();

  strcpy(odometry.host, carmen_get_tenchar_host_name());

#ifdef _REENTRANT
  pthread_create(&main_thread, NULL, start_thread, NULL);
  thread_is_running = 1;
#endif
  return 0;
}

int 
carmen_cerebellum_run(void) 
{
  IPC_RETURN_TYPE err;

  if (update_status() == 1)
    {
      err = IPC_publishData(CARMEN_BASE_ODOMETRY_NAME, &odometry);
      carmen_test_ipc_exit(err, "Could not publish", 
			   CARMEN_BASE_ODOMETRY_NAME);
    }
#ifndef _REENTRANT
  command_robot();
#endif
  
  return 1;
}

void 
carmen_cerebellum_shutdown(int x)
{
  if(x == SIGINT) 
    {
      carmen_verbose("\nShutting down robot...");
      sleep(1);
      carmen_cerebellum_set_velocity(0, 0);
      carmen_cerebellum_limp();
      last_update = carmen_get_time_ms();
      carmen_cerebellum_disconnect_robot();
      carmen_verbose("done.\n");
    }
}

void 
carmen_cerebellum_emergency_crash(int x __attribute__ ((unused)))
{
  carmen_cerebellum_set_velocity(0, 0);
  last_update = carmen_get_time_ms();
}
