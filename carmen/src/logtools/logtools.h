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


#define INSIDE   0
#define OUTSIDE  1

typedef struct {
  double        x;
  double        y;
  double        z;
} VECTOR3;

typedef struct {
  double        q0;
  double        q1;
  double        q2;
  double        q3;
} QUATERNION;

typedef struct {
  int           numvectors;
  VECTOR3     * vec;
} VECTOR3_SET;

#define POINT3 VECTOR3

typedef struct {
  int           numpoints;
  POINT3      * p;
} POINT3_SET;

typedef struct {
  int           x;
  int           y;
  int           z;
} iVECTOR3;

typedef struct {
  int           numvectors;
  iVECTOR3    * vec;
} iVECTOR3_SET;

#define iPOINT3 iVECTOR3

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
  VECTOR3       origin;
  VECTOR3       direction;
} RAY3;

typedef struct {
  VECTOR3          origin;
  VECTOR3          edge0;
  VECTOR3          edge1;
} TRIANGLE_EDGE3;

typedef struct {
  VECTOR3          point0;
  VECTOR3          point1;
  VECTOR3          point2;
} TRIANGLE3;

typedef struct {
  int              numtriangles;
  TRIANGLE3      * triangle;
} TRIANGLE3_SET;

typedef struct {
  /* Ax + By + CZ + D = 0 */
  double      a;
  double      b;
  double      c;
  double      d;
} PLANE_PARAMS;


typedef struct {
  VECTOR3     point0;
  VECTOR3     point1;
} LINE3;

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
  double        forward;
  double        sideward;
  double        upward;
} MOVEMENT3; 

typedef struct {
  MOVEMENT3     trans;
  VECTOR3       rot;
} RMOVE3;

typedef struct {
  double        tv;
  double        rv;
} RVEL2;

typedef struct {
  VECTOR3       pos;
  VECTOR3       rot;
} RPOS3;

typedef struct {
  struct timeval      time;
  RPOS3               rpos;
} POSSENS3_DATA;

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
  /*
    p[0] = xmin, ymin, zmin
    p[1] = xmin, ymax, zmin
    p[2] = xmax, ymax, zmin
    p[3] = xmax, ymin, zmin
    p[4] = xmin, ymin, zmax
    p[5] = xmin, ymax, zmax
    p[6] = xmax, ymax, zmax
    p[7] = xmax, ymin, zmax
  */
  VECTOR3             min;
  VECTOR3             max;
  VECTOR3             p[8];
} BOUNDING_BOX3;

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
  VECTOR3             offset;   /* offset to robot center */
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
  
  /* positions */
  int                 numpositions3d;
  POSSENS3_DATA    *  psens3d;
  
  /* corrected positions */
  int                 numcpositions;
  POSSENS2_DATA     * cpsens;
  
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
  
  int                 numrfid;
  RFID_DATA        *  rfid;
  
  int                 numposcorr;
  POS_CORR2_DATA    * poscorr;
  
  REC2_INFO           info;
  
} REC2_DATA;

typedef struct {
  double              pan;
  double              tilt;
} DECL_TYPE;

typedef struct {
  VECTOR3             relpt;
  VECTOR3             abspt;
  int                 tag;
  int                 info;
} LASER_COORD3;

typedef struct {
  VECTOR3             laseroffset;
  DECL_TYPE           declination;
  LASERSENS2_DATA   * data;
  VECTOR3             origin;
  LASER_COORD3      * coord;
  DISTRIBUTION      * dynamic;
} LASERSENS3_DATA;

typedef struct {
  double              axisdist;          /* distance between tilt axis and
					    laserrangefinder center */
  MOVEMENT3           displace;          /* rel. position of the tilt axis
					    to the center of the robot */
} PANTILT_SETTINGS;

enum HARDWARE_TYPE { AMTEC_PANTILT };

typedef struct {
  PANTILT_SETTINGS    amtec_pantilt;
} HARDWARE_CONF;
  
typedef struct {
  RPOS3               estpos;
  enum HARDWARE_TYPE  hw_type;
  HARDWARE_CONF     * settings;
  int                 numswscans;
  LASERSENS3_DATA   * swscan;
} LASERSWEEP3_DATA;

