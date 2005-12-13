#include <carmen/carmen.h>
#include <carmen/proccontrol_messages.h>

void
carmen_proccontrol_subscribe_pidtable_message(carmen_proccontrol_pidtable_message
					     *pidtable,
					     carmen_handler_t handler,
					     carmen_subscribe_t subscribe_how)
{
  carmen_subscribe_message(CARMEN_PROCCONTROL_PIDTABLE_NAME, 
			   CARMEN_PROCCONTROL_PIDTABLE_FMT,
			   pidtable, 
			   sizeof(carmen_proccontrol_pidtable_message), 
			   handler, subscribe_how);
}

void
carmen_proccontrol_unsubscribe_pidtable_message(carmen_handler_t handler)
{
  carmen_unsubscribe_message(CARMEN_PROCCONTROL_PIDTABLE_NAME, handler);
}

void
carmen_proccontrol_subscribe_output_message(carmen_proccontrol_output_message
					   *output,
					   carmen_handler_t handler,
					   carmen_subscribe_t subscribe_how)
{
  carmen_subscribe_message(CARMEN_PROCCONTROL_OUTPUT_NAME,
			   CARMEN_PROCCONTROL_OUTPUT_FMT,
			   output, sizeof(carmen_proccontrol_output_message), 
			   handler, subscribe_how);
}

void
carmen_proccontrol_unsubscribe_output_message(carmen_handler_t handler)
{
  carmen_unsubscribe_message(CARMEN_PROCCONTROL_OUTPUT_NAME, handler);
}

void 
carmen_proccontrol_set_module_state(char *module_name, int requested_state)
{
  static carmen_proccontrol_moduleset_message msg;
  static int first = 1;

  if(first) {
    carmen_ipc_define_test_exit(CARMEN_PROCCONTROL_MODULESET_NAME, 
				CARMEN_PROCCONTROL_MODULESET_FMT);
    msg.host = carmen_get_host();
    first = 0;
  }
  msg.module_name = module_name;
  msg.requested_state = requested_state;
  msg.timestamp = carmen_get_time();
  carmen_ipc_publish_exit(CARMEN_PROCCONTROL_MODULESET_NAME, msg);
}

void 
carmen_proccontrol_set_group_state(char *group_name, int requested_state)
{
  static carmen_proccontrol_groupset_message msg;
  static int first = 1;

  if(first) {
    carmen_ipc_define_test_exit(CARMEN_PROCCONTROL_GROUPSET_NAME, 
				CARMEN_PROCCONTROL_GROUPSET_FMT);
    msg.host = carmen_get_host();
    first = 0;
  }
  msg.group_name = group_name;
  msg.requested_state = requested_state;
  msg.timestamp = carmen_get_time();
  carmen_ipc_publish_exit(CARMEN_PROCCONTROL_GROUPSET_NAME, msg);
}
