#ifndef UXTOOLS_UTILS_H
#define UXTOOLS_UTILS_H

#ifndef _STDIO_H
#include <stdio.h>
#endif

#ifndef	_MATH_H
#include <math.h>
#endif

#ifndef _SYS_TIME_H
#include <sys/time.h>
#endif

#ifndef MAX
#define MAX(x,y) (x > y ? x : y)
#endif

#ifndef MIN
#define MIN(x,y) (x < y ? x : y)
#endif


#ifndef MIN
#define MIN(x,y) (x < y ? x : y)
#endif

#ifndef MIN3
#define MIN3(x,y,z) MIN(MIN(x,y),z)
#endif

#ifndef MAX
#define MAX(x,y) (x > y ? x : y)
#endif

#ifndef MAX3
#define MAX3(x,y,z) MAX(MAX(x,y),z)
#endif

#ifndef TRUE
#define TRUE  1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#ifndef M_TWO_PI
#define M_TWO_PI  6.2831853071795864769252867665590058
#endif

#define ANGLE_0_25_DEG_IN_RAD 0.00436332312998582394

#define READ_MODE_SILENT    0x00
#define READ_MODE_VERBOSE   0x01
#define READ_MODE_DONT_STOP 0x02

typedef struct {
  double        x;
  double        y;
} VECTOR2;

typedef struct {
  int           numvectors;
  VECTOR2     * vec;
} VECTOR2_SET;

typedef struct {
  int           x;
  int           y;
} iVECTOR2;

typedef struct {
  int           numvectors;
  iVECTOR2    * vec;
} iVECTOR2_SET;

#define POINT2 VECTOR2

typedef struct {
  int           numpoints;
  POINT2      * p;
} POINT2_SET;

#define iPOINT2 iVECTOR2

typedef struct {
  VECTOR2     point0;
  VECTOR2     point1;
} LINE2;

typedef struct {
  int         start;
  int         end;
} EDGEREF;

typedef struct {
  int              numedges;
  EDGEREF        * edge;
} EDGEREF_SET;


typedef struct {
  int                numpoints;
  POINT2           * pt;
} POLYGON;

typedef struct {
  double        x;
  double        y;
  double        o;
} RPOS2;

typedef struct {
  double        forward;
  double        sideward;
  double        rotation;
} RMOVE2;

typedef struct {
  double        tv;
  double        rv;
} RVEL2;

typedef struct {
  struct timeval      time;
  double              x;
  double              y;
  double              o;
} POS_CORR2_DATA;

typedef struct {
  double easting;
  double northing;
  int    zone;
  char   letter;
} UTM_COORD;

typedef struct {
  double longitude;
  double latitude;
} LL_COORD;

typedef struct {
  struct timeval      time;
  double              utc;
  double              latitude;
  char                lat_orient;
  double              longitude;
  char                long_orient;
  int                 gps_quality;
  int                 num_satellites;
  int                 status;
  double              hdop;
  double              sea_level;
  double              altitude;
  double              geo_sea_level;
  double              geo_sep; 
  double              speed_over_ground;
  int                 data_age;
} GPS_DATA;

typedef struct {
  struct timeval      time;
  double              latitude;
  double              longitude;
  int                 gpsidx;
} EST_DATA;

typedef struct {
  struct timeval      time;
  double              rx;
  double              ry;
  double              rz;
} COMPASS3_DATA;

typedef struct {
  struct timeval      time;
  double              o;
} COMPASS2_DATA;

typedef struct {
  VECTOR2             min;
  VECTOR2             max;
} BOUNDING_BOX2;

typedef struct {
  VECTOR2             relpt;
  VECTOR2             abspt;
  int                 tag;
  int                 info;
} LASER_COORD2;

typedef struct {
  double              start;
  double              end;
  double              delta;
} LASER_FOV;

typedef struct {
  RMOVE2              offset;
  LASER_FOV           fov;
} LASER_PROPERTIES2;

typedef struct {
  double              base_height;
  double              top_height;
} AMTEC_SETTINGS;

typedef struct {
  VECTOR2             offset;   /* offset to robot center */
  int                 on_amtec;
  AMTEC_SETTINGS      amtec;
} LASER_PROPERTIES3;

typedef struct {
  RPOS2               pos;
  double              vel;
  double              cov_sx;
  double              cov_sy;
  double              cov_sxy;
  VECTOR2             ll;       /* lower left */
  VECTOR2             ur;       /* upper right */
} P_STATE;

typedef struct {
  int                 numdynprobs;
  float             * dynprob;    /* probability that's the beam is a human */
  int                 numpstates;
  P_STATE           * pstate;
} P_TRACKING;

typedef struct {
  int                 numprobs;
  double            * prob;   
} DISTRIBUTION;

typedef struct {
  struct timeval      time;
  int                 partial;
  int                 numvalues;
  double              fov;
  RMOVE2              offset;
  float             * val;
  float             * angle;
} LASER_DATA;

