#ifndef GPS_NMEA_MESSAGES_H
#define GPS_NMEA_MESSAGES_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
  int                 nr;             /* number of the gps unit */
  
  double              utc;            /* Universal Time Coordinated (UTC) */
  double              latitude;
  char                lat_orient;     /* N or S (North or South) */
  double              longitude;
  char                long_orient;    /* E or W (East or West) */
  int                 gps_quality;    /* GPS Quality Indicator,
				         0 - fix not available,
				         1 - GPS fix,
				         2 - Differential GPS fix */
  int                 num_satellites; /* Number of satellites in view, 00-12 */
  double              hdop;           /* Horizontal Dilution of precision */
  double              sea_level;      /* Antenna Altitude above/below 
					 mean-sea-level (geoid) */
  double              altitude;       /* Units of antenna altitude, meters */
  double              geo_sea_level;  /* Geoidal separation, the difference
					 between the WGS-84 earth ellipsoid and
					 mean-sea-level (geoid), "-" means
					 mean-sea-level below ellipsoid */
  double              geo_sep;        /* Units of geoidal separation, meters */
  int                 data_age;       /* Age of differential GPS data, time 
					 in seconds since last SC104 type 1 or
					 9 update, null field when DGPS is not
					 used */
  double              timestamp;
  char*                host;
} carmen_gps_gpgga_message;

#define CARMEN_GPS_GPGGA_MESSAGE_FMT "{int, double,double,char,double,char,int,int,double,double,double,double,double,int,double,string}"
#define CARMEN_GPS_GPGGA_MESSAGE_NAME "carmen_gps_nmea_gpgga"

#ifdef __cplusplus
}
#endif

#endif
