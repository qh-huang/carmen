
#ifndef GNAV_INTERFACE_H
#define GNAV_INTERFACE_H

#include "gnav_messages.h"

#ifdef __cplusplus
extern "C" {
#endif


void carmen_gnav_subscribe_room_message(carmen_gnav_room_msg *room_msg,
					carmen_handler_t handler,
					carmen_subscribe_t subscribe_how);

carmen_rooms_topology_p carmen_gnav_get_rooms_topology();

void carmen_gnav_subscribe_path_message(carmen_gnav_path_msg *path_msg,
					carmen_handler_t handler,
					carmen_subscribe_t subscribe_how);

void carmen_gnav_set_goal(int room);


#ifdef __cplusplus
}
#endif

#endif