typedef struct {
  RPOS2               estpos;
  LASER_DATA          laser;
  LASER_COORD2      * coord;
  POLYGON             poly;
  BOUNDING_BOX2       bbox;
  DISTRIBUTION      * dynamic;
  int                 id;
} LASERSENS2_DATA;

typedef struct {
  struct timeval      time;
  RPOS2               rpos;
  RVEL2               rvel;
} POSSENS2_DATA;

#define GSM_CELL_TYPE    2
#define GSM_CHANNEL_TYPE 1
#define GSM_UNKNOWN_TYPE 0

#define GSM_DEV_MODEM     1
#define GSM_DEV_PHONE     2
#define GSM_DEV_UNKNOWN   0

typedef struct {
  int                 mcc;
  int                 mnc;
  int                 lac;
  int                 cid;
  int                 bsic;
  int                 ch;
  int                 rxl;
  int                 c1;
  int                 c2;
  int                 cnct;
  int                 type;
  int                 dev;
  int                 cellidx;
} GSM_CELL_INFO;

typedef struct {
  struct timeval      time;
  int                 numcells;
  GSM_CELL_INFO     * cell;
  int                 gpsidx;
} GSM_DATA;

typedef struct {
  struct timeval      time;
  char              * mac;
  char              * essid;
  char              * protocol;
  char              * mode;
  int                 nwid;       
  int                 channel;    
  int                 encryption; 
  int                 rate;       
  int                 beacon;     
  int                 signal;
  int                 link;
  int                 noise;
  char                precloc;
  float               distance;
} WIFI_DATA;



enum FILE_TYPE    { SCRIPT,
		    REC,
		    CARMEN,
		    MOOS,
                    PLAYER,
                    SAPHIRA,
                    PLACELAB,
                    UNKOWN };

#define FILE_SCRIPT_EXT              ".script"
#define FILE_CARMEN_EXT              ".crm"
#define FILE_REC_EXT                 ".rec"
#define FILE_MOOS_EXT                ".alog"
#define FILE_PLAYER_EXT              ".plf"
#define FILE_SAPHIRA_EXT             ".2d"
#define FILE_PLACELAB_EXT            ".plab"

enum ENTRY_TYPE   { POSITION,
		    CORR_POSITION,
		    LASER_VALUES,
		    LASER_VALUES3,
		    CAMERA3D,
		    AMTEC_POS,
		    HUMAN_PROB,
		    GPS,
		    ESTIMATE,
		    GSM,
		    WIFI,
		    COMPASS,
		    POS_CORR,
		    HELI_POS,
		    MARKER,
		    RFID_TAG,
                    UNKNOWN };

typedef struct {
  enum ENTRY_TYPE          type;
  int                      index;
  int                      idx1, idx2;
  int                      linenr;
} ENTRY_POSITION;

typedef struct {

  enum FILE_TYPE     system;
  
} REC2_INFO;


typedef struct {
  
  struct timeval      time;
  char              * datastr; 
  char              * tag;

} MARKER_DATA;

enum RFID_TYPE    { ALIEN, SIEMENS };

typedef struct {

  struct timeval      time;
  RPOS2               estpos;
  enum RFID_TYPE      type;
  unsigned long long  tag;
  int                 antenna;
  int                 count;
  double              distance;
  double              signal;
  
} RFID_DATA;



typedef struct {

  /* entries */
  int                 numentries;
  ENTRY_POSITION    * entry;  

  /* positions */
  int                 numpositions;
  POSSENS2_DATA     * psens;
  
  /* laser scans */
  int                 numlaserscans;
  LASERSENS2_DATA   * lsens;

  int                 numcompass;
  COMPASS3_DATA     * compass;

  int                 numwifi;
  WIFI_DATA         * wifi;

  int                 numgsm;
  GSM_DATA          * gsm;

  int                 numgps;
  GPS_DATA          * gps;

  int                 numestimates;
  EST_DATA          * est;

  int                 nummarkers;
  MARKER_DATA      *  marker;
  
  REC2_INFO           info;
  
} REC2_DATA;

typedef struct {
  double              pan;
  double              tilt;
} DECL_TYPE;

typedef struct {
  int                numvalues;
  double           * val;
} VALUE_SET;

typedef struct {
  int                numvalues;
  int              * val;
} iVALUE_SET;

typedef struct {
  int                numfaces;
  int              * facept;
} POLYGON_REF;

typedef struct {
  int                numvectors;
  int              * vecref;
} VECTORREF_SET;

typedef struct {
  int                numpoints;
  int              * ptref;
} POINTREF_SET;

typedef struct {
  int          numpoints;
  double       xm;
  double       ym;
  double       phi;
  double       ndist;
  double       error;
  VECTOR2      pos;
} LINE2_LSQFT;

typedef struct {
  int idx0;
  int idx1;
} RANGE;

typedef struct {
  int          numranges;
  RANGE      * range;
} RANGE_SET;

typedef struct {
  int          len;
  double     * val;
} GAUSS_KERNEL;

