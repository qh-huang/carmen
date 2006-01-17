
#include <carmen/carmen.h>
#include <carmen/carmen_stdio.h>

/* this programm shows an example how to read log files
   with the readlog library provided by logger           */

int main(int argc, char* argv[]) {

  if (argc != 2) 
    carmen_die("Syntax: %s <carmen-logfile>\n", argv[0]);

  int i;
  char tenchar[11];
  char* rest = NULL;

  /*   file handler */
  carmen_FILE *logfile = NULL;

  /*   log file index structure */
  carmen_logfile_index_p logfile_index = NULL;

  /*  open logfile for reading */
  logfile = carmen_fopen(argv[1], "r");
  if(logfile == NULL)
    carmen_die("Error: could not open file %s for reading.\n", argv[1]);

  /* create index structure */
  logfile_index = carmen_logfile_index_messages(logfile);
  
  /*   buffer for reading the lines */
  const int max_line_length=99999;
  char line[max_line_length+1];
  
  /*   used to read the messages. erase stucture before! */
  carmen_robot_laser_message l;
  carmen_erase_structure(&l, sizeof(carmen_robot_laser_message) );
  carmen_base_odometry_message o;
  carmen_erase_structure(&o, sizeof(carmen_base_odometry_message) );

  /*   iterate over all entries in the logfile */
  while (!carmen_logfile_eof(logfile_index)) {
    
    /*     read the line from the file */
    carmen_logfile_read_next_line(logfile_index, 
				  logfile,
				  max_line_length, 
				  line);
    
    if (strncmp(line, "ROBOTLASER1 ", 12) == 0) {

      rest = carmen_string_to_robot_laser_message(line, &l);

      fprintf(stdout, "FLASER %d ", l.num_readings);
      for (i=0; i<l.num_readings; i++)
	fprintf(stdout, "%.2f ", l.range[i]);
      fprintf(stdout, "%lf %lf %lf ", l.laser_pose.x, l.laser_pose.y, l.laser_pose.theta);
      fprintf(stdout, "%lf %lf %lf ", l.robot_pose.x, l.robot_pose.y, l.robot_pose.theta);

      strncpy(tenchar, l.host, 10);
      tenchar[10]='\0';
      fprintf(stdout, "%lf %s %s", l.timestamp, tenchar, rest);
    }
    else if (strncmp(line, "ROBOTLASER2 ", 12) == 0) {

      rest = carmen_string_to_robot_laser_message(line, &l);

      fprintf(stdout, "RLASER %d ", l.num_readings);
      for (i=0; i<l.num_readings; i++)
	fprintf(stdout, "%.2f ", l.range[i]);
      fprintf(stdout, "%lf %lf %lf ", l.laser_pose.x, l.laser_pose.y, l.laser_pose.theta);
      fprintf(stdout, "%lf %lf %lf ", l.robot_pose.x, l.robot_pose.y, l.robot_pose.theta);

      strncpy(tenchar, l.host, 10);
      tenchar[10]='\0';
      fprintf(stdout, "%lf %s %s", l.timestamp, tenchar, rest);
    }
    else if (strncmp(line, "FLASER ", 7) == 0) {
      fputs(line, stdout);
    }
    else if (strncmp(line, "RLASER", 7) == 0) {
      fputs(line, stdout);
    }
    else if (strncmp(line, "RAWLASER", 8) == 0) {
      fputs(line, stdout);
    }
    else if (strncmp(line, "ODOM ", 5) == 0) {
      fputs(line, stdout);
    }
  }

  fflush(stdout);

  /* close the file */
  carmen_fclose(logfile);

  /* free index structure */
  carmen_logfile_free_index(&logfile_index);
  return 0;    
}
