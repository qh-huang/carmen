#include <carmen/carmen.h>
#include "segwaycore.h"

int quit_signal = 0;
segway_t segway;
double accel_factor, torque_factor;
int gain_schedule;

void shutdown_handler(int sig)
{
  if(sig == SIGINT)
    quit_signal = 1;
}

static void segway_velocity_handler(MSG_INSTANCE msgRef, 
				    BYTE_ARRAY callData,
				    void *clientData __attribute__ ((unused)))
{
  IPC_RETURN_TYPE err;
  carmen_base_velocity_message vel;
 
  err = IPC_unmarshallData(IPC_msgInstanceFormatter(msgRef), callData, &vel,
                           sizeof(carmen_base_velocity_message));
  IPC_freeByteArray(callData);
  carmen_test_ipc_return(err, "Could not unmarshall", 
			 IPC_msgInstanceName(msgRef));
  segway_set_velocity(&segway, vel.tv, vel.rv);
}

static void publish_segway_odometry(segway_p segway)
{
  carmen_base_odometry_message odometry;
  IPC_RETURN_TYPE err;

  odometry.x = segway->x;
  odometry.y = segway->y;
  odometry.theta = segway->theta;
  odometry.tv = (segway->lw_velocity + segway->rw_velocity) / 2.0;
  odometry.rv = segway->yaw_rate;
  odometry.acceleration = 0;
  odometry.timestamp = carmen_get_time_ms();
  strcpy(odometry.host, carmen_get_tenchar_host_name());
  err = IPC_publishData(CARMEN_BASE_ODOMETRY_NAME, &odometry);
  carmen_test_ipc_exit(err, "Could not publish", CARMEN_BASE_ODOMETRY_NAME);
}

void register_ipc_messages(void)
{
  IPC_RETURN_TYPE err;
  
  /* define messages created by this module */
  err = IPC_defineMsg(CARMEN_BASE_ODOMETRY_NAME, IPC_VARIABLE_LENGTH,
                      CARMEN_BASE_ODOMETRY_FMT);
  carmen_test_ipc_exit(err, "Could not define", CARMEN_BASE_ODOMETRY_NAME);
  
  err = IPC_defineMsg(CARMEN_BASE_SONAR_NAME, IPC_VARIABLE_LENGTH,
                      CARMEN_BASE_SONAR_FMT);
  carmen_test_ipc_exit(err, "Could not define IPC message", 
		       CARMEN_BASE_SONAR_NAME);

  err = IPC_defineMsg(CARMEN_BASE_VELOCITY_NAME, IPC_VARIABLE_LENGTH,
                      CARMEN_BASE_VELOCITY_FMT);
  carmen_test_ipc_exit(err, "Could not define", CARMEN_BASE_VELOCITY_NAME);

  /* setup incoming message handlers */
  err = IPC_subscribe(CARMEN_BASE_VELOCITY_NAME, 
                      segway_velocity_handler, NULL);
  carmen_test_ipc_exit(err, "Could not subscribe", CARMEN_BASE_VELOCITY_NAME);
  IPC_setMsgQueueLength(CARMEN_BASE_VELOCITY_NAME, 1);
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

  /* connect to IPC network */
  carmen_initialize_ipc(argv[0]);
  carmen_param_check_version(argv[0]);
  register_ipc_messages();
  
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
      count++;
      if(count == 10) {
	publish_segway_odometry(&segway);
	count = 0;
	sleep_ipc(0.01);
	fprintf(stderr, "%dV ", (int)segway.voltage);
      }
      segway_clear_status(&segway);
    }
    segway_update_status(&segway);
  } while(!quit_signal);

  segway_stop(&segway);
  segway_free(&segway);
  return 0;
}

