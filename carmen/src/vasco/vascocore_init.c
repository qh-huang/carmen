#include <carmen/carmen.h>

#include "vascocore.h"
#include "vascocore_intern.h"

carmen_vascocore_param_t      carmen_vascocore_settings;
carmen_vascocore_map_t        carmen_vascocore_map;
carmen_vascocore_history_t    carmen_vascocore_history;

void
vascocore_get_params( int argc, char **argv, carmen_vascocore_param_p param )
{
  carmen_param_t param_list[] = {
    
    {"vascocore", "verbose", CARMEN_PARAM_ONOFF, 
     &param->verbose, 0, NULL},
    
    {"vascocore", "max_usable_laser_range", CARMEN_PARAM_DOUBLE, 
     &param->max_usable_laser_range, 0, NULL},
    
    {"vascocore", "local_map_max_range", CARMEN_PARAM_DOUBLE, 
     &param->local_map_max_range, 0, NULL},
    {"vascocore", "local_map_resolution", CARMEN_PARAM_DOUBLE, 
     &param->local_map_resolution, 0, NULL},
    {"vascocore", "local_map_kernel_len", CARMEN_PARAM_INT, 
     &param->local_map_kernel_len, 0, NULL},
    {"vascocore", "local_map_use_odometry", CARMEN_PARAM_ONOFF, 
     &param->local_map_use_odometry, 0, NULL},
    {"vascocore", "local_map_num_convolve", CARMEN_PARAM_INT, 
     &param->local_map_num_convolve, 0, NULL},
    {"vascocore", "local_map_std_val", CARMEN_PARAM_DOUBLE, 
     &param->local_map_std_val, 0, NULL},
    {"vascocore", "local_map_history_length", CARMEN_PARAM_INT, 
     &param->local_map_history_length, 0, NULL},
    {"vascocore", "local_map_max_used_history", CARMEN_PARAM_INT, 
     &param->local_map_max_used_history, 0, NULL},
    {"vascocore", "local_map_min_bbox_distance", CARMEN_PARAM_DOUBLE, 
     &param->local_map_min_bbox_distance, 0, NULL},
    {"vascocore", "local_map_use_last_scans", CARMEN_PARAM_INT,
     &param->local_map_use_last_scans, 0, NULL},
    {"vascocore", "local_map_object_prob", CARMEN_PARAM_DOUBLE, 
     &param->local_map_object_prob, 0, NULL},

    {"vascocore", "bounding_box_max_range", CARMEN_PARAM_DOUBLE, 
     &param->bounding_box_max_range, 0, NULL},
    {"vascocore", "bounding_box_border", CARMEN_PARAM_DOUBLE, 
     &param->bounding_box_border, 0, NULL},

    {"vascocore", "motion_model_forward", CARMEN_PARAM_DOUBLE, 
     &param->motion_model_forward, 0, NULL},
    {"vascocore", "motion_model_sideward", CARMEN_PARAM_DOUBLE, 
     &param->motion_model_sideward, 0, NULL},
    {"vascocore", "motion_model_rotation", CARMEN_PARAM_DOUBLE, 
     &param->motion_model_rotation, 0, NULL},

    {"vascocore", "pos_corr_step_size_forward", CARMEN_PARAM_DOUBLE, 
     &param->pos_corr_step_size_forward, 0, NULL},
    {"vascocore", "pos_corr_step_size_sideward", CARMEN_PARAM_DOUBLE, 
     &param->pos_corr_step_size_sideward, 0, NULL},
    {"vascocore", "pos_corr_step_size_rotation", CARMEN_PARAM_DOUBLE, 
     &param->pos_corr_step_size_rotation, 0, NULL},
    {"vascocore", "pos_corr_step_size_loop", CARMEN_PARAM_INT, 
     &param->pos_corr_step_size_loop, 0, NULL},
    
  };

  carmen_param_install_params(argc, argv, param_list, 
			      sizeof(param_list) / sizeof(param_list[0]));

#ifdef DEBUG
    fprintf( stderr, "vascocore_verbose                       %d\n",
	     param->verbose );
    fprintf( stderr, "vascocore_max_usable_laser_range        %f\n",
	     param->max_usable_laser_range );
    fprintf( stderr, "vascocore_local_map_max_range           %f\n", 
	     param->local_map_max_range );
    fprintf( stderr, "vascocore_local_map_resolution          %f\n", 
	     param->local_map_resolution );
    fprintf( stderr, "vascocore_local_map_kernel_len          %d\n", 
	     param->local_map_kernel_len );
    fprintf( stderr, "vascocore_local_map_use_odometry        %d\n", 
	     param->local_map_use_odometry );
    fprintf( stderr, "vascocore_local_map_num_convolve        %d\n", 
	     param->local_map_num_convolve );
    fprintf( stderr, "vascocore_local_map_std_val             %f\n", 
	     param->local_map_std_val );
    fprintf( stderr, "vascocore_local_map_history_length      %d\n", 
	     param->local_map_history_length );
    fprintf( stderr, "vascocore_local_map_max_used_history    %d\n", 
	     param->local_map_max_used_history );
    fprintf( stderr, "vascocore_local_map_min_bbox_distance   %f\n", 
	     param->local_map_min_bbox_distance );
    fprintf( stderr, "vascocore_local_map_use_last_scans      %d\n", 
	     param->local_map_use_last_scans );
    fprintf( stderr, "vascocore_local_map_object_prob         %f\n", 
	     param->local_map_object_prob );
    fprintf( stderr, "vascocore_bounding_box_max_range        %f\n", 
	     param->bounding_box_max_range );
    fprintf( stderr, "vascocore_bounding_box_border           %f\n", 
	     param->bounding_box_border );
    fprintf( stderr, "vascocore_motion_model_forward          %f\n", 
	     param->motion_model_forward );
    fprintf( stderr, "vascocore_motion_model_sideward         %f\n", 
	     param->motion_model_sideward );
    fprintf( stderr, "vascocore_motion_model_rotation         %f\n", 
	     param->motion_model_rotation );
    fprintf( stderr, "vascocore_pos_corr_step_size_forward    %f\n", 
	     param->pos_corr_step_size_forward );
    fprintf( stderr, "vascocore_pos_corr_step_size_sideward   %f\n", 
	     param->pos_corr_step_size_sideward );
    fprintf( stderr, "vascocore_pos_corr_step_size_rotation   %f\n", 
	     param->pos_corr_step_size_rotation );
    fprintf( stderr, "vascocore_pos_corr_step_size_loop       %d\n", 
	     param->pos_corr_step_size_loop );
#endif
}

