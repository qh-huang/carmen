#include <errno.h>
#include <carmen/carmen.h>

#include "rflex_brake.h"

void
usage( char *prgname )
{
  fprintf( stderr,
	   "Usage: %s <brake>\n  0: brake off, 1: brake on\n",
	   prgname );
}
     
int
main( int argc, char *argv[] )
{
  IPC_RETURN_TYPE err;
  carmen_rflex_brake_message data;

  if (argc!=2) {
    usage(argv[0]);
    exit(1);
  }

  carmen_ipc_initialize(argc, argv);
  
  data.set_brake  = atoi(argv[1]);

  err = IPC_publishData (CARMEN_RFLEX_BRAKE_NAME, &data );
  carmen_test_ipc(err, "Could not publish", CARMEN_RFLEX_BRAKE_NAME);

  exit(0);
}
