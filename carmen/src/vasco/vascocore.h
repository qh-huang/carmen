/** @addtogroup vasco libvascocore **/
// @{

/** 
 * \file vascocore.h 
 * \brief Library for scan-matching. 
 *
 * Libfrary for the CARMEN mapper. 
 **/

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


typedef struct {

  int                                    verbose;
  
  double                                 max_usable_laser_range;
  
  double                                 local_map_max_range;
  double                                 local_map_resolution;
  int                                    local_map_kernel_len;
  int                                    local_map_use_odometry;
  int                                    local_map_num_convolve;
  double                                 local_map_std_val;
  int                                    local_map_history_length;
  int                                    local_map_max_used_history;
  double                                 local_map_min_bbox_distance;
  double                                 local_map_object_prob;
  int                                    local_map_use_last_scans;

  double                                 bounding_box_max_range;
  double                                 bounding_box_border;

  double                                 motion_model_forward;
  double                                 motion_model_sideward;
  double                                 motion_model_rotation;
  
  double                                 pos_corr_step_size_forward;
  double                                 pos_corr_step_size_sideward;
  double                                 pos_corr_step_size_rotation;
  int                                    pos_corr_step_size_loop;
  
} carmen_vascocore_param_t, *carmen_vascocore_param_p;


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
void vascocore_init_no_ipc(carmen_vascocore_param_t *new_settings);

void           vascocore_reset();

carmen_point_t vascocore_scan_match( carmen_laser_laser_message scan,
				     carmen_point_t pos );

carmen_point_t
vascocore_scan_match_general(int num_readings, float *range, float *angle,
			     double fov,
			     carmen_point_t pos, int first);

#endif /* ifdef BD_UTILS_H */

// @}
