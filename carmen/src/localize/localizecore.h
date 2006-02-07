/*********************************************************
 *
 * This source code is part of the Carnegie Mellon Robot
 * Navigation Toolkit (CARMEN)
 *
 * CARMEN Copyright (c) 2002 Michael Montemerlo, Nicholas
 * Roy, and Sebastian Thrun
 *
 * CARMEN is free software; you can redistribute it and/or 
 * modify it under the terms of the GNU General Public 
 * License as published by the Free Software Foundation; 
 * either version 2 of the License, or (at your option)
 * any later version.
 *
 * CARMEN is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied 
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 * PURPOSE.  See the GNU General Public License for more 
 * details.
 *
 * You should have received a copy of the GNU General 
 * Public License along with CARMEN; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place, 
 * Suite 330, Boston, MA  02111-1307 USA
 *
 ********************************************************/

#ifndef CARMEN_LOCALIZECORE_H
#define CARMEN_LOCALIZECORE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <carmen/map.h>
#include "localize_motion.h"

#define      SMALL_PROB        0.01

/* localize parameter structure */

typedef struct {
  double front_laser_offset, rear_laser_offset;
  int num_particles;
  double max_range, min_wall_prob, outlier_fraction;
  double update_distance;
  double integrate_angle; /* used to compute laser_skip*/
  int laser_skip;
  int use_rear_laser, do_scanmatching;
  int constrain_to_map;
#ifdef OLD_MOTION_MODEL
  double odom_a1, odom_a2, odom_a3, odom_a4;
#endif
  double occupied_prob;
  double lmap_std;
  double global_lmap_std, global_evidence_weight, global_distance_threshold;
  int global_test_samples;
  int use_sensor;
#ifndef OLD_MOTION_MODEL
  carmen_localize_motion_model_t *motion_model;
#endif  
} carmen_localize_param_t, *carmen_localize_param_p;

typedef struct {
  float x, y, theta, weight;
} carmen_localize_particle_t, *carmen_localize_particle_p;

typedef struct {
  int initialized, first_odometry, global_mode;
  carmen_localize_param_p param;
  carmen_localize_particle_p particles;
  carmen_point_t last_odometry_position;
  float **temp_weights;
  float distance_travelled;
  char laser_mask[361];
} carmen_localize_particle_filter_t, *carmen_localize_particle_filter_p;

typedef struct {
  float x, y, range;
  float prob;
  char mask;
} carmen_localize_laser_point_t, *carmen_localize_laser_point_p;

typedef struct {
  carmen_point_t mean, std;
  carmen_point_t odometry_pos;
  double xy_cov;
  int converged;
  int num_readings;
  carmen_localize_laser_point_t mean_scan[361];
} carmen_localize_summary_t, *carmen_localize_summary_p;

#include "likelihood_map.h"

carmen_localize_particle_filter_p carmen_localize_particle_filter_new(carmen_localize_param_p 
							param);

void carmen_localize_initialize_particles_uniform(carmen_localize_particle_filter_p filter,
					   carmen_robot_laser_message *laser,
					   carmen_localize_map_p map);

void carmen_localize_initialize_particles_gaussians(carmen_localize_particle_filter_p filter,
					     carmen_localize_map_p map, int num_modes,
					     carmen_point_t *mean,
					     carmen_point_t *std);

void carmen_localize_initialize_particles_gaussian(carmen_localize_particle_filter_p filter,
					    carmen_localize_map_p map, 
					    carmen_point_t mean, 
					    carmen_point_t std);

void carmen_localize_initialize_particles_manual(carmen_localize_particle_filter_p filter,
					  double *x, double *y, double *theta,
					  double *weight, int num_particles);

int carmen_localize_initialize_particles_placename(carmen_localize_particle_filter_p filter,
						    carmen_localize_map_p map,
						    carmen_map_placelist_p placelist,
						    char *placename);

void carmen_localize_incorporate_odometry(carmen_localize_particle_filter_p filter,
				   carmen_point_t odometry_position);

void carmen_localize_incorporate_laser(carmen_localize_particle_filter_p filter,
				       carmen_localize_map_p map, int num_readings, 
				       float *range, double forward_offset, 
				       double angular_resolution,
				       double laser_maxrange,
				       double first_beam_angle,
				       int backwards);

void carmen_localize_resample(carmen_localize_particle_filter_p filter);

void carmen_localize_run(carmen_localize_particle_filter_p filter, carmen_localize_map_p map,
		  carmen_robot_laser_message *laser, double forward_offset,
		  int backwards);

void carmen_localize_laser_scan_gd(int num_readings, float *range,
				   double angular_resolution,
				   double first_beam_angle,
				   carmen_point_p laser_pos, double forward_offset,
				   carmen_localize_map_p map, int laser_skip);

void carmen_localize_summarize(carmen_localize_particle_filter_p filter, 
			       carmen_localize_summary_p summary, 
			       carmen_localize_map_p map,
			       int num_readings, float *range, double forward_offset,
			       double angular_resolution,
			       double first_beam_angle,
			       int backwards);

#ifdef __cplusplus
}
#endif

#endif
