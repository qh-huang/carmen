#include <carmen/carmen.h>
#include "segwaycore.h"
#include "segway_ipc.h"

int quit_signal = 0;
segway_t segway;
double accel_factor, torque_factor;
int gain_schedule;

void shutdown_handler(int sig)
{
  if(sig == SIGINT)
    quit_signal = 1;
}

void read_parameters(int argc, char **argv)
{
  char *schedule;

  carmen_param_t param[] = {
    {"segway", "accel_limit", CARMEN_PARAM_DOUBLE, &accel_factor, 0, NULL},
    {"segway", "torque_limit", CARMEN_PARAM_DOUBLE, &torque_factor, 0, NULL},
    {"segway", "gain_schedule", CARMEN_PARAM_STRING, &schedule, 0, NULL},
  };
  carmen_param_install_params(argc, argv, param, sizeof(param) / 
			      sizeof(param[0]));
  if(strncmp("heavy", schedule, 5) == 0)
    gain_schedule = 2;
  else if(strncmp("tall", schedule, 4) == 0)
    gain_schedule = 1;
  else 
    gain_schedule = 0;
}

int main(int argc, char **argv)
{
  int count = 0;
  double current_time;

  /* connect to IPC network */
  carmen_initialize_ipc(argv[0]);
  carmen_param_check_version(argv[0]);
  carmen_segway_register_messages();
  
  read_parameters(argc, argv);

  segway_initialize(&segway);
  segway_clear_status(&segway);
  signal(SIGINT, shutdown_handler);
  siginterrupt(SIGINT, 1);

  segway_set_max_velocity(&segway, 1.0);
  segway_set_max_acceleration(&segway, accel_factor);
  segway_set_max_torque(&segway, torque_factor);
  segway_set_gain_schedule(&segway, SEGWAY_LIGHT);

  do {
    if(segway.status_ready) {
      current_time = carmen_get_time_ms();
      carmen_segway_publish_odometry(&segway, current_time);
      carmen_segway_publish_pose(&segway, current_time);
      sleep_ipc(0.001);

      count++;
      if(count == 10) {
	count = 0;
	fprintf(stderr, "%d%% ", (int)segway.voltage);
      }
      segway_clear_status(&segway);
    }
    segway_update_status(&segway);
  } while(!quit_signal);

  segway_stop(&segway);
  segway_free(&segway);
  return 0;
}

