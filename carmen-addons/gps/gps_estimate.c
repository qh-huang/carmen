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
This file contains a position estimate related functions
including Kalman filter integration with robot odometry
**********************************************************/

#include <carmen/carmen.h>
#include "gps_estimate.h"

#define GPS_SETTLE_DOWN_TIME 15

static gps_estimate_state_t gps_estimate_state;
static gps_kalman_state_t gps_kalman_state; 

/**************start of matrix operations******************/
//computes Res = A1*A2
static void Mpy(double A1[3][3], double A2[3][3], double Res[3][3])
{
  int i,j;
  double Temp[3][3];

  for(i = 0; i < 3; i++)
    { 
      for(j = 0; j < 3; j++)
	{
	  Temp[i][j] = A1[i][0]*A2[0][j] + A1[i][1]*A2[1][j] + 
	    A1[i][2]*A2[2][j];
	}
    }
  
  for(i = 0; i < 3; i++)
    { 
      for(j = 0; j < 3; j++)
	{
	  Res[i][j] = Temp[i][j];
	}
    }
}

//computes Res = transpose(A1)
static void Transpose(double A1[3][3], double Res[3][3])
{
  int i,j;
  double Temp[3][3];

  for(i = 0; i < 3; i++)
    { 
      for(j = 0; j < 3; j++)
	{
	  Temp[i][j] = A1[j][i];
	}
    }

  for(i = 0; i < 3; i++)
    { 
      for(j = 0; j < 3; j++)
	{
	  Res[i][j] = Temp[i][j];
	}
    }
}


//computes Res = A1*V
static void MpybyV(double A1[3][3], double V[3], double Res[3])
{
  int i;
  double Temp[3];

  for(i = 0; i < 3; i++)
    { 
      Temp[i] = A1[i][0]*V[0] + A1[i][1]*V[1] + 
	A1[i][2]*V[2];
    }
  
  for(i = 0; i < 3; i++)
    { 
      Res[i] = Temp[i];
    }
  

}

//computes Res = A1 + A2
static void Add(double A1[3][3], double A2[3][3], double Res[3][3])
{
  int i,j;
  double Temp[3][3];

  for(i = 0; i < 3; i++)
    { 
      for(j = 0; j < 3; j++)
	{
	  Temp[i][j] = A1[i][j] + A2[i][j];
	}
    }

  for(i = 0; i < 3; i++)
    { 
      for(j = 0; j < 3; j++)
	{
	  Res[i][j] = Temp[i][j];
	}
    }

}

//computes Res = A1 - A2
static void Sub(double A1[3][3], double A2[3][3], double Res[3][3])
{
  int i,j;
  double Temp[3][3];

  for(i = 0; i < 3; i++)
    { 
      for(j = 0; j < 3; j++)
	{
	  Temp[i][j] = A1[i][j] - A2[i][j];
	}
    }

  for(i = 0; i < 3; i++)
    { 
      for(j = 0; j < 3; j++)
	{
	  Res[i][j] = Temp[i][j];
	}
    }
}


static void invert(double a[3][3])
{
  int i,j,k;
  double q;
  double eps;
  int n = 3;

  eps = 10e-16;
  
  
  for (i= 0; i < n; i++)
    {
      if (fabs(a[i][i] <= eps)) 
	return;
      q = 1.0 / a[i][i];
      a[i][i] = 1.0;
      for (k = 0; k < n; k++)
	a[i][k] *= q;
      for (j = 0; j < n; j++)
	if (i != j)
	  {
	    q = a[j][i];
	    a[j][i] = 0.0;
	    for (k = 0; k < n; k++)
	      a[j][k] -= q * a[i][k];
	  }
    }
  return;
}


/**************end of matrix operations******************/

//computes Kalman gain as: K = Ek*(Ek+Eg)^(-1)
static int computeKalmanGain(double Ek[3][3], double Eg[3][3], double K[3][3])
{
  double A[3][3];  

  Add(Ek, Eg, A);
  invert(A);
  Mpy(Ek,A,K);

  return 0;
}


