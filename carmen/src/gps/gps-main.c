#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <math.h>
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>
#include <sys/signal.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/ioctl.h>

#include <carmen/carmen.h>
#include <carmen/gps_nmea_interface.h>

#include "gps.h"
#include "gps-ipc.h"
#include "gps-io.h"

void
read_parameters(SerialDevice *dev, int argc, char **argv)
{
  carmen_param_t gps_dev[] = {
    {"gps", "nmea_dev", CARMEN_PARAM_STRING, &(dev->ttyport), 0, NULL},
    {"gps", "nmea_baud", CARMEN_PARAM_INT, &(dev->baud), 0, NULL}};
  
  carmen_param_install_params(argc, argv, gps_dev, 
			      sizeof(carmen_param_t) / sizeof(gps_dev[0]));
  
}

void
print_usage( void )
{
  fprintf( stderr, "gps-nmea [-nr NR] [-dev DEVICE]\n");
}

/**************************************************************************
 * MAIN-LOOP
 **************************************************************************/

int
main(int argc, char *argv[])
{
  int                         gps_nr = 0;
  SerialDevice                dev;
  carmen_gps_gpgga_message    gpgga;

  gpgga.host = carmen_get_host();

  //gethostname( gpgga.host, 10 );
  carmen_ipc_initialize( argc, argv );
  ipc_initialize_messages();
 
  read_parameters( &dev, argc, argv );

  carmen_extern_gpgga_ptr = &gpgga;
  carmen_extern_gpgga_ptr->nr = gps_nr;
  
  DEVICE_init_params( &dev );

  fprintf( stderr, "INFO: ************************\n" );
  fprintf( stderr, "INFO: ********* GPS   ********\n" );
  fprintf( stderr, "INFO: ************************\n" );

  fprintf( stderr, "INFO: open device (%s) ", dev.ttyport );
  if (DEVICE_connect_port( &dev )<0) {
    fprintf( stderr, "ERROR: can't open device !!!\n\n" );
    exit(1);
  } else {
    fprintf( stderr, "INFO: done\n" );
  }

  while(TRUE) {

    if ( DEVICE_bytes_waiting( dev.fd )>10 ) {
      if (DEVICE_read_data( dev )) {
	gpgga.timestamp = carmen_get_time();
	ipc_publish_position();
      }
      usleep(100000);
    } else {
      carmen_ipc_sleep(0.25);
      //IPC_listen(0);
      //usleep(250000);
    }

  }

  return(0);
}