typedef struct {
  int                 numsweeps;
  LASERSWEEP3_DATA  * sweep;
  int                 numscans;
  LASERSENS2_DATA   * scan;
} LASERSCAN3_DATA;


typedef struct {
  struct timeval      time;
  DECL_TYPE           pos;
} AMTEC_POSITION;

enum CAMERA3_SCAN_TYPE { CAMERA3_RANGE, CAMERA3_IMAGE, CAMERA3_POINTS };

typedef struct {
  
  struct timeval      time;
  RPOS3               estpos;
  int                 width;
  int                 height;
  float             * range;
  float             * intensity;
  VECTOR3           * points;
  enum CAMERA3_SCAN_TYPE type;
  
} CAMERA3_DATA;

typedef struct {
  HARDWARE_CONF       hardware;
  double              space;
} REC3_SETTINGS;

typedef struct {

  /* settings */
  REC3_SETTINGS       settings;

  /* entries */
  int                 numentries;
  ENTRY_POSITION    * entry;  

  /* positions */
  int                 numpositions;
  POSSENS3_DATA     * psens;

  /* corrected positions */
  int                 numcpositions;
  POSSENS3_DATA     * cpsens;

  /* amtec positions */
  int                 numamtecpos;
  AMTEC_POSITION    * amtec;

  /* laser scans */
  LASERSCAN3_DATA     lsens;

  /* laser scans */
  int                 numcamera3d;
  CAMERA3_DATA      * camera3d;

  int                 numcompass;
  COMPASS3_DATA     * compass;

  int                 numgps;
  GPS_DATA          * gps;

} REC3_DATA;

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
  int                numpoints;
  int              * planept;
  PLANE_PARAMS       param;
} PLANE_POINTREF;

typedef struct {
  int                sweep;
  int                scan;
  int                beam;
} BEAMREF;

typedef struct {
  int                numpoints;
  BEAMREF          * planebeam;
  PLANE_PARAMS       param;
} PLANE_BEAMREF;

typedef struct {
  int                numpoints;
  int              * plane;
} POINT_PLANEREF;

typedef struct {
  int                numpoints;
  POINT3           * point;
  int                numpolygons;
  POLYGON_REF      * polygon;
  int                numplanes;
  PLANE_POINTREF   * plane;
} SPF_POINTS_DATA;

typedef struct {
  int                numplanes;
  PLANE_BEAMREF    * plane;
} PLANE_BEAM_DATA;

typedef struct {
  BEAMREF            ref0;
  BEAMREF            ref1;
  BEAMREF            ref2;
} TRIANGLE3_BEAMREF;

typedef struct {
  int                pt0;
  int                pt1;
  int                pt2;
} TRIANGLEREF;

typedef struct {
  int                 numtriangles;
  TRIANGLE3_BEAMREF * triangle;
} TRIANGLE3_BEAMREF_SET;

typedef struct {
  int                 numtriangles;
  TRIANGLEREF       * triangle;
} TRIANGLEREF_SET;

typedef struct {
  int                 numlines;
  LINE3             * line;
} LINE3_SET;

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
  double    y;
  double    u;
  double    v;
} YUV;

typedef struct {
  double    r;
  double    g;
  double    b;
} RGB;

typedef struct {
  double    h;
  double    s;
  double    v;
} HSV;

typedef struct {
  double    dx;
  double    dy;
  double    dz;
  double    droll;
  double    dyaw;
  double    dpitch;
  int       from;
  int       to;
} LINK;

typedef struct {
  int       id;
  double    x;
  double    y;
  double    z;
  double    roll;
  double    yaw;
  double    pitch;
  int       numlinks;
  LINK    * lnk;
} NODE;

typedef struct {
  int       numnodes;
  NODE    * nodes;
} MESH_DATA;

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

int           timeCompare ( struct timeval time1, struct timeval time2 );

double        timeDiff( struct timeval  t1, struct timeval t2);

double        get_user_time( void );

void *        mdalloc(int ndim, int width, ...);

void          mdfree(void *tip, int ndim);

int           load_rec2d_file( char *filename, REC2_DATA *rec,
			       enum FILE_TYPE type, int mode );

int           read_rec2d_file( char *filename, REC2_DATA *rec, int mode );

int           read_data2d_file( REC2_DATA * rec, char * filename );

int           write_rec2d_file( char *filename, REC2_DATA rec );

