#include <carmen/carmen.h>
#include <ctype.h>

int carmen_logger_nogz;
int carmen_playback_nogz;

carmen_logger_file_p outfile = NULL;
double logger_starttime;
carmen_localize_globalpos_message *localize_msg;


void shutdown_module(int sig)
{
  if(sig == SIGINT) {
    carmen_logger_fclose(outfile);
    carmen_ipc_disconnect();
    fprintf(stderr, "\nDisconnecting.\n");
    exit(0);
  }
}

void robot_frontlaser_handler(carmen_robot_laser_message *frontlaser)
{
  int i;
  double x, y;

  if (localize_msg->timestamp == 0)
    return;

  fprintf(stderr, "F");
  carmen_localize_correct_laser(frontlaser, localize_msg);

  for(i = 0; i < frontlaser->num_readings; i++) {
    x = frontlaser->laser_location.x+cos(-M_PI/2 + M_PI*i/180.0 + frontlaser->laser_location.theta)*frontlaser->range[i];
    y = frontlaser->laser_location.y+sin(-M_PI/2 + M_PI*i/180.0 + frontlaser->laser_location.theta)*frontlaser->range[i];
    carmen_logger_fprintf(outfile, "%.2f %.2f\n", x, y);
  }
}

int main(int argc, char **argv)
{
  char filename[1024];
  char key;

  carmen_ipc_initialize(argc, argv);
  carmen_param_check_version(argv[0]);	

  if (argc < 2) 
    carmen_die("usage: %s [--nogz] <logfile>\n", argv[0]);

  carmen_logger_nogz = 0;
  if (argc == 2)
    sprintf(filename, argv[1]);
  else if (argc == 3) {
    if (!strcmp(argv[1], "--nogz"))
      carmen_logger_nogz = 1;
    else
      carmen_die("usage: %s [--nogz] <logfile>\n", argv[0]);
    sprintf(filename, argv[2]);
  }

#ifndef NO_ZLIB
  if (!carmen_logger_nogz && strcmp(filename + strlen(filename) - 3, ".gz"))
    sprintf(filename + strlen(filename), ".gz");
#endif

  outfile = carmen_logger_fopen(filename, "r");
  if (outfile != NULL) {
    fprintf(stderr, "Overwrite %s? ", filename);
    scanf("%c", &key);
    if (toupper(key) != 'Y')
      exit(-1);
    carmen_logger_fclose(outfile);
  }
  outfile = carmen_logger_fopen(filename, "w");
  if(outfile == NULL)
    carmen_die("Error: Could not open file %s for writing.\n", filename);
  carmen_logger_write_header(outfile);

  carmen_robot_subscribe_frontlaser_message(NULL, 
					    (carmen_handler_t)robot_frontlaser_handler, 
					    CARMEN_SUBSCRIBE_ALL);
  localize_msg = (carmen_localize_globalpos_message *)
    calloc(1, sizeof(carmen_localize_globalpos_message));
  carmen_test_alloc(localize_msg);
  carmen_localize_subscribe_globalpos_message(localize_msg, NULL, CARMEN_SUBSCRIBE_ALL);

  logger_starttime = carmen_get_time();


  signal(SIGINT, shutdown_module);
  carmen_ipc_dispatch();
  return 0;
}
