
#ifndef DOT_INTERFACE_H
#define DOT_INTERFACE_H

#include "dot_messages.h"

#ifdef __cplusplus
extern "C" {
#endif

void carmen_dot_start();
void carmen_dot_subscribe_pos_message(carmen_dot_pos_msg *msg,
					   carmen_handler_t handler,
					   carmen_subscribe_t subscribe_how);


#ifdef __cplusplus
}
#endif

#endif