//initializes Kalman state variables
int carmen_gps_estimate_kalman_init(double initialtheta, 
				    double initialthetastd,
				    double odomdiststdper1m, 
				    double odomthetastdper1m,
				    double odomthetastdper1rad,
				    double gpsXYstdper1precdil)
{
  
  //Kalman filter parameters
  gps_kalman_state.odomDistvarper1m = odomdiststdper1m*odomdiststdper1m;
  gps_kalman_state.odomThetavarper1m = odomthetastdper1m*odomthetastdper1m;
  gps_kalman_state.odomThetavarper1rad = odomthetastdper1rad*
    odomthetastdper1rad;
  gps_kalman_state.gpsXYvarper1precdil = gpsXYstdper1precdil*
    gpsXYstdper1precdil;

  //initialize current values
  gps_kalman_state.currentCov[0][0] = 0;
  gps_kalman_state.currentCov[0][1] = 0;
  gps_kalman_state.currentCov[0][2] = 0;
  gps_kalman_state.currentCov[1][0] = 0;
  gps_kalman_state.currentCov[1][1] = 0;
  gps_kalman_state.currentCov[1][2] = 0;
  gps_kalman_state.currentCov[2][0] = 0;
  gps_kalman_state.currentCov[2][1] = 0;
  gps_kalman_state.currentCov[2][2] = initialthetastd*initialthetastd;
  gps_kalman_state.currentPose[0] = 0;
  gps_kalman_state.currentPose[1] = 0;
  gps_kalman_state.currentPose[2] = initialtheta;

  //initialize other variables
  gps_kalman_state.gps_first_cnt = 0;
  gps_kalman_state.odom_no_message_yet = 1;
  gps_kalman_state.xgps2loc = 0;
  gps_kalman_state.ygps2loc = 0;
  gps_kalman_state.lastodom_x = -1;
  gps_kalman_state.lastodom_y = -1;
  gps_kalman_state.lastodom_theta = -1;
  
  return 0;
}


//converts from gps coordinates into the gps global coordinate system
//Y is north positive, x is east positive
int carmen_gps_estimate_convertfromgps2local(double gpslat, double gpslong, 
					     int satnum,
					     double *gpsX, double *gpsY)
{
  double EarthR = 6373002.24;
  double X, Y;
  double gpsreflat;

  if(gps_estimate_state.gps_reflatset == 0 && satnum > 0)
    {
      gps_estimate_state.gps_reflat = gpslat;
      gps_estimate_state.gps_reflatset = 1;
    }
  gpsreflat = gps_estimate_state.gps_reflat;

  Y = EarthR*gpslat*M_PI/180.0;
  X = EarthR*cos(gpsreflat*M_PI/180.0)*(-gpslong)*M_PI/180.0;

  *gpsX = X - gps_estimate_state.gps_originX;
  *gpsY = Y - gps_estimate_state.gps_originY;

  return 0;
}

//gps state initialization
int carmen_gps_estimate_init(double gps_originlat, double gps_origin_lon)
{

  gps_estimate_state.gps_originX = 0;
  gps_estimate_state.gps_originY = 0;
  gps_estimate_state.gps_reflat = gps_originlat;
  gps_estimate_state.gps_reflatset = 0;

  if(carmen_gps_estimate_convertfromgps2local(gps_originlat, gps_origin_lon, 
					      0,
					   &gps_estimate_state.gps_originX, 
				       &gps_estimate_state.gps_originY) < 0)
    return -1;


  return 0;

}



