#include <stdio.h>
#include "valet_interface.h"

int main(int argc __attribute__ ((unused)), char *argv[]) {

  int park = 1;

  carmen_initialize_ipc(argv[0]);
  carmen_param_check_version(argv[0]);

  while (1) {
    if (fgetc(stdin) == '\n') {
      if (park)
	carmen_valet_park();
      else
	carmen_valet_return();
      park = !park;
    }
  }

  return 0;
}