void
vascocore_initialize_maps( carmen_vascocore_map_t *local_map  )
{
  int                  size_x, size_y;
  carmen_point_t       npos = { 0.0, 0.0, 0.0 };
  if (carmen_vascocore_settings.verbose) {
    fprintf( stderr, "***************************************\n" );
    fprintf( stderr, "*        MAPS\n" );
    fprintf( stderr, "***************************************\n" );
  }
  size_x = (int) ceil((carmen_vascocore_settings.local_map_max_range)/
		      carmen_vascocore_settings.local_map_resolution);
  size_y = (int) ceil((carmen_vascocore_settings.local_map_max_range)/
		      carmen_vascocore_settings.local_map_resolution);
  if (carmen_vascocore_settings.verbose) {
    fprintf( stderr, "* INFO: create -local- map: %d x %d\n",
	     2*size_x, size_y );
  }
  initialize_map( local_map, 2*size_x, size_y, 60, size_y/2,
		  carmen_vascocore_settings.local_map_resolution, npos );
  if (carmen_vascocore_settings.verbose) {
    fprintf( stderr, "***************************************\n" );
  }
}


void
vascocore_alloc_history( carmen_vascocore_history_t * history )
{
  int i, j, nh;
 
  nh = carmen_vascocore_settings.local_map_history_length;
  nh = (nh>0?nh:1);
    
  history->data =
    (carmen_vascocore_extd_laser_t *)
    malloc(nh * sizeof(carmen_vascocore_extd_laser_t));
  carmen_test_alloc(history->data);
  
  for (i=0; i<nh; i++) {
    history->data[i].estpos.x     = 0;
    history->data[i].estpos.y     = 0;
    history->data[i].estpos.theta = 0;
    history->data[i].time         = 0;
    history->data[i].bbox.min.x   = 0;
    history->data[i].bbox.min.y   = 0;
    history->data[i].bbox.max.x   = 0;
    history->data[i].bbox.max.y   = 0;
    history->data[i].numvalues    = 0;
    history->data[i].val        =
      (double *) malloc( MAX_NUM_LASER_VALUES * sizeof(double) );
    carmen_test_alloc(history->data[i].val);
    history->data[i].angle      =
      (double *) malloc( MAX_NUM_LASER_VALUES * sizeof(double) );
    carmen_test_alloc(history->data[i].angle);
    history->data[i].coord      =
      (carmen_vec2_t *) malloc( MAX_NUM_LASER_VALUES * sizeof(carmen_vec2_t) );
    carmen_test_alloc(history->data[i].coord);
    for (j=0; j<MAX_NUM_LASER_VALUES; j++) {
      history->data[i].coord[j].x  = 0;
      history->data[i].coord[j].y  = 0;
      history->data[i].angle[j]    = 0;
      history->data[i].val[j]      = 0;
    }
  }
  history->length = nh;
  history->ptr    = 0;
}

void
vascocore_init( int argc, char **argv )
{
  vascocore_get_params( argc, argv, &carmen_vascocore_settings );
  vascocore_initialize_maps( &carmen_vascocore_map );
  vascocore_alloc_history( &carmen_vascocore_history );
}

void
vascocore_init_no_ipc(carmen_vascocore_param_t *new_settings)
{
  carmen_vascocore_settings = *new_settings;
  vascocore_initialize_maps( &carmen_vascocore_map );
  vascocore_alloc_history( &carmen_vascocore_history );
}

void
vascocore_reset()
{
  carmen_vascocore_history.ptr = 0;
}