//incorporates a new observation from gps
int carmen_gps_estimate_kalman_gpsobserv_update(double Xgpscoord, 
						double Ygpscoord, 
					 int SatNum, double precdil)
{
  double A[3][3];
  double K[3][3], V[3];

  //see if the data exists and more or less valid
  if(SatNum == 0 || precdil < 0.005)
    {
      return 0;
    }

  
  //let the gps settle down
  if(gps_kalman_state.gps_first_cnt < GPS_SETTLE_DOWN_TIME)
    {
      gps_kalman_state.gps_first_cnt++;
      return 0;
    }

  //see if the first odometry message has not been received
  if(gps_kalman_state.odom_no_message_yet == 1)
    return 0;

  //if this is the first time gps data is incorporated then we need to init
  //local coordinates
  if(gps_kalman_state.gps_first_cnt == GPS_SETTLE_DOWN_TIME)
    {
      gps_kalman_state.xgps2loc = gps_kalman_state.currentPose[0] - Xgpscoord;
      gps_kalman_state.ygps2loc = gps_kalman_state.currentPose[1] - Ygpscoord;
      gps_kalman_state.gps_first_cnt++;
    }

  //---finally do the actual incorporation of the observation-----
  //construct gps mean vector
  V[0] = Xgpscoord + gps_kalman_state.xgps2loc;
  V[1] = Ygpscoord + gps_kalman_state.ygps2loc;
  V[2] = 0;

  //construct gps covariance matrix
  A[0][0] = gps_kalman_state.gpsXYvarper1precdil*precdil;
  A[0][1] = 0;
  A[0][2] = 0;
  A[1][0] = 0;
  A[1][1] = gps_kalman_state.gpsXYvarper1precdil*precdil;
  A[1][2] = 0;
  A[2][0] = 0;
  A[2][1] = 0;
  A[2][2] = 1000000;  

  //compute gain
  computeKalmanGain(gps_kalman_state.currentCov, A, K);
  
  //update means
  V[0] -= gps_kalman_state.currentPose[0];
  V[1] -= gps_kalman_state.currentPose[1];
  V[2] -= gps_kalman_state.currentPose[2];
  V[2] = carmen_normalize_theta(V[2]);
  MpybyV(K, V, V);
  gps_kalman_state.currentPose[0] += V[0];
  gps_kalman_state.currentPose[1] += V[1];
  gps_kalman_state.currentPose[2] += V[2];
  gps_kalman_state.currentPose[2] = 
    carmen_normalize_theta(gps_kalman_state.currentPose[2]);

  //update covariances
  Mpy(K, gps_kalman_state.currentCov, A);
  Sub(gps_kalman_state.currentCov, A, gps_kalman_state.currentCov);
  
  return 0;
}

//returns some of the state variables
int carmen_gps_estimate_kalman_getstate(double *Xcoord, double *Ycoord, 
					double *Theta, 
					double *Xvar, double *Yvar, 
					double *Thetavar, int *bValid)
{
  *Xcoord = gps_kalman_state.currentPose[0];
  *Ycoord = gps_kalman_state.currentPose[1];
  *Theta = gps_kalman_state.currentPose[2];
  *Xvar = gps_kalman_state.currentCov[0][0];
  *Yvar = gps_kalman_state.currentCov[1][1];
  *Thetavar = gps_kalman_state.currentCov[2][2];

  //state was not initialized yet
  if(gps_kalman_state.odom_no_message_yet == 1)
    {
      *bValid = 0;
    }
  else
    {
      *bValid = 1;
    }

  return 0;
}


