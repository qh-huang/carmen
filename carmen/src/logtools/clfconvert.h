#ifndef CLFCONVERT_H
#define CLFCONVERT_H

#ifdef __cplusplus
extern "C" {
#endif

#include <carmen/carmen.h>


void 
carmen_clfconvert_transform_laser_message(carmen_point_t refpose_currentframe,
					  carmen_point_t refpose_newframe,
					  carmen_robot_laser_message* msg);

void carmen_clfconvert_transform_laser_message_laser_pose(carmen_point_t refpose_currentframe,
							  carmen_point_t refpose_newframe,
							  carmen_robot_laser_message* msg);


void carmen_clfconvert_transform_laser_message_odom_pose(carmen_point_t refpose_currentframe,
							 carmen_point_t refpose_newframe,
							 carmen_robot_laser_message* msg);

void 
carmen_clfconvert_transform_odometry_message(carmen_point_t refpose_currentframe,
					     carmen_point_t refpose_newframe,
					     carmen_base_odometry_message* msg) ;

void 
carmen_clfconvert_transform_truepos_message(carmen_point_t refpose_currentframe,
					    carmen_point_t refpose_newframe,
					    carmen_simulator_truepos_message* msg);
  

#ifdef __cplusplus
}
#endif


#endif

