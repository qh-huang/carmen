
#ifndef WALKER_INTERFACE_H
#define WALKER_INTERFACE_H

#include "walker_messages.h"

#ifdef __cplusplus
extern "C" {
#endif


void carmen_walker_set_goal(int goal);

void carmen_walker_subscribe_goal_changed_message
(carmen_walker_goal_changed_msg *msg, carmen_handler_t handler,
 carmen_subscribe_t subscribe_how);

#ifdef __cplusplus
}
#endif

#endif
