#ifndef WALKERSERIAL_INTERFACE_H
#define WALKERSERIAL_INTERFACE_H

#include <carmen/carmen.h>
#include "walkerserial_messages.h"

#ifdef __cplusplus
extern "C" {
#endif


void carmen_walkerserial_subscribe_button_message
(carmen_walkerserial_button_msg *button_msg,
 carmen_handler_t handler,
 carmen_subscribe_t subscribe_how);

void carmen_walkerserial_subscribe_clutch_message
(carmen_walkerserial_clutch_msg *clutch_msg,
 carmen_handler_t handler,
 carmen_subscribe_t subscribe_how);

void carmen_walkerserial_engage_clutch();

void carmen_walkerserial_disengage_clutch();


#ifdef __cplusplus
}
#endif

#endif
