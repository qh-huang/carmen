#include <carmen/carmen.h>
#include "pantilt.h"
#include "pantilt_messages.h"

PantiltDeviceType    pDevice;
PantiltSettingsType  pSettings;
PantiltLimitsType    pLimits;
PantiltPosType       pPos;

void 
commShutdown( void ) {
  carmen_ipc_disconnect();
  fprintf( stderr, "\n" );
  exit(0);
}

int 
main( int argc, char *argv[] ) {

  unsigned char  buffer[BUFFER_SIZE];
  int            i=0, val = 0;
  int            do_reset      = FALSE;
  int            do_reset_pan  = FALSE;
  int            do_reset_tilt = FALSE;

  pSettings.doff_tilt = 0;
  pSettings.doff_pan  = 0;

  for (i=1; i<argc; i++) {
    if ((strcmp(argv[i],"-reset")==0)){
      do_reset = TRUE;
    } else if ((strcmp(argv[i],"-reset-pan")==0)){
      do_reset_pan = TRUE;
    } else if ((strcmp(argv[i],"-reset-tilt")==0)){
      do_reset_tilt = TRUE;
    } else {
      fprintf(stderr, "Usage: %s [-reset] [-reset-pan] [-reset-tilt]\n",
	      argv[0]);
      exit(0);
    }
  }

  /* connect to IPC server */
  carmen_ipc_initialize(argc, argv);
  //  carmen_initialize_ipc(argv[0]);
  carmen_param_check_version(argv[0]);
  ipc_initialize_messages();

  PantiltInit( argc, argv );
  
  if (do_reset_pan && do_reset_tilt)
    do_reset = TRUE;
  if (do_reset) {
    writeCMD( pDevice, "R", buffer );
    sleep(20);
  } else if (do_reset_pan) {
    writeCMD( pDevice, "RP", buffer );
    sleep(20);
  } else if (do_reset_tilt) {
    writeCMD( pDevice, "RT", buffer );
    sleep(20);
  }

  fprintf( stderr, "PT2: initialization done\n" );
  
  while(TRUE) {
    if ((val=numChars(pDevice.fd))) {
      read(pDevice.fd,&buffer, 512);
      ProcessLine(buffer, val );
    }
    carmen_ipc_sleep(20);
      //IPC_listen(0);
      //    usleep(20000);
  }
  
}

