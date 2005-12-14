#include <carmen/carmen.h>

carmen_FILE *outfile = NULL;
double logger_starttime;
carmen_localize_globalpos_message *localize_msg;

void shutdown_module(int sig)
{
  if(sig == SIGINT) {
    carmen_fclose(outfile);
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
    x = frontlaser->laser_pose.x+cos(-M_PI/2 + M_PI*i/180.0 + frontlaser->laser_pose.theta)*frontlaser->range[i];
    y = frontlaser->laser_pose.y+sin(-M_PI/2 + M_PI*i/180.0 + frontlaser->laser_pose.theta)*frontlaser->range[i];
    carmen_fprintf(outfile, "%.2f %.2f\n", x, y);
  }
}

int main(int argc, char **argv)
{
  char filename[1024];
  char key;

  carmen_ipc_initialize(argc, argv);
  carmen_param_check_version(argv[0]);	

  if(argc < 2) 
    carmen_die("usage: %s <logfile>\n", argv[0]);

  sprintf(filename, argv[1]);

  outfile = carmen_fopen(filename, "r");
  if (outfile != NULL) {
    fprintf(stderr, "Overwrite %s? ", filename);
    scanf("%c", &key);
    if (toupper(key) != 'Y')
      exit(-1);
    carmen_fclose(outfile);
  }
  outfile = carmen_fopen(filename, "w");
  if(outfile == NULL)
    carmen_die("Error: Could not open file %s for writing.\n", filename);
  carmen_logwrite_write_header(outfile);

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