int           write_data2d_file( char * filename, REC2_DATA rec );

int           rec2_parse_line( char *line, REC2_DATA *rec, int alloc, int verbose );

int           rec3_parse_line( char *line, REC3_DATA *rec, int alloc, int mode );

int           carmen_parse_line( char *line, REC2_DATA *rec, int alloc, int mode );

int           moos_parse_line( char *line, REC2_DATA *rec, int alloc, int mode );

int           player_parse_line( char *line, REC2_DATA *rec, int alloc, int mode );

int           saphira_parse_line( char *line, REC2_DATA *rec, int alloc, int mode );

int           placelab_parse_line( char *line, REC2_DATA *rec, int alloc, int mode );

int           read_rec3d_file( char *filename, REC3_DATA *rec, int verbose  );

int           write_rec3d_file( char *filename, REC3_DATA rec );

int           read_script( char *filename, REC2_DATA *script, int verbose );

int           read_beam_planes( char *filename, PLANE_BEAM_DATA *planes,
				int verbose );

void          init_spf_point_data( SPF_POINTS_DATA *spf );

int           insert_spf_point_data( char *filename, SPF_POINTS_DATA *spf  );

void          spf_get_component( SPF_POINTS_DATA spf, POINTREF_SET component,
				 SPF_POINTS_DATA * spf_comp );

void          spf_filter_small_components( SPF_POINTS_DATA spf,
					   int num_components, POINTREF_SET *components,
					   int min_comp_size, SPF_POINTS_DATA * spf_comp );

void          spf_write_data( SPF_POINTS_DATA spf, char * filename );

void          insert_smf_data( char *filename, SPF_POINTS_DATA *spf  );

VECTOR3       translate_and_rotate_vector3( VECTOR3 p, VECTOR3 rot,
					    VECTOR3 trans );

VECTOR3       rotate_and_translate_vector3( VECTOR3 p, VECTOR3 rot,
					    VECTOR3 trans );

VECTOR3       rotate_vector3( VECTOR3 p, VECTOR3 rot );

VECTOR3       rotate3_x( VECTOR3 p, double rot );

VECTOR3       rotate3_y( VECTOR3 p, double rot );

VECTOR3       rotate3_z( VECTOR3 p, double rot );

VECTOR2       rotate_vector2( VECTOR2 p, double angle );

VECTOR2       rotate_and_translate_vector2( VECTOR2 p, double rot,
					    VECTOR2 trans );

double        vector2_length( VECTOR2 v1 );

double        vector3_length( VECTOR3 v1 );

double        vector2_distance( VECTOR2 p1, VECTOR2 p2 );

double        vector3_distance( VECTOR3 v1, VECTOR3 v2 );

VECTOR3       vector3_add( VECTOR3 v1, VECTOR3 v2 );

VECTOR3       vector3_diff( VECTOR3 v1, VECTOR3 v2 );

VECTOR3       vector3_cross( VECTOR3 v1, VECTOR3 v2 );

VECTOR3       vector3_scalar_mult( VECTOR3 v1, double m );

double        vector3_dot_mult( VECTOR3 v1, VECTOR3 v2 );


void          compute_alloc_rec3_coordpts( REC3_DATA *rec );

int           get_alloc_rec3_size( REC3_DATA *rec );

void          compute_rel_rec3_coordpts( REC3_DATA *rec );

void          compute_abs_rec3_coordpts( REC3_DATA *rec );

void          compute_abs_scan_coordpts( LASERSENS3_DATA *data );

void          compute_rec2d_coordpts( REC2_DATA *rec );

VECTOR2       compute_laser_abs_point( RPOS2 rpos, double val,
				       RMOVE2 offset, double angle );

void          compute_estpos( REC3_DATA *rec );

void          compute_rec3_triangles( REC3_DATA rec,
				      TRIANGLE3_BEAMREF_SET *tri,
				      double minsize, double maxsize,
				      double maxrange );

void          compute_vec2_convex_hull( VECTOR2_SET pts,
					VECTORREF_SET *hull );

void          init_bounding_box( BOUNDING_BOX3 *box );

void          update_bounding_box( VECTOR3 v, BOUNDING_BOX3 *box );

void          compute_bounding_box( VECTOR3_SET vset, BOUNDING_BOX3 *box );

