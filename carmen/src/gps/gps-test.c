#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <math.h>
#include <values.h>
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>
#include <sys/signal.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/ioctl.h>

#include <carmen/carmen.h>
#include <carmen/gps_nmea_interface.h>

int                          gps_update = FALSE;
carmen_gps_gpgga_message     gps_gpgga;

void
ipc_gps_gpgga_handler( carmen_gps_gpgga_message *data __attribute__ ((unused)))
{
  gps_update++;
}

void
ipc_update( void )
{
  IPC_listen(0);
}

void
ipc_init( char * modulename )
{

  carmen_initialize_ipc( modulename );
  /*************************** SUBSCRIBE ***********************/
  carmen_gps_subscribe_nmea_message( &gps_gpgga,
				     (carmen_handler_t) ipc_gps_gpgga_handler,
				     CARMEN_SUBSCRIBE_ALL );
}

int
main( int argc __attribute__ ((unused)), char *argv[] )
{
  ipc_init(argv[0] );

  while(1) {

    ipc_update();

    if (gps_update) {
      fprintf( stderr, "===================================\n" );
      fprintf( stderr, "        gps gpgga message\n" );
      fprintf( stderr, "===================================\n" );
      fprintf( stderr, " utc:            %f\n", gps_gpgga.utc );
      fprintf( stderr, " latitude:       %f\n", gps_gpgga.latitude );
      fprintf( stderr, " lat_orient:     %c\n", gps_gpgga.lat_orient );
      fprintf( stderr, " longitude:      %f\n", gps_gpgga.longitude );
      fprintf( stderr, " long_orient:    %c\n", gps_gpgga.long_orient ); 
      fprintf( stderr, " gps_quality:    %d\n", gps_gpgga.gps_quality );
      fprintf( stderr, " num_satellites: %d\n", gps_gpgga.num_satellites );
      fprintf( stderr, " hdop:           %f\n", gps_gpgga.hdop );  
      fprintf( stderr, " sea_level:      %f\n", gps_gpgga.sea_level ); 
      fprintf( stderr, " altitude:       %f\n", gps_gpgga.altitude );  
      fprintf( stderr, " geo_sea_level:  %f\n", gps_gpgga.geo_sea_level );
      fprintf( stderr, " geo_sep:        %f\n", gps_gpgga.geo_sep ); 
      fprintf( stderr, " data_age:       %d\n", gps_gpgga.data_age );
      fprintf( stderr, "===================================\n" );
      fprintf( stderr, "\n" );
      gps_update = FALSE;
    }

    usleep(10000);

  }
    
  return(0);
  
}

