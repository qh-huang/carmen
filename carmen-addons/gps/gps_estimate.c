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


static gps_estimate_state_t gps_estimate_state;


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





