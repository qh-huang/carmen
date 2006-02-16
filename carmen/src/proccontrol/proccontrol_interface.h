
/** @addtogroup proccontrol libproccontrol_interface **/
// @{

/** \file proccontrol_interface.h
 * \brief Definition of the interface of the module proccontrol.
 *
 * This file specifies the interface to subscribe the messages of
 * that module and to receive its data via ipc.
 **/

#ifndef CARMEN_PROCCONTROL_INTERFACE_H
#define CARMEN_PROCCONTROL_INTERFACE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <carmen/proccontrol_messages.h>

void
carmen_proccontrol_subscribe_pidtable_message(carmen_proccontrol_pidtable_message
					     *pidtable,
					     carmen_handler_t handler,
					     carmen_subscribe_t subscribe_how);
  
void
carmen_proccontrol_unsubscribe_pidtable_message(carmen_handler_t handler);

void
carmen_proccontrol_subscribe_output_message(carmen_proccontrol_output_message *output,
					   carmen_handler_t handler,
					   carmen_subscribe_t subscribe_how);
  
void
carmen_proccontrol_unsubscribe_output_message(carmen_handler_t handler);

void 
carmen_proccontrol_set_module_state(char *module_name, int requested_state);

void 
carmen_proccontrol_set_group_state(char *group_name, int requested_state);

#ifdef __cplusplus
}
#endif

#endif
// @}
