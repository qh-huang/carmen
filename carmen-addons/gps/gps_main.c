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
 * Public License along with Foobar; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place, 
 * Suite 330, Boston, MA  02111-1307 USA
 *
 ********************************************************/

/*********************************************************
This file contains an entry point for GPS executable and other
high level functions
**********************************************************/

#include <carmen/carmen.h>
#include "gps_garmin.h"
#include "gps_estimate.h"
#include "gps_messages.h"

//parameters
static char *parmstrGPSDev;
static double parmgpscoord_originlat = -1;
static double parmgpscoord_originlon = -1;
static int parmintegrate_odometry = 0;
static double parminitialtheta = -1;
static double parminitialthetastd = -1; 
static double parmodomdiststdper1m = -1;
static double parmodomthetastdper1m = -1;
static double parmodomthetastdper1rad = -1;
static double parmgpsXYstdper1precdil = -1;

//messages
static carmen_base_odometry_message odometry;

#define DEBUGFILE 0

#if DEBUGFILE
static FILE *fDeb = NULL;
static FILE *fDebFinal = NULL;
#endif

/*
static void debug_run(double *lat, double *lon, int *satnum, double *precdil)
{
  //static int count = 0;
  double X = 0, Y = 0, Theta = 0;
  double Xvar, Yvar, Thetavar;
  int bValid;
  int datatype;
  static FILE *fin = NULL;

  if(fin == NULL) fin = fopen("data.debug", "r");
  
  *satnum = 0;
  *precdil = 0;
  
  while(!feof(fin))
    {
      fscanf(fin, "%d", &datatype);
      if(datatype == 0)
	{
	 
	  //baseodom
	  fscanf(fin, "%lg %lg %lg\n", &X, &Y, &Theta);
	  if(carmen_gps_estimate_kalman_baseodom_update(X, Y, Theta) < 0)
	    {
	      carmen_perror("\nerror incorporating odometry readings");
	      return;
	    }  
	 
	  carmen_gps_estimate_kalman_getstate(&X, &Y, &Theta, 
					  &Xvar, &Yvar, &Thetavar, &bValid);

#if DEBUGFILE	  
	  fprintf(fDebFinal, "%.2f %.2f %.2f %.2f %.2f %.2f %d\n", X, Y, 
		  Theta, 
		  Xvar, Yvar, Thetavar, bValid);
#endif

	}
      else
	{
	  //gps
	  fscanf(fin, "%lg %lg %d %lg\n", lon, lat, satnum, precdil);
	  break;
	}
    }

}
*/


static void publish_gps_position(double Xcoord, double Ycoord, double Theta, 
				 double Xvar, double Yvar, double Thetavar)
{
  IPC_RETURN_TYPE err;
  char *host;
  static carmen_gps_position_message msg;
  static int first = 1;
  
  if(first) {
    host = carmen_get_tenchar_host_name();
    strcpy(msg.host, host);
    first = 0;
  }

  //copy the contents of the message
  msg.gpscoordX = Xcoord;
  msg.gpscoordY = Ycoord;
  msg.gpscoordTheta = Theta;
  msg.Xvar = Xvar;
  msg.Yvar = Yvar;
  msg.Thetavar = Thetavar;
  msg.timestamp = carmen_get_time_ms();

  err = IPC_publishData(CARMEN_GPS_POSITION_NAME, &msg);
  carmen_test_ipc(err, "Could not publish", CARMEN_GPS_POSITION_NAME);
}


static void base_odometry_handler(void) 
{
  double X,Y,Theta;
  static int bFirst = 1;
  static struct timeval last_time;
  struct timeval current_time;
  int sec, msec;

  
  //get data
  X = odometry.x;
  Y = odometry.y;
  Theta = odometry.theta;

  if(!bFirst)
    {
      gettimeofday(&current_time, NULL);
      msec = current_time.tv_usec - last_time.tv_usec;
      sec = current_time.tv_sec - last_time.tv_sec;
      if (msec < 0) 
	{
	  msec += 1e6;
	  sec--;
	}
      last_time = current_time;
    }
  else
    {
      bFirst = 0;
      gettimeofday(&last_time, NULL);
      sec = 0;
      msec = 0;
    }

  //update the kalman filter 
  if(carmen_gps_estimate_kalman_baseodom_update(X, Y, Theta) < 0)
    {
      carmen_perror("\nerror incorporating odometry readings");
      return;
    }
#if DEBUGFILE
  fprintf(fDeb, "baseodom time passed: %d s %d ms, ", sec, msec);
  fprintf(fDeb, "x=%.2f y=%.2f theta=%f\n", X, Y, Theta);
#endif

}


