#ifndef SEGWAY_INTERFACE_H
#define SEGWAY_INTERFACE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "segway_messages.h"

void
carmen_segway_subscribe_pose_message(carmen_segway_pose_message *pose,
				     carmen_handler_t handler,
				     carmen_subscribe_t subscribe_how);

#ifdef __cplusplus
}
#endif

#endif
