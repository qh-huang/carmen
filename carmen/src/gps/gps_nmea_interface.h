#ifndef GPS_NMEA_INTERFACE_H
#define GPS_NMEA_INTERFACE_H

#include <carmen/carmen.h>
#include <carmen/gps_nmea_messages.h>

#ifdef __cplusplus
extern "C" {
#endif

void
carmen_gps_subscribe_nmea_message(carmen_gps_gpgga_message *nmea,
				  carmen_handler_t handler,
				  carmen_subscribe_t subscribe_how);
       
#ifdef __cplusplus
}
#endif

#endif
