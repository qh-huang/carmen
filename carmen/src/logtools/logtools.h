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
} logtools_vector2_t;

typedef struct {
  int           numvectors;
  logtools_vector2_t     * vec;
} logtools_vector2_t_SET;

typedef struct {
  int           x;
  int           y;
} logtools_ivector2_t;

typedef struct {
  int                      numvectors;
  logtools_ivector2_t    * vec;
} logtools_ivector2_set_t;

#define POINT2 logtools_vector2_t

typedef struct {
  double        x;
  double        y;
  double        o;
} logtools_rpos2_t;

typedef struct {
  double        forward;
  double        sideward;
  double        rotation;
} logtools_rmove2_t;

typedef struct {
  double        tv;
  double        rv;
} logtools_rvel2_t;

typedef struct {
  double easting;
  double northing;
  int    zone;
  char   letter;
} logtools_utm_coord_t;

typedef struct {
  double longitude;
  double latitude;
} logtools_ll_coord_t;

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
} logtools_gps_data_t;

typedef struct {
  logtools_vector2_t             min;
  logtools_vector2_t             max;
} BOUNDING_BOX2;

typedef struct {
  logtools_vector2_t             relpt;
  logtools_vector2_t             abspt;
  int                 tag;
  int                 info;
} LASER_COORD2;

typedef struct {
  double              start;
  double              end;
  double              delta;
} LASER_FOV;

typedef struct {
  logtools_rmove2_t              offset;
  LASER_FOV           fov;
} LASER_PROPERTIES2;

typedef struct {
  double              base_height;
  double              top_height;
} AMTEC_SETTINGS;

typedef struct {
  logtools_vector2_t             offset;   /* offset to robot center */
  int                 on_amtec;
  AMTEC_SETTINGS      amtec;
} LASER_PROPERTIES3;

typedef struct {
  logtools_rpos2_t               pos;
  double              vel;
  double              cov_sx;
  double              cov_sy;
  double              cov_sxy;
  logtools_vector2_t             ll;       /* lower left */
  logtools_vector2_t             ur;       /* upper right */
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
  logtools_rmove2_t              offset;
  float             * val;
  float             * angle;
} LASER_DATA;

typedef struct {
  logtools_rpos2_t               estpos;
  LASER_DATA          laser;
  LASER_COORD2      * coord;
  BOUNDING_BOX2       bbox;
  DISTRIBUTION      * dynamic;
  int                 id;
} logtools_lasersens2_data_t;

typedef struct {
  struct timeval      time;
  logtools_rpos2_t               rpos;
  logtools_rvel2_t               rvel;
} logtools_possens2_data_t;

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



enum logtools_file_t    { SCRIPT,
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
		    LASER_VALUES,
		    HUMAN_PROB,
		    GPS,
		    GSM,
		    WIFI,
		    MARKER,
                    UNKNOWN };

typedef struct {
  enum ENTRY_TYPE          type;
  int                      index;
  int                      idx1, idx2;
  int                      linenr;
} ENTRY_POSITION;

typedef struct {

  enum logtools_file_t     system;
  
} REC2_INFO;


typedef struct {
  
  struct timeval      time;
  char              * datastr; 
  char              * tag;

} MARKER_DATA;

enum RFID_TYPE    { ALIEN, SIEMENS };

typedef struct {

  struct timeval      time;
  logtools_rpos2_t               estpos;
  enum RFID_TYPE      type;
  unsigned long long  tag;
  int                 antenna;
  int                 count;
  double              distance;
  double              signal;
  
} RFID_DATA;



typedef struct {

  /* entries */
  int                            numentries;
  ENTRY_POSITION               * entry;  

  /* positions */
  int                            numpositions;
  logtools_possens2_data_t     * psens;
  
  /* laser scans */
  int                            numlaserscans;
  logtools_lasersens2_data_t   * lsens;

  int                            numwifi;
  WIFI_DATA                    * wifi;

  int                            numgsm;
  GSM_DATA                     * gsm;

  int                            numgps;
  logtools_gps_data_t          * gps;

  int                            nummarkers;
  MARKER_DATA                 *  marker;
  
  REC2_INFO                     info;
  
} logtools_rec2_data_t;

typedef struct {
  int                numvalues;
  double           * val;
} logtools_value_set_t;

typedef struct {
  int                numvalues;
  int              * val;
} logtools_ivalue_set_t;

typedef struct {
  int                numvectors;
  int              * vecref;
} VECTORREF_SET;

typedef struct {
  int                numpoints;
  int              * ptref;
} POINTREF_SET;

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
  logtools_ivector2_t   * grid;
} GRID_LINE;

