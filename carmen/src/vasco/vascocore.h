#ifndef VASCOCORE_UTILS_H
#define VASCOCORE_UTILS_H

#ifndef _SYS_TIME_H
#include <sys/time.h>
#endif

#ifndef TRUE
#define TRUE  1
#endif

#ifndef FALSE
#define FALSE 0
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

typedef struct {

  double                    x;
  double                    y;

} carmen_vec2_t;

typedef struct {

  int                       x;
  int                       y;
}
carmen_ivec2_t;

typedef struct {
  
  short                     x;
  short                     y;

} carmen_svec2_t;

typedef struct {
  
  double                    forward;
  double                    sideward;
  double                    rotation;

} carmen_move_t;

typedef struct {
  
  int                       len;
  double                  * val;

} carmen_gauss_kernel_t;

typedef struct {
  
  carmen_vec2_t             min;
  carmen_vec2_t             max;

} carmen_bbox_t;


void *         carmen_mdalloc(int ndim, int width, ...);

void           camen_mdfree(void *tip, int ndim);

double         carmen_vec_distance( carmen_vec2_t p1, carmen_vec2_t p2 );

double         carmen_vec_length( carmen_vec2_t v1 );

double         carmen_point_dist( carmen_point_t pos1, carmen_point_t pos2 );

double         carmen_move_length( carmen_move_t move );

double         carmen_gauss( double x, double mu, double sigma );

carmen_gauss_kernel_t   carmen_gauss_kernel(int length );

double         carmen_orientation_diff( double start, double end );

carmen_vec2_t  carmen_laser_point( carmen_point_t rpos,
				   double val, double angle );

carmen_point_t carmen_point_with_move( carmen_point_t start,
				       carmen_move_t move );

carmen_point_t carmen_point_backwards_with_move( carmen_point_t start,
						 carmen_move_t move );

carmen_point_t carmen_point_from_move( carmen_move_t move );

carmen_point_t carmen_point_backwards_from_move( carmen_move_t move );

carmen_move_t  carmen_move_between_points( carmen_point_t start,
					   carmen_point_t end );

void           vascocore_init( int argc, char **argv );

void           vascocore_reset();

carmen_point_t vascocore_scan_match( carmen_laser_laser_message scan,
				     carmen_point_t pos );

carmen_point_t
vascocore_scan_match_general(int num_readings, float *range, float *angle,
			     carmen_point_t pos, int first);

#endif /* ifdef BD_UTILS_H */

