#ifndef CARMEN_PANTILT_INTERFACE_H
#define CARMEN_PANTILT_INTERFACE_H

#include <carmen/pantilt_messages.h>

#ifdef __cplusplus
extern "C" {
#endif

void 
carmen_pantilt_subscribe_status_message(carmen_pantilt_status_message *pantilt,
					carmen_handler_t handler,
					carmen_subscribe_t subscribe_how);
void
carmen_pantilt_unsubscribe_status_message(carmen_handler_t handler);
  
void 
carmen_pantilt_move( double pan, double tilt );

void 
carmen_pantilt_move_pan( double pan );

void 
carmen_pantilt_move_tilt( double tilt );


#ifdef __cplusplus
}
#endif

#endif