typedef struct {
  logtools_rpos2_t             offset;
  double            resolution;
  float          ** maphit;
  short          ** mapsum;
  float          ** mapprob;
  float          ** calc;
  logtools_ivector2_t          mapsize;
  logtools_vector2_t           center;
  double            zoom;
} logtools_grid_map2_t;

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

int           load_rec2d_file( char *filename, logtools_rec2_data_t *rec,
			       enum logtools_file_t type, int mode );

int           read_rec2d_file( char *filename, logtools_rec2_data_t *rec, int mode );

int           read_data2d_file( logtools_rec2_data_t * rec, char * filename );

int           write_rec2d_file( char *filename, logtools_rec2_data_t rec );

int           write_data2d_file( char * filename, logtools_rec2_data_t rec );

int           rec2_parse_line( char *line, logtools_rec2_data_t *rec, int alloc, int verbose );

int           carmen_parse_line( char *line, logtools_rec2_data_t *rec, int alloc, int mode );

int           moos_parse_line( char *line, logtools_rec2_data_t *rec, int alloc, int mode );

int           player_parse_line( char *line, logtools_rec2_data_t *rec, int alloc, int mode );

int           saphira_parse_line( char *line, logtools_rec2_data_t *rec, int alloc, int mode );

int           placelab_parse_line( char *line, logtools_rec2_data_t *rec, int alloc, int mode );

int           read_script( char *filename, logtools_rec2_data_t *script, int verbose );

logtools_vector2_t  rotate_vector2( logtools_vector2_t p, double angle );

logtools_vector2_t  rotate_and_translate_vector2( logtools_vector2_t p,
						  double rot,
						  logtools_vector2_t trans );

double        vector2_length( logtools_vector2_t v1 );

double        vector2_distance( logtools_vector2_t p1, logtools_vector2_t p2 );

void          compute_rec2d_coordpts( logtools_rec2_data_t *rec );

logtools_vector2_t       compute_laser_abs_point( logtools_rpos2_t rpos, double val,
				       logtools_rmove2_t offset, double angle );

void          compute_vec2_convex_hull( logtools_vector2_t_SET pts,
					VECTORREF_SET *hull );

double        gauss_function( double x, double mu, double sigma );

GAUSS_KERNEL  compute_gauss_kernel( int length );

logtools_rmove2_t        compute_movement2_between_rpos2( logtools_rpos2_t s, logtools_rpos2_t e );

logtools_rpos2_t         compute_rpos2_with_movement2( logtools_rpos2_t start, logtools_rmove2_t move );

logtools_rpos2_t         compute_rpos2_backwards_with_movement2( logtools_rpos2_t start,
						      logtools_rmove2_t move );

double        convert_orientation_to_range( double angle );

double        compute_orientation_diff( double start, double end );

void          robot2map( logtools_rpos2_t pos, logtools_rpos2_t corr, logtools_rpos2_t *map );

void          map2robot( logtools_rpos2_t map, logtools_rpos2_t corr, logtools_rpos2_t *pos );

void          computeCorr( logtools_rpos2_t pos, logtools_rpos2_t map, logtools_rpos2_t *corr );

void          compute_forward_correction( logtools_rpos2_t pos, logtools_rpos2_t corr, logtools_rpos2_t *cpos );

void          compute_backward_correction( logtools_rpos2_t cpos, logtools_rpos2_t corr, logtools_rpos2_t *pos );

void          compute_correction_parameters( logtools_rpos2_t pos, logtools_rpos2_t cpos, logtools_rpos2_t *corr );

void          update_correction_parameters( logtools_rpos2_t cpos, logtools_rpos2_t delta, logtools_rpos2_t *corr );


double        double_time( struct timeval time );

void          convert_time( double tval, struct timeval *time );

int           timeCompare ( struct timeval time1, struct timeval time2 );

double        timeDiff( struct timeval  t1, struct timeval t2);

double        get_user_time( void );


double        sick_laser_fov( int numbeams );

double        sick_laser_angle_diff( int num, double fov );

double        kl_distance( GAUSSIAN g1, GAUSSIAN g2 );

double        rpos2_distance( logtools_rpos2_t p1, logtools_rpos2_t p2 );

char *        extended_filename( char * filename );

char *        printable_filename( char * filename );

FILE *        fopen_filename( char * filename, char * mode );

char *        str_get_valstr( char *str );

char *        str_get_str( char *str );

int           str_get_numbers( char *str, int num, ... );

double        gps_degree_abs_decimal( double degree_minute );

double        gps_degree_decimal( double degree_minute, char orient );

logtools_utm_coord_t     ll2utm( logtools_ll_coord_t ll );

logtools_ll_coord_t      utm2ll( logtools_utm_coord_t utm );

#endif /* ifdef BD_UTILS_H */

