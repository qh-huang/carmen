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
This file contains a code specific to Garmin GPS configuration
**********************************************************/

#include <carmen/carmen.h>
#include "gps_nmea.h"

static int ttyfp;

//initializes Garmin GPS. Returns -1 if failed, and 0 otherwise
int carmen_gps_initialize_garmingps(char* strGPSDev)
{  
  struct termios termios;
 
  carmen_warn("Initializing GPS:\t");
 
  //open the port
  ttyfp = open(strGPSDev, O_RDWR|O_NOCTTY|O_NONBLOCK|O_CREAT);
  if (ttyfp == -1){
    carmen_warn("FAILED\n");
    carmen_warn("Could not open the control port %s\n", strGPSDev);
    return -1;
  }

  //configure the port
  if (isatty(ttyfp))
    {
      if (tcgetattr(ttyfp, &termios) == -1){
	carmen_warn("FAILED\n");
	carmen_warn("Could not obtain the attributes of %s\n", strGPSDev);
	close(ttyfp);
	return -1;
      }

      //set the attributes in the structure
      cfmakeraw(&termios);
      if(cfsetospeed(&termios, B19200) == -1){
	carmen_warn("FAILED\n");
	carmen_warn("Could not set the output speed of %s\n", strGPSDev);
	close(ttyfp);
        return -1;
      }

      if(cfsetispeed(&termios, B19200) == -1){
	carmen_warn("FAILED\n");
	carmen_warn("Could not set the output speed of %s\n", strGPSDev);
	close(ttyfp);
        return -1;
      }

      //set the attributes on the actual port
      tcflush(ttyfp, TCIFLUSH);
      if (tcsetattr(ttyfp, TCSANOW, &termios) == -1){
	carmen_warn("FAILED\n");
	carmen_warn("Could not set the attributes of %s\n", strGPSDev);
	close(ttyfp);
	return -1;
      }
    }
  else
    {
      carmen_warn("FAILED\n");
      carmen_warn("Invalid port %s\n", strGPSDev);
      close(ttyfp);
      return -1;
    }

  carmen_warn("done\n");

  carmen_warn("Checking the protocol:\t");

  //test that the GPS corresponds to NMEA format
  if(carmen_gps_nmeacheck(ttyfp) == -1)
    {
      carmen_warn("FAILED\n");
      carmen_warn("The GPS protocol does not seem to be NMEA\n");
      close(ttyfp);
      return -1;
    }

  carmen_warn("done\n");
  return 0;
}


//shuts down the GPS
int carmen_gps_shutdown_garmingps()
{
  carmen_warn("\nShutting down GPS:\t");

  if(close(ttyfp) == -1)
    {
      carmen_warn("FAILED\n");
      return -1;
    }

  carmen_warn("done\n");
  return 0;
}

//runs a new cycle of processing GPS that is configured in NMEA mode
//returns 0 if successful, -1 otherwise
int carmen_gps_run_garmingps(double* plat, double *plong, int *psatnum, 
			double *precdil)
{
  return carmen_gps_run_nmea(ttyfp, plat, plong, psatnum, precdil);
}