static int initialize_gps_ipc(void)
{

  IPC_RETURN_TYPE err;

 /* define messages created by this module */
  err = IPC_defineMsg(CARMEN_GPS_POSITION_NAME, IPC_VARIABLE_LENGTH, 
		      CARMEN_GPS_POSITION_FMT);
  carmen_test_ipc_exit(err, "Could not define", CARMEN_GPS_POSITION_NAME);


  /* subscribe to other messages */

  if(parmintegrate_odometry)
    {
      carmen_base_subscribe_odometry_message(&odometry,
		 (carmen_handler_t)base_odometry_handler,
					 CARMEN_SUBSCRIBE_LATEST);
    }

  return 0;

}

static int read_gps_parameters(int argc, char **argv)
{
  int num_items;

  carmen_param_t param_list[] = {
    {"gps", "dev", CARMEN_PARAM_STRING, &parmstrGPSDev, 0, NULL},
    {"gps", "originlat", CARMEN_PARAM_DOUBLE, 
     &parmgpscoord_originlat, 0, NULL},
    {"gps", "originlon", CARMEN_PARAM_DOUBLE, 
     &parmgpscoord_originlon, 0, NULL},
    {"gps", "integrate_with_odometry", CARMEN_PARAM_ONOFF, 
     &parmintegrate_odometry, 0, NULL},
    {"gps", "initialtheta", CARMEN_PARAM_DOUBLE, 
     &parminitialtheta, 0, NULL},
    {"gps", "initialthetastd", CARMEN_PARAM_DOUBLE, 
     &parminitialthetastd, 0, NULL},
    {"gps", "odomdiststdper1m", CARMEN_PARAM_DOUBLE, 
     &parmodomdiststdper1m, 0, NULL},
    {"gps", "odomthetastdper1m", CARMEN_PARAM_DOUBLE, 
     &parmodomthetastdper1m, 0, NULL},
    {"gps", "odomthetastdper1rad", CARMEN_PARAM_DOUBLE, 
     &parmodomthetastdper1rad, 0, NULL},
    {"gps", "gpsxystdper1precdil", CARMEN_PARAM_DOUBLE, 
     &parmgpsXYstdper1precdil, 0, NULL}
  };
    
  num_items = sizeof(param_list)/sizeof(param_list[0]);
  
  carmen_param_install_params(argc, argv, param_list, num_items);
  
  carmen_warn("Using gps_dev:\t\t%s\n", parmstrGPSDev);

  return 0;
}

int start_gps(int argc, char **argv)
{

  if (read_gps_parameters(argc, argv) < 0)
    return -1;

  if(initialize_gps_ipc() < 0) 
    {
      carmen_perror("\nError: Could not initialize IPC.\n");
      return -1;
    }

  
  if(carmen_gps_initialize_garmingps(parmstrGPSDev) < 0) 
    {
      carmen_perror("Could not initialize gps");
      return -1;
    }
  

  //send down the origin of the gps coordinate system as latitude and longitude
  if(carmen_gps_estimate_init(parmgpscoord_originlat, parmgpscoord_originlon) < 0)
      {
	carmen_perror("Could not initialize gps state variables");
	return -1;
      }
 


  //initialize Kalman filter if we integrate with odometry
  if(parmintegrate_odometry)
    {
      if(carmen_gps_estimate_kalman_init(parminitialtheta, 
	    parminitialthetastd, parmodomdiststdper1m, 
	    parmodomthetastdper1m, parmodomthetastdper1rad,
					 parmgpsXYstdper1precdil) < 0)
	{
	 carmen_perror("Could not initialize gps kalman filter variables");
	 return -1; 
	}
    }

  return 0;
}

