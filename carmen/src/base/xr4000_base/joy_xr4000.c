#include <carmen/carmen.h>

#define        TRANSLATE_VELOCITY           40.0
#define        ROTATE_VELOCITY              carmen_degrees_to_radians(40.0)

#ifdef blah
static void send_base_velocity_command(double tv, double rv)
{
  IPC_RETURN_TYPE err;
  char *host;
  static carmen_base_velocity_message v;

  v.tv = tv;
  v.rv = rv;
  v.timestamp = carmen_get_time();
  v.host = carmen_get_host();

  err = IPC_publishData(CARMEN_BASE_VELOCITY_NAME, &v);
  carmen_test_ipc(err, "Could not publish", CARMEN_BASE_VELOCITY_NAME);  
}
#endif

static void send_base_holonomic_velocity_command(double xv, double yv, double rv)
{
  IPC_RETURN_TYPE err;
  char *host;
  static carmen_base_holonomic_velocity_message v;
  static int first = 1;

  if(first) {
    host = carmen_get_tenchar_host_name();
    strcpy(v.host, host);
    first = 0;
  }
  v.xv = xv;
  v.yv = yv;
  v.rv = rv;
  
  v.timestamp = carmen_get_time();

  err = IPC_publishData(CARMEN_BASE_HOLONOMIC_VELOCITY_NAME, &v);
  carmen_test_ipc(err, "Could not publish", CARMEN_BASE_HOLONOMIC_VELOCITY_NAME);  
}

int main(int argc, char *argv[])
{
  int joystick, a, b;
  int *axes, *buttons;
  double forwardv, sidev, rv;

  joystick = init_joystick(&a, &b);
  if(joystick < 0)
    carmen_die("Error: could not initialize joystick.\n");
  else {
    axes = (int *)calloc(a, sizeof(int));
    carmen_test_alloc(axes);
    buttons = (int *)calloc(b, sizeof(int));
    carmen_test_alloc(buttons);
  }

  carmen_ipc_initialize(argc, argv);

  while(1) {
    if(get_joystick(axes, buttons) >= 0) {
      forwardv = axes[0] / 32767.0 * TRANSLATE_VELOCITY;
      sidev = axes[1] / 32767.0 * TRANSLATE_VELOCITY;
      rv = -axes[2] / 32767.0 * ROTATE_VELOCITY;
      send_base_holonomic_velocity_command(forwardv, sidev, rv);
    }
    carmen_ipc_sleep(0.05);
  }

  return 0;
}
