
/** @addtogroup gps libgps_interface **/
// @{

/** \file gps_nmea_interface.h
 * \brief Definition of the interface of the module gps.
 *
 * This file specifies the interface to subscribe the messages of
 * that module and to receive its data via ipc.
 **/

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
void
carmen_gps_subscribe_nmea_rmc_message(carmen_gps_gprmc_message *nmea,
				  carmen_handler_t handler,
				  carmen_subscribe_t subscribe_how);
              
#ifdef __cplusplus
}
#endif

#endif

//@}