//incorporate new information from the base odometry
int carmen_gps_estimate_kalman_baseodom_update(
			double odom_x, double odom_y, double odom_theta)
{
  double dist, cosu, sinu, dQ1, dQ2, dX, dY;
  double A[3][3], W[3][3], T[3][3], noiseCov[3][3];
  double zero_eps = 0.0005;


  //see if this is the first message
  if(gps_kalman_state.odom_no_message_yet == 1)
    {
      //then just store the initial values
      gps_kalman_state.lastodom_x = odom_x;
      gps_kalman_state.lastodom_y = odom_y;
      gps_kalman_state.lastodom_theta = odom_theta;
      
      gps_kalman_state.odom_no_message_yet = 0;
      return 0;      
    }

  //compute traveled distance
  dX = odom_x - gps_kalman_state.lastodom_x;
  dY = odom_y - gps_kalman_state.lastodom_y;
  dist = hypot(dX, dY);

  //compute angle difference
  if(fabs(dist) <=  zero_eps)
    {
      //no distance is traveled
      dQ1 = odom_theta - gps_kalman_state.lastodom_theta;
      dQ1 = carmen_normalize_theta(dQ1);
      dQ2 = 0;
    }
  else
    {
      dQ1 = atan2(dY, dX) - gps_kalman_state.lastodom_theta;
      dQ1 = carmen_normalize_theta(dQ1);
      dQ2 = odom_theta - atan2(dY, dX);
      dQ2 = carmen_normalize_theta(dQ2);
    }
  
  //--------first step of prediction---------
  //update mean and variances
  gps_kalman_state.currentPose[2] = 
    carmen_normalize_theta(gps_kalman_state.currentPose[2] + dQ1);
  gps_kalman_state.currentCov[2][2] = gps_kalman_state.currentCov[2][2] +
    gps_kalman_state.odomThetavarper1rad*dQ1*dQ1;


  //----------second step of prediction----------
  //cosu, cosv
  cosu = cos(gps_kalman_state.currentPose[2]);
  sinu = sin(gps_kalman_state.currentPose[2]);

  //compute dX, dY
  dX = dist*cosu;
  dY = dist*sinu;

  //update means
  gps_kalman_state.currentPose[0] =  gps_kalman_state.currentPose[0] + dX;
  gps_kalman_state.currentPose[1] =  gps_kalman_state.currentPose[1] + dY;

  //construct jacobians
  A[0][0] = 1;
  A[0][1] = 0;
  A[0][2] = -dY;
  A[1][0] = 0;
  A[1][1] = 1;
  A[1][2] = dX;
  A[2][0] = 0;
  A[2][1] = 0;
  A[2][2] = 1;  
  W[0][0] = cosu;
  W[0][1] = -dY;
  W[0][2] = 0;
  W[1][0] = sinu;
  W[1][1] = dX;
  W[1][2] = 0;
  W[2][0] = 0;
  W[2][1] = 1;
  W[2][2] = 0;  

  //construct noise covariance matrix
  noiseCov[0][0] = dist*dist*gps_kalman_state.odomDistvarper1m;
  noiseCov[0][1] = 0;
  noiseCov[0][2] = 0;
  noiseCov[1][0] = 0;
  noiseCov[1][1] = dist*dist*gps_kalman_state.odomThetavarper1m;
  noiseCov[1][2] = 0;
  noiseCov[2][0] = 0;
  noiseCov[2][1] = 0;
  noiseCov[2][2] = 0;  

  //update variances
  //currentCov = A*currentCov*A' + W*noiseCov*W'
  Mpy(A, gps_kalman_state.currentCov, gps_kalman_state.currentCov);
  Transpose(A, T);
  Mpy(gps_kalman_state.currentCov, T, gps_kalman_state.currentCov);
  Mpy(W, noiseCov, noiseCov);
  Transpose(W, T);
  Mpy(noiseCov, T, T);
  Add(gps_kalman_state.currentCov, T, gps_kalman_state.currentCov);

  //---------third step of prediction-------------
  //update mean and variances
  gps_kalman_state.currentPose[2] = 
    carmen_normalize_theta(gps_kalman_state.currentPose[2] + dQ2);
  gps_kalman_state.currentCov[2][2] = gps_kalman_state.currentCov[2][2] +
    gps_kalman_state.odomThetavarper1rad*dQ2*dQ2;


  //remember the old values of odometry
  gps_kalman_state.lastodom_x = odom_x;
  gps_kalman_state.lastodom_y = odom_y;
  gps_kalman_state.lastodom_theta = odom_theta;

  return 0;
}





