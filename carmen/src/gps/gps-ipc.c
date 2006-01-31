#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <signal.h>
#include <math.h>
#include <stdarg.h>
#include <sys/time.h>

#include <carmen/carmen.h>
#include <carmen/gps_nmea_interface.h>


#include "gps.h"

carmen_gps_gpgga_message       * carmen_extern_gpgga_ptr =  NULL;
carmen_gps_gprmc_message       * carmen_extern_gprmc_ptr =  NULL;

void
ipc_publish_position( void )
{
  IPC_RETURN_TYPE err = IPC_OK;
  if (carmen_extern_gpgga_ptr!=NULL) {
    err = IPC_publishData (CARMEN_GPS_GPGGA_MESSAGE_NAME, 
			   carmen_extern_gpgga_ptr );
    carmen_test_ipc(err, "Could not publish", CARMEN_GPS_GPGGA_MESSAGE_NAME);
    fprintf( stderr, "(gga)" );
  }
  if (carmen_extern_gprmc_ptr!=NULL) {
    err = IPC_publishData (CARMEN_GPS_GPRMC_MESSAGE_NAME, 
			   carmen_extern_gprmc_ptr );
    carmen_test_ipc(err, "Could not publish", CARMEN_GPS_GPRMC_MESSAGE_NAME);
    fprintf( stderr, "(rmc)" );
  }
}

void
ipc_initialize_messages( void )
{
  IPC_RETURN_TYPE err;

  err = IPC_defineMsg(CARMEN_GPS_GPGGA_MESSAGE_NAME, IPC_VARIABLE_LENGTH, 
		      CARMEN_GPS_GPGGA_MESSAGE_FMT);
  carmen_test_ipc_exit(err, "Could not define", CARMEN_GPS_GPGGA_MESSAGE_NAME);

  err = IPC_defineMsg(CARMEN_GPS_GPRMC_MESSAGE_NAME, IPC_VARIABLE_LENGTH, 
		      CARMEN_GPS_GPRMC_MESSAGE_FMT);
  carmen_test_ipc_exit(err, "Could not define", CARMEN_GPS_GPRMC_MESSAGE_NAME);

}
