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

#ifndef GPS_ESTIMATE_H
#define GPS_ESTIMATE_H


struct GPS_ESTIMATE_STATE
{
  double gps_originX;
  double gps_originY;
  double gps_reflat;
  int    gps_reflatset;
};
typedef struct GPS_ESTIMATE_STATE gps_estimate_state_t;


#ifdef __cplusplus
extern "C" {
#endif

int carmen_gps_estimate_convertfromgps2local(double gpslat, double gpslong, 
					     int satnum,
					     double *gpsX, double *gpsY);
int carmen_gps_estimate_init(double gps_originlat, double gps_originlon);

#ifdef __cplusplus
}
#endif


#endif
