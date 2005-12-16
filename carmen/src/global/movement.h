#ifndef MOVEMENT_H
#define MOVEMENT_H

#ifdef __cplusplus
extern "C" {
#endif

#include <carmen/carmen.h>

/* rtr-movement (1st rotation, translation, 2nd rotation) */
typedef struct  {
  double r1;
  double d;
  double r2;
} carmen_movement_rtr_t;

/* fsr-movement (forward, sideward, rotate) */
typedef struct  {
  double f;
  double s;
  double r;
} carmen_movement_fsr_t;

/* ** Rotation-Translation-Rotation Transformations *********** */
  
carmen_movement_rtr_t 
carmen_movement_rtr_from_point(carmen_point_t pt);
  
carmen_point_t 
carmen_movement_point_from_rtr(carmen_movement_rtr_t move);

carmen_movement_rtr_t  
carmen_movement_invert_rtr(carmen_movement_rtr_t r);

carmen_point_t 
carmen_movement_rtr_move_point(carmen_point_t t, 
			       carmen_movement_rtr_t r);

carmen_movement_rtr_t  
carmen_movement_compose_rtr(carmen_movement_rtr_t r,
			    carmen_movement_rtr_t add) ;

carmen_movement_rtr_t  
carmen_movement_rtr_between_points(carmen_point_t t1, 
				   carmen_point_t t2);

/* ** Forward-Sideward-Rotation Transformations *********** */

carmen_movement_fsr_t 
carmen_movement_fsr_from_point(carmen_point_t pt);

carmen_point_t 
carmen_movement_point_from_fsr(carmen_movement_fsr_t move);

carmen_movement_fsr_t  
carmen_movement_invert_fsr(carmen_movement_fsr_t r);

carmen_point_t 
carmen_movement_fsr_move_point(carmen_point_t t, 
			       carmen_movement_fsr_t r);

carmen_movement_fsr_t  
carmen_movement_compose_fsr(carmen_movement_fsr_t r,
			    carmen_movement_fsr_t add) ;


carmen_movement_fsr_t  
carmen_movement_fsr_between_points(carmen_point_t t1, 
				   carmen_point_t t2);


/* ** Coversion between FSR and RTR Transformations *********** */

carmen_movement_rtr_t 
carmen_movement_fsr_to_rtr(carmen_movement_fsr_t move);

carmen_movement_fsr_t 
carmen_movement_rtr_to_fsr(carmen_movement_rtr_t move);

/* ** Transformation between coordinate frames ************ */


/* returns pt_frame1 in the coordinate frame2 */
carmen_point_t 
carmen_movement_transformation_between_frames(carmen_point_t reference_pt_frame1, 
					      carmen_point_t reference_pt_frame2,
					      carmen_point_t pt_frame1);
  

#ifdef __cplusplus
}
#endif


#endif