int carmen_gps_run(void)
{
  double lat, lon, precdil;
  int satnum;
  static int bFirst = 1;
  int bValid = 0;
  double Xcoord = -1, Ycoord = -1, Theta = -1;
  double Xvar = -1, Yvar = -1, Thetavar = -1;
  static struct timeval last_time;
  struct timeval current_time;
  int sec, msec;
  
  if(!bFirst)
    {
      gettimeofday(&current_time, NULL);
      msec = current_time.tv_usec - last_time.tv_usec;
      sec = current_time.tv_sec - last_time.tv_sec;
      if (msec < 0) 
	{
	  msec += 1e6;
	  sec--;
	}
      last_time = current_time;
    }
  else
    {
      gettimeofday(&last_time, NULL);
      sec = 0;
      msec = 0;
    }

#if DEBUGFILE
  fprintf(fDeb, "gps time passed: %d s %d ms, ", sec, msec);
#endif

  //get the gps data
  if(carmen_gps_run_garmingps(&lat, &lon, &satnum, &precdil) < 0)
    {
      carmen_perror("\nerror executing gps cycle");
      return -1;
    }  
  /*debug_run(&lat, &lon, &satnum, &precdil); */

  //get the global gps coordinates
  if(carmen_gps_estimate_convertfromgps2local(lat, lon, satnum, 
					      &Xcoord, &Ycoord) < 0)
    {
      carmen_perror("\nerror converting gps readings into global coordinates");
      return -1;
    }

#if DEBUGFILE
  fprintf(fDeb, "x=%.2f y=%.2f lon=%f lat=%f sat=%d dil=%.2f\n", Xcoord,Ycoord,
	  lon,lat,satnum,precdil);
#endif

  //if integrated with odometry then pass it through a Kalman filter
  if(parmintegrate_odometry)
    {
      //incorporate gps readings
      if(carmen_gps_estimate_kalman_gpsobserv_update(Xcoord, Ycoord, 
					      satnum, precdil) < 0)
	{
	  carmen_perror("\nerror incorporating a gps reading");
	  return -1;
	}      

      //update the coordinates
      carmen_gps_estimate_kalman_getstate(&Xcoord, &Ycoord, &Theta, 
					  &Xvar, &Yvar, &Thetavar, &bValid);
    }
  else
    {
      //the data is valid whenever there are some satellites are visible
      if(satnum > 0 && precdil > 0.005)
	bValid = 1;
      else
	bValid = 0;

      //set the theta
      Theta = 0;
      
      //set the variances
      Xvar = parmgpsXYstdper1precdil*parmgpsXYstdper1precdil*precdil;
      Yvar = parmgpsXYstdper1precdil*parmgpsXYstdper1precdil*precdil;
      Thetavar = 1000000; //theta is invalid
    }

  //print the data on the screen
  if(bFirst)
    {
      carmen_warn("x(lon)*y(lat)*sat*dil:\t%+010.1f(%8.4f)*%+010.1f(%8.4f)*%2d*%4.1f",
		  Xcoord,lon,Ycoord,lat,satnum,precdil);
    }
  else
  {
    carmen_warn("\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b%+010.1f(%8.4f)*%+010.1f(%8.4f)*%2d*%4.1f",
		Xcoord,lon,Ycoord,lat,satnum,precdil);
  }

#if DEBUGFILE
  fprintf(fDebFinal, "%.2f %.2f %.2f %.2f %.2f %.2f %d\n", Xcoord, Ycoord, 
	  Theta, Xvar, Yvar, Thetavar, bValid);
#endif
  
  if(bValid == 1)
    {
      publish_gps_position(Xcoord, Ycoord, Theta, Xvar, Yvar, Thetavar);
    }


  bFirst = 0;

  return 0;
}

void shutdown_gps(int signo __attribute__ ((unused)))
{
  carmen_gps_shutdown_garmingps();
  close_ipc();
  exit(-1);
}


int main(int argc, char **argv)
{
#if DEBUGFILE
  fDeb = fopen("gps.debug", "w");
  fDebFinal = fopen("gps_final.debug", "w");
#endif
  
  carmen_initialize_ipc(argv[0]);
  carmen_param_check_version(argv[0]);

  if (start_gps(argc, argv) < 0)
    exit(-1);

  signal(SIGINT, shutdown_gps);

  while(1) {
    sleep_ipc(0.2);    
    if(carmen_gps_run() == -1)
      shutdown_gps(0);
  }
  return 0;
}

