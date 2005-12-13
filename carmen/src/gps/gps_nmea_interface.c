#include <carmen/carmen.h>
#include <carmen/gps_nmea_interface.h>

void
carmen_gps_subscribe_nmea_message(carmen_gps_gpgga_message *nmea,
				  carmen_handler_t handler,
				  carmen_subscribe_t subscribe_how)
{
  carmen_subscribe_message(CARMEN_GPS_GPGGA_MESSAGE_NAME, 
			   CARMEN_GPS_GPGGA_MESSAGE_FMT,
			   nmea, sizeof(carmen_gps_gpgga_message), handler,
			   subscribe_how);
}


