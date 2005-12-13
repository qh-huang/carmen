
#include "clfconvert.h"
#include "movement.h"

void carmen_clfconvert_transform_laser_message_laser_pose(carmen_point_t refpose_currentframe,
							  carmen_point_t refpose_newframe,
							  carmen_robot_laser_message* msg) {
  msg->laser_pose = 
    carmen_movement_transformation_between_frames(refpose_currentframe, 
						  refpose_newframe,
						  msg->laser_pose);
}


void carmen_clfconvert_transform_laser_message_odom_pose(carmen_point_t refpose_currentframe,
							 carmen_point_t refpose_newframe,
							 carmen_robot_laser_message* msg) {
  msg->robot_pose = 
    carmen_movement_transformation_between_frames(refpose_currentframe, 
						  refpose_newframe, 
						  msg->laser_pose);
}

void carmen_clfconvert_transform_laser_message(carmen_point_t refpose_currentframe,
					       carmen_point_t refpose_newframe,
					       carmen_robot_laser_message* msg) {


  carmen_clfconvert_transform_laser_message_laser_pose(refpose_currentframe,
						       refpose_newframe,
						       msg);


  carmen_clfconvert_transform_laser_message_odom_pose(refpose_currentframe,
						      refpose_newframe,
						      msg);
}

void carmen_clfconvert_transform_odometry_message(carmen_point_t refpose_currentframe,
						  carmen_point_t refpose_newframe,
						  carmen_base_odometry_message* msg) {

  carmen_point_t t;
  
  t.x = msg->x;
  t.y = msg->y;
  t.theta = msg->theta;
  t = carmen_movement_transformation_between_frames(refpose_currentframe, 
						    refpose_newframe, t);
  msg->x = t.x;
  msg->y = t.y;
  msg->theta = t.theta;
}

void carmen_clfconvert_transform_truepos_message(carmen_point_t refpose_currentframe,
						 carmen_point_t refpose_newframe,
						 carmen_simulator_truepos_message* msg) {

  msg->odometrypose =  carmen_movement_transformation_between_frames(refpose_currentframe,
								     refpose_newframe, 
								     msg->odometrypose);
}

