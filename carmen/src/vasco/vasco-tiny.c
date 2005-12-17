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
#include <time.h>
#include <sys/ioctl.h>

#include <carmen/carmen.h>

#include "vascocore.h"

#define MAX_NUM_LASER_VALUES              401
#define MAX_NAME_LENGTH                   256
#define MAX_LINE_LENGTH                 65536

void
print_usage( void )
{
  fprintf( stderr, "usage: vasco-tiny <LOG-FILE>\n" );
}

double
rad2deg(double x)
{
  return x * 57.29577951308232087679;
}


int
main( int argc, char *argv[] )
{ 
  carmen_laser_laser_message      scan;
  carmen_point_t                  pos, corrpos, odo;
  char                            line[MAX_LINE_LENGTH];
  char                            command[MAX_NAME_LENGTH];
  char                            dummy[MAX_NAME_LENGTH];
  char                            host[MAX_NAME_LENGTH];
  char                          * running;
  double                          time;
  int                             i;
  FILE                          * fp;
  carmen_move_t                   pos_move = {0.0, 0.0, 0.0};
  carmen_move_t                   corr_move = {0.0, 0.0, 0.0};
  carmen_point_t                  old_pos = {0.0, 0.0, 0.0};
  carmen_point_t                  old_corrpos = {0.0, 0.0, 0.0};
  
  if (argc!=2) {
    print_usage();
    exit(1);
  }
  
  scan.range = (float *) malloc( MAX_NUM_LASER_VALUES * sizeof(float) );
  carmen_test_alloc(scan.range);
  
  scan.remission = (float *) malloc( MAX_NUM_LASER_VALUES * sizeof(float) );
  carmen_test_alloc(scan.remission);

  carmen_ipc_initialize(argc, argv);

  /*****************************************************************/
  vascocore_init( argc, argv );
  /*****************************************************************/

  if ((fp=fopen(argv[1],"r"))==0) {
    fprintf( stderr, "Can't open input file %s !\n", argv[1] );
    exit(0);
  }

  while (fgets(line,MAX_LINE_LENGTH,fp) != NULL) {

    if ( (sscanf(line,"%s",command)!=0) &&
	 (!strcmp( command, "FLASER") ) ) {
      sscanf(line, "%s %d",
	     dummy, &scan.num_readings );
      running = line;
      strtok( running, " ");
      strtok( NULL, " ");
      for (i=0; i<scan.num_readings; i++) {
	scan.range[i] = (float) (atof( strtok( NULL, " ") ));
      }
      pos.x = atof( strtok( NULL, " ") );
      pos.y = atof( strtok( NULL, " ") );
      pos.theta = atof( strtok( NULL, " ") );
      odo.x = atof( strtok( NULL, " ") );
      odo.y = atof( strtok( NULL, " ") );
      odo.theta = atof( strtok( NULL, " ") );
      scan.timestamp = atof( strtok( NULL, " ") );
      strncpy( host, strtok( NULL, " "), MAX_NAME_LENGTH );
      time = atof( strtok( NULL, " ") );
      /*****************************************************************/
      corrpos = vascocore_scan_match( scan, pos );
      /*****************************************************************/
      printf( "FLASER %d", scan.num_readings );
      for (i=0; i<scan.num_readings; i++) {
	printf( " %.2f", scan.range[i] );
      }
      printf( " %.6f %.6f %.6f %.6f %.6f %.6f %.6f %s %.6f\n",	
	      corrpos.x, corrpos.y, corrpos.theta,
	      odo.x, odo.y, odo.theta, scan.timestamp, host, time );

      corr_move = carmen_move_between_points( old_corrpos, corrpos );
      pos_move = carmen_move_between_points( old_pos, pos );

      fprintf( stderr, "***********************************************\n" );
      fprintf( stderr, "expected movement  %.6f m %.6f m %.6f degree\n",
	       corr_move.forward, corr_move.sideward,
	       rad2deg(corr_move.rotation) );
      fprintf( stderr, "corrected movement %.6f m %.6f m %.6f degree\n",
	       pos_move.forward, pos_move.sideward,
	       rad2deg(pos_move.rotation) );
      fprintf( stderr, "***********************************************\n\n" );
      
      old_pos     = pos;
      old_corrpos = corrpos;
      
    } else if ( (sscanf(line,"%s",command)!=0) &&
	 (!strcmp( command, "ROBOTLASER1") ) ) {

      running = line;
      strtok( running, " ");

      for (i=0; i<6; i++) {
	strtok( NULL, " ");
      }
      
      scan.num_readings = (int) (atoi( strtok( NULL, " ") ));

      //      fprintf(stderr, "%d ", scan.num_readings);

      for (i=0; i<scan.num_readings; i++) {
	scan.range[i] = (float) (atof( strtok( NULL, " ") ));
      }

      scan.num_remissions = (int) (atoi( strtok( NULL, " ") ));
      ///      fprintf(stderr, "(%d) ", scan.num_remissions);
      for (i=0; i<scan.num_remissions; i++) {
	scan.remission[i] = (float) (atof( strtok( NULL, " ") ));
      }
      

      pos.x = atof( strtok( NULL, " ") );
      pos.y = atof( strtok( NULL, " ") );
      pos.theta = atof( strtok( NULL, " ") );
      odo.x = atof( strtok( NULL, " ") );
      odo.y = atof( strtok( NULL, " ") );
      odo.theta = atof( strtok( NULL, " ") );

      for (i=0; i<5; i++) {
	strtok( NULL, " ");
      }


      scan.timestamp = atof( strtok( NULL, " ") );
      strncpy( host, strtok( NULL, " "), MAX_NAME_LENGTH );
      time = atof( strtok( NULL, " ") );

      ///      fprintf(stderr, "SM" );

      /*****************************************************************/
      corrpos = vascocore_scan_match( scan, pos );
      /*****************************************************************/
      printf( "FLASER %d", scan.num_readings );
      for (i=0; i<scan.num_readings; i++) {
	printf( " %.2f", scan.range[i] );
      }
      printf( " %.6f %.6f %.6f %.6f %.6f %.6f %.6f %s %.6f\n",	
	      corrpos.x, corrpos.y, corrpos.theta,
	      odo.x, odo.y, odo.theta, scan.timestamp, host, time );

      corr_move = carmen_move_between_points( old_corrpos, corrpos );
      pos_move = carmen_move_between_points( old_pos, pos );

      fprintf( stderr, "***********************************************\n" );
      fprintf( stderr, "expected movement  %.6f m %.6f m %.6f degree\n",
	       corr_move.forward, corr_move.sideward,
	       rad2deg(corr_move.rotation) );
      fprintf( stderr, "corrected movement %.6f m %.6f m %.6f degree\n",
	       pos_move.forward, pos_move.sideward,
	       rad2deg(pos_move.rotation) );
      fprintf( stderr, "***********************************************\n\n" );
      
      old_pos     = pos;
      old_corrpos = corrpos;
      
    } else {
      //  fputs( line, stdout );
    }
    fflush(stdout);
  }
  fclose(fp);

  return(0);
  
}
