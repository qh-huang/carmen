#include <carmen/carmen.h>
#include <limits.h>

#include "cerebellum_com.h"

#define METRES_PER_CEREBELLUM .010
#define ROT_VEL_FACT_RAD 100

static double max_t_vel = 1.0;

static void 
set_wheel_velocities(double vl, double vr)
{
  if(vl > max_t_vel)
    vl = max_t_vel;
  else if(vl < -max_t_vel)
    vl = -max_t_vel;
  if(vr > max_t_vel)
    vr = max_t_vel;
  else if(vr < -max_t_vel)
    vr = -max_t_vel;

  vl /= METRES_PER_CEREBELLUM;
  vr /= METRES_PER_CEREBELLUM;

  carmen_cerebellum_set_velocity((int)vl, (int)vr);
}

void 
shutdown_cerebellum(int signo __attribute__ ((unused))) 
{
  fprintf(stderr, "\nShutting down robot...");
  sleep(1);
  carmen_cerebellum_disconnect_robot();
  fprintf(stderr, "done.\n");
  carmen_terminal_restore();
  exit(0);
}

int 
main(int argc __attribute__ ((unused)), 
     char **argv __attribute__ ((unused))) 
{
  double wheel_diameter;
  char dev_name[100];
  double vl, vr;
  int k;
  double tv, rv;
  int a,b,c,d;
  int error;

  tv = rv = a = b = c = d = error = 0;

  signal(SIGINT, shutdown_cerebellum);
  signal(SIGTERM, shutdown_cerebellum);

  carmen_terminal_cbreak(0);

  strcpy(dev_name, "/dev/ttyS0");
  wheel_diameter = 0.165;

  if (carmen_cerebellum_connect_robot(dev_name) < 0) 
    return -1;

  while(1) {
    //fprintf(stderr, ".");

    k = getchar();

    if (k != EOF) {
      switch(k) {
      case 'e': 
	error = carmen_cerebellum_get_state(&a, &b, &c, &d);

	fprintf(stderr, "%8x %8x %8x %8x %d\n",a,b,c,d,error);

	break;
      case 'd': carmen_cerebellum_limp(); break;
      case 'u': tv = 0.5; rv = 0.2; break;
      case 'i': tv = 0.5; rv = 0.0; break;
      case 'o': tv = 0.5; rv = -0.2; break;
      case 'j': tv = 0.0; rv = 0.2; break;
      case 'k': tv = 0.0; rv = 0; break;
      case 'l': tv = 0.0; rv = -0.2; break;
      case 'm': tv = -0.5; rv = 0.2; break;
      case ',': tv = -0.5; rv = 0; break;
      case '.': tv = -0.5; rv = -0.2; break;
      default:  tv = 0.0; rv = 0; break;
      }

      if (tv == 0.0 && rv == 0.0) {
	carmen_cerebellum_set_velocity(0, 0);
      } else {
	vl = tv / METRES_PER_CEREBELLUM;
	vr = tv / METRES_PER_CEREBELLUM;
	vl -= 0.5 * rv * ROT_VEL_FACT_RAD*3;
	vr += 0.5 * rv * ROT_VEL_FACT_RAD*3;
	fprintf(stderr, "%x %x\n",(int)vl,(int)vr);
	carmen_cerebellum_set_velocity((int)vl, (int)vr);
      }
    }
  }
  return 0;
}