typedef struct {
  double       mu;
  double       sigma;
} GAUSSIAN;

typedef struct {
  int          numgaussians;
  GAUSSIAN   * gauss;
} GAUSSIAN_SET;

typedef struct {
  int          numgrids;
  iVECTOR2   * grid;
} GRID_LINE;

typedef struct {
  RPOS2             offset;
  double            resolution;
  float          ** maphit;
  short          ** mapsum;
  float          ** mapprob;
  float          ** calc;
  iVECTOR2          mapsize;
  VECTOR2           center;
  double            zoom;
} GRID_MAP2;

#define rad2deg rad2Deg
double        rad2Deg(double val);

#define deg2rad deg2Rad
double        deg2Rad(double val);

double        normalize_theta( double theta ); 

double        fsgn( double val );

int           sgn( int val );

double        random_gauss( void );

void *        mdalloc(int ndim, int width, ...);

void          mdfree(void *tip, int ndim);

int           load_rec2d_file( char *filename, REC2_DATA *rec,
			       enum FILE_TYPE type, int mode );

int           read_rec2d_file( char *filename, REC2_DATA *rec, int mode );

int           read_data2d_file( REC2_DATA * rec, char * filename );

int           write_rec2d_file( char *filename, REC2_DATA rec );

int           write_data2d_file( char * filename, REC2_DATA rec );

int           rec2_parse_line( char *line, REC2_DATA *rec, int alloc, int verbose );

int           carmen_parse_line( char *line, REC2_DATA *rec, int alloc, int mode );

int           moos_parse_line( char *line, REC2_DATA *rec, int alloc, int mode );

int           player_parse_line( char *line, REC2_DATA *rec, int alloc, int mode );

int           saphira_parse_line( char *line, REC2_DATA *rec, int alloc, int mode );

int           placelab_parse_line( char *line, REC2_DATA *rec, int alloc, int mode );

int           read_script( char *filename, REC2_DATA *script, int verbose );

VECTOR2       rotate_vector2( VECTOR2 p, double angle );

VECTOR2       rotate_and_translate_vector2( VECTOR2 p, double rot,
					    VECTOR2 trans );

double        vector2_length( VECTOR2 v1 );

double        vector2_distance( VECTOR2 p1, VECTOR2 p2 );

void          compute_rec2d_coordpts( REC2_DATA *rec );

VECTOR2       compute_laser_abs_point( RPOS2 rpos, double val,
				       RMOVE2 offset, double angle );

void          compute_vec2_convex_hull( VECTOR2_SET pts,
					VECTORREF_SET *hull );

double        distance_vec2_to_line2( VECTOR2 p, LINE2 l );

double        gauss_function( double x, double mu, double sigma );

GAUSS_KERNEL  compute_gauss_kernel( int length );

RMOVE2        compute_movement2_between_rpos2( RPOS2 s, RPOS2 e );

RPOS2         compute_rpos2_with_movement2( RPOS2 start, RMOVE2 move );

RPOS2         compute_rpos2_backwards_with_movement2( RPOS2 start,
						      RMOVE2 move );

double        convert_orientation_to_range( double angle );

double        compute_orientation_diff( double start, double end );

void          robot2map( RPOS2 pos, RPOS2 corr, RPOS2 *map );

void          map2robot( RPOS2 map, RPOS2 corr, RPOS2 *pos );

void          computeCorr( RPOS2 pos, RPOS2 map, RPOS2 *corr );

LINE2_LSQFT   compute_lsqf_line( VECTOR2_SET p );


void          compute_forward_correction( RPOS2 pos, RPOS2 corr, RPOS2 *cpos );

void          compute_backward_correction( RPOS2 cpos, RPOS2 corr, RPOS2 *pos );

void          compute_correction_parameters( RPOS2 pos, RPOS2 cpos, RPOS2 *corr );

void          update_correction_parameters( RPOS2 cpos, RPOS2 delta, RPOS2 *corr );


double        double_time( struct timeval time );

void          convert_time( double tval, struct timeval *time );

int           timeCompare ( struct timeval time1, struct timeval time2 );

double        timeDiff( struct timeval  t1, struct timeval t2);

double        get_user_time( void );


double        sick_laser_fov( int numbeams );

double        sick_laser_angle_diff( int num, double fov );

double        kl_distance( GAUSSIAN g1, GAUSSIAN g2 );

double        rpos2_distance( RPOS2 p1, RPOS2 p2 );

char *        extended_filename( char * filename );

char *        printable_filename( char * filename );

FILE *        fopen_filename( char * filename, char * mode );

char *        str_get_valstr( char *str );

char *        str_get_str( char *str );

int           str_get_numbers( char *str, int num, ... );

double        gps_degree_abs_decimal( double degree_minute );

double        gps_degree_decimal( double degree_minute, char orient );

UTM_COORD     ll2utm( LL_COORD ll );

LL_COORD      utm2ll( UTM_COORD utm );

#endif /* ifdef BD_UTILS_H */

