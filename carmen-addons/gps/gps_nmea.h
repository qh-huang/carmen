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

#ifndef GPS_NMEA_H
#define GPS_NMEA_H

//receive messages
#define GPSNMEARecPosn      "$GPGGA"

//the size of the GPS buffer
#define GPS_NMEA_BUFFERSIZE 2048

//the maximum time of a GPS check in secs
#define GPS_CHECK_TIMEOUT 10

#ifdef __cplusplus
extern "C" {
#endif


//function prototypes
int carmen_gps_nmeacheck(int ttyfp);
int carmen_gps_run_nmea(int ttyfp, double* plat, double *plong, int *psatnum, 
			     double *precdil);

#ifdef __cplusplus
}
#endif


#endif