void          compute_bounding_box_with_min_max( VECTOR3 min, VECTOR3 max,
						 BOUNDING_BOX3 *box );

void          compute_bounding_box_plus_dist( VECTOR3_SET vset,
					      BOUNDING_BOX3 *box, double dist);

double        compute_triangle3_area( TRIANGLE3 tri );


double        distance_vec3_to_plane( PLANE_PARAMS pl, VECTOR3 p );

double        distance_vec3_to_line3( VECTOR3 p, LINE3 l );

double        distance_vec2_to_line2( VECTOR2 p, LINE2 l );

double        compute_factor_to_line( VECTOR3 p, LINE3 l );

VECTOR3       projection_vec3_to_line( VECTOR3 p, LINE3 l );

VECTOR2       projection_vec3_to_vec2( PLANE_PARAMS pl, VECTOR3 pt );

void          projection_vec3set_to_vec2set( PLANE_PARAMS pl, VECTOR3_SET pts,
					     VECTOR2_SET *pset );

VECTOR3       projection_vec3_to_plane( PLANE_PARAMS pl, VECTOR3 p );

void          projection_vec3set_to_plane( PLANE_PARAMS pl, VECTOR3_SET pts,
					    VECTOR3_SET *pset );

PLANE_PARAMS  compute_plane_from_three_vec3( VECTOR3 p1, VECTOR3 p2,
					     VECTOR3 p3 );

int           intersection_two_planes( PLANE_PARAMS pl1, PLANE_PARAMS pl2,
				       LINE3* L );

double        gauss_function( double x, double mu, double sigma );

GAUSS_KERNEL  compute_gauss_kernel( int length );
/*
  int           compute_alpha_shape( VECTOR2_SET vset, int alpha, EDGEREF_SET *e_set );
*/

RMOVE2        compute_movement2_between_rpos2( RPOS2 s, RPOS2 e );

RPOS2         compute_rpos2_with_movement2( RPOS2 start, RMOVE2 move );

RPOS2         compute_rpos2_backwards_with_movement2( RPOS2 start,
						      RMOVE2 move );

double        convert_orientation_to_range( double angle );

double        compute_orientation_diff( double start, double end );

void          robot2map( RPOS2 pos, RPOS2 corr, RPOS2 *map );

void          map2robot( RPOS2 map, RPOS2 corr, RPOS2 *pos );

void          computeCorr( RPOS2 pos, RPOS2 map, RPOS2 *corr );

VECTOR3       compute_triangle_normal( TRIANGLE3 tri );

double        angle_between_two_vec3( VECTOR3 v1, VECTOR3 v2 );

LINE2_LSQFT   compute_lsqf_line( VECTOR2_SET p );


YUV           convert_from_rgb( RGB color );

RGB           convert_from_yuv( YUV color );

RGB           hsv_to_rgb( HSV color );

RGB           val_to_rgb( double val );

RGB           val_to_gray( double val );


void          compute_forward_correction( RPOS2 pos, RPOS2 corr, RPOS2 *cpos );

void          compute_backward_correction( RPOS2 cpos, RPOS2 corr, RPOS2 *pos );

void          compute_correction_parameters( RPOS2 pos, RPOS2 cpos, RPOS2 *corr );

void          update_correction_parameters( RPOS2 cpos, RPOS2 delta, RPOS2 *corr );

QUATERNION    quaternions_conjugate( QUATERNION q );

VECTOR3       quaternions_convert_to_euler( QUATERNION q );

QUATERNION    quaternions_create_from_euler( VECTOR3 a );

double        quaternions_magnitude( QUATERNION q );

QUATERNION    quaternions_multiply( QUATERNION q1, QUATERNION q2 );

void          mesh_from_spf( SPF_POINTS_DATA scan, MESH_DATA * mesh );

int           mesh_components( MESH_DATA mesh, POINTREF_SET ** components );

typedef void (*draw_function_t)(int x,int y);

void          draw_set_function( draw_function_t funct );

void          draw_ellipse( iPOINT2 p, const int a,const int b);

void          draw_line( iPOINT2 p1, iPOINT2 p2 );
     
void          draw_circle( iPOINT2 p, const int r );

void          draw_filled_circle( iPOINT2 p, const int r );

void          convert_time( double tval, struct timeval *time );

double        double_time( struct timeval time );

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

