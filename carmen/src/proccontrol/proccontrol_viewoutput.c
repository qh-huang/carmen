#include <carmen/carmen.h>
#include <carmen/param_interface.h>
#include <carmen/proccontrol_interface.h>

void output_handler(carmen_proccontrol_output_message *output)
{
  fprintf(stderr, "%s", output->output);
}

int main(int argc, char **argv)
{
  /* connect to the IPC server, regsiter messages */
  carmen_ipc_initialize(argc, argv);

  carmen_proccontrol_subscribe_output_message(NULL, (carmen_handler_t)
					      output_handler,
					      CARMEN_SUBSCRIBE_ALL);
  carmen_ipc_dispatch();
  return 0;
}
