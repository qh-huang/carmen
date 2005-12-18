/* #include <stdio.h> */
/* #include <stdlib.h> */
/* #include <string.h> */
/* #include <signal.h> */
/* #include <math.h> */
/* #include <unistd.h> */
/* #include <termios.h> */
/* #include <fcntl.h> */
/* #include <sys/signal.h> */
/* #include <sys/types.h> */
/* #include <sys/time.h> */
/* #include <time.h> */
/* #include <sys/ioctl.h> */

#include <carmen/carmen.h>
#include <carmen/carmen_stdio.h>

#include "vascocore.h"


void
print_usage( void )
{
  fprintf( stderr, "usage: vasco-tiny [options] <carmen-log-file>\n" );
  fprintf( stderr, "  Options:  -f, --flaser   : use old FLASER instead of ROBOTLASER1\n" );
}

double
rad2deg(double x)
{
  return x * 57.29577951308232087679;
}


int
main( int argc, char *argv[] )
{ 
  carmen_point_t                  pos, corrpos, odo;
  double                          time;
  int                             i;
  int                             scancnt;
  carmen_move_t                   pos_move = {0.0, 0.0, 0.0};
  carmen_move_t                   corr_move = {0.0, 0.0, 0.0};
  carmen_point_t                  old_pos = {0.0, 0.0, 0.0};
  carmen_point_t                  old_corrpos = {0.0, 0.0, 0.0};
  int use_flaser = 0;
  
  if (argc < 2) {
    print_usage();
    exit(1);
  }

  scancnt=0;
  for (i=1; i < argc-1; i++) {
    if (!strcmp(argv[i], "-f")  || !strcmp(argv[i], "--flaser"))
      use_flaser = 1;
  }

  carmen_ipc_initialize(argc, argv);
  vascocore_init(argc, argv);

  /*   file handler */
  carmen_FILE *logfile = NULL;

  /*   log file index structure */
  carmen_logfile_index_p logfile_index = NULL;

  /*  open logfile for reading */
  logfile = carmen_fopen(argv[argc-1], "r");
  if(logfile == NULL)
    carmen_die("Error: could not open file %s for reading.\n", argv[1]);

  /* create index structure */
  logfile_index = carmen_logfile_index_messages(logfile);
  
  /*   buffer for reading the lines */
  const int max_line_length=9999;
  char line[max_line_length+1];
  
  /*   used to read the messages. erase stucture before! */
  carmen_robot_laser_message l;
  carmen_erase_structure(&l, sizeof(carmen_robot_laser_message) );

  /*   iterate over all entries in the logfile */
  while (!carmen_logfile_eof(logfile_index)) {

    int correct_msg = 1;
    /*     read the line from the file */
    carmen_logfile_read_next_line(logfile_index, 
				  logfile,
				  max_line_length, 
				  line);
    
    /*     what kind of message it this? */
    if (use_flaser == 0 && strncmp(line, "ROBOTLASER1 ", 12) == 0) {
      /*  convert the string using the corresponding read function */
      char* next = carmen_string_to_robot_laser_message(line, &l);
      time = atof( strtok( next, " ") );

    }
    else if (use_flaser == 1 && strncmp(line, "FLASER ", 7) == 0) {
      /*  convert the string using the corresponding read function */
      char* next = carmen_string_to_robot_laser_message_orig(line, &l);
      time = atof( strtok( next, " ") );
    }
    else {
      correct_msg = 0;
      fputs(line, stdout);
    }

    if (correct_msg) {
      odo = l.robot_pose;
      pos = l.laser_pose;

      carmen_laser_laser_message scan;
      carmen_erase_structure(&scan,  sizeof(carmen_laser_laser_message) );
      scan.timestamp      = l.timestamp;
      scan.config         = l.config;
      scan.num_readings   = l.num_readings;
      scan.num_remissions = l.num_remissions;
      scan.range          = l.range;
      scan.remission      = l.remission;
      scan.host           = l.host;
      corrpos = vascocore_scan_match( scan, pos );
      scancnt++;
      corr_move = carmen_move_between_points( old_corrpos, corrpos );
      pos_move = carmen_move_between_points( old_pos, pos );

      fprintf( stderr, "expected movement  %.6f m %.6f m %.6f degree\n",
	       corr_move.forward, corr_move.sideward,
	       rad2deg(corr_move.rotation) );
      fprintf( stderr, "corrected movement %.6f m %.6f m %.6f degree\n\n",
	       pos_move.forward, pos_move.sideward,
	       rad2deg(pos_move.rotation) );

      l.laser_pose = corrpos;

      carmen_logwrite_write_robot_laser(&l,1,(carmen_FILE*)stdout, time);
      fflush(stdout);
   
      old_pos     = pos;
      old_corrpos = corrpos;
    }
    
  }
  
  /* close the file */
  carmen_fclose(logfile);

  fprintf(stderr, "\ndone, corrected %d messages!\n", scancnt);

  /* free index structure */
  carmen_logfile_free_index(&logfile_index);
  
  return(0);
  
}
