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

#define DEBUGFILE 1
#define LOGFILE   1

#if DEBUGFILE
static FILE *fDeb = NULL;
#endif
#if LOGFILE
static FILE *fLog = NULL;
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

#if LOGFILE 
	  fprintf(fLog, "%.2f %.2f %.2f %.2f %.2f %.2f %d\n", X, Y, 
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


static void publish_gps_position(double Xcoord, double Ycoord, int satnum,
				 double precdil)  
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
  msg.precdil = precdil;
  msg.satnum = satnum;
  msg.timestamp = carmen_get_time_ms();

  err = IPC_publishData(CARMEN_GPS_POSITION_NAME, &msg);
  carmen_test_ipc(err, "Could not publish", CARMEN_GPS_POSITION_NAME);
}


static int initialize_gps_ipc(void)
{

  IPC_RETURN_TYPE err;

 /* define messages created by this module */
  err = IPC_defineMsg(CARMEN_GPS_POSITION_NAME, IPC_VARIABLE_LENGTH, 
		      CARMEN_GPS_POSITION_FMT);
  carmen_test_ipc_exit(err, "Could not define", CARMEN_GPS_POSITION_NAME);


  /* subscribe to other messages */


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

  //initialize gps
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
 
  return 0;
}

int carmen_gps_run(void)
{
  double lat, lon, precdil;
  int satnum;
  static int bFirst = 1;
  double Xcoord = -1, Ycoord = -1;
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
  fprintf(fDeb, "time=%f gps time passed: %d s %d ms, ", 
	  carmen_get_time_ms(), sec, msec);
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

#if LOGFILE
  fprintf(fLog, "%f %.2f %.2f %f %f %d %f\n", 
	  carmen_get_time_ms(), Xcoord, Ycoord, lon, lat, satnum, precdil);
#endif

  //print the data on the screen
  if(bFirst)
    {
      carmen_warn("x*y*sat*dil:\t%+010.1f*%+010.1f*%2d*%4.1f",
		  Xcoord,Ycoord,satnum,precdil);
    }
  else
  {
    carmen_warn("\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b%+010.1f*%+010.1f*%2d*%4.1f",
		Xcoord,Ycoord,satnum,precdil);
  }
  
  if(satnum > 0)
    {
      publish_gps_position(Xcoord, Ycoord, satnum, precdil);
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
  if(fDeb == NULL)
    {
      carmen_warn("ERROR: can not open debug file\n");
      exit(1);
    }
#endif

#if LOGFILE
  fLog = fopen("gps.log", "w");
  if(fLog == NULL)
    {
      carmen_warn("ERROR: can not open log file\n");
      exit(1);
    }
  fprintf(fLog, "timestamp x y lon lat satnum precdil\n");
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

