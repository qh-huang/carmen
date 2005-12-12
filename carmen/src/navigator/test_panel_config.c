#include <carmen/carmen.h>

void
navigator_display_config(char *attribute, int value)
{
  carmen_navigator_display_config_message msg;
  IPC_RETURN_TYPE err;

  if (strncmp(attribute, "reset all", 9) == 0)
    {
      msg.attribute = attribute;
      msg.reset_all_to_defaults = 1;
    }
  else
    {
      msg.timestamp = carmen_get_time();
      msg.host = carmen_get_host();
      msg.attribute = attribute;
      msg.value = value;
      msg.reset_all_to_defaults = 0;
    }

  err = IPC_publishData(CARMEN_NAVIGATOR_DISPLAY_CONFIG_NAME, &msg);
  carmen_test_ipc_exit(err, "Could not publish", 
		       CARMEN_NAVIGATOR_DISPLAY_CONFIG_NAME);
}

int
main (int argc __attribute__ ((unused)), char *argv[] __attribute__ ((unused)))
{
  carmen_initialize_ipc(argv[0]);
  carmen_param_check_version(argv[0]);

  navigator_display_config("robot colour", 0xff << 16 | 218 << 8 | 185);
  sleep(3);
  navigator_display_config("robot colour", -1);
  navigator_display_config("track robot", 0);
  navigator_display_config("show particles", 1);
  sleep(3);
  navigator_display_config("reset all", 1);

  return 0;
}
