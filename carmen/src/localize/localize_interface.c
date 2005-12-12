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

#include <carmen/carmen.h>
#include "localize_messages.h"

static carmen_localize_globalpos_message **globalpos_message_pointer_internal = NULL;
static carmen_handler_t *globalpos_message_handler_internal = NULL;
static carmen_localize_particle_message **particle_pointer_internal = NULL;
static carmen_handler_t *particle_handler_internal = NULL;
static carmen_localize_sensor_message **sensor_pointer_internal = NULL;
static carmen_handler_t *sensor_handler_internal = NULL;
static carmen_localize_initialize_message **localize_initialize_internal = NULL;
static carmen_handler_t *localize_initialize_handler = NULL;

static unsigned int timeout = 5000;

static int context_array_size = 0;
static IPC_CONTEXT_PTR *context_array = NULL;

static int
get_context_id(void)
{
  int index = 0;
  IPC_CONTEXT_PTR current_context = IPC_getContext();

  if (context_array == NULL)
    return -1;

  while (context_array[index] != NULL) 
    {
      if (context_array[index] == current_context)
	return index;
      index++;
    }

  return -1;

}

static int 
add_context(void)
{
  int index;
  
  if (context_array == NULL)
    {
      context_array = (IPC_CONTEXT_PTR *)calloc(10, sizeof(IPC_CONTEXT_PTR));
      carmen_test_alloc(context_array);

      globalpos_message_pointer_internal = 
	(carmen_localize_globalpos_message **)
	calloc(10, sizeof(carmen_localize_globalpos_message *));
      carmen_test_alloc(globalpos_message_pointer_internal);

      particle_pointer_internal = (carmen_localize_particle_message **)
	calloc(10, sizeof(carmen_localize_particle_message *));
      carmen_test_alloc(particle_pointer_internal);

      sensor_pointer_internal = (carmen_localize_sensor_message **)
	calloc(10, sizeof(carmen_localize_sensor_message *));
      carmen_test_alloc(sensor_pointer_internal);

      localize_initialize_internal = (carmen_localize_initialize_message **)
	calloc(10, sizeof(carmen_localize_initialize_message *));
      carmen_test_alloc(localize_initialize_internal);

      globalpos_message_handler_internal = (carmen_handler_t *)
	calloc(10, sizeof(carmen_handler_t));
      carmen_test_alloc(globalpos_message_handler_internal);

      particle_handler_internal = (carmen_handler_t *)
	calloc(10, sizeof(carmen_handler_t));
      carmen_test_alloc(particle_handler_internal);

      sensor_handler_internal = (carmen_handler_t *)
	calloc(10, sizeof(carmen_handler_t));
      carmen_test_alloc(sensor_handler_internal);

      localize_initialize_handler = (carmen_handler_t *)
	calloc(10, sizeof(carmen_handler_t));
      carmen_test_alloc(localize_initialize_handler);

      context_array_size = 10;
      context_array[0] = IPC_getContext();

      return 0;
    }

  index = 0;
  while (index < context_array_size && context_array[index] != NULL) 
    index++;
  
  if (index == context_array_size) 
    {
      context_array_size += 10;
      context_array = (IPC_CONTEXT_PTR *)realloc
	(context_array, context_array_size*sizeof(IPC_CONTEXT_PTR));
      carmen_test_alloc(context_array);
      memset(context_array+index, 0, 10*sizeof(IPC_CONTEXT_PTR));
    }

  context_array[index] = IPC_getContext();
  return index;
}

static void 
globalpos_interface_handler(MSG_INSTANCE msgRef, BYTE_ARRAY callData,
                            void *clientData __attribute__ ((unused)))
{
  FORMATTER_PTR formatter;
  IPC_RETURN_TYPE err = IPC_OK;
  int context_id;
  
  context_id = get_context_id();

  if (context_id < 0) 
    {
      carmen_warn("Bug detected: invalid context\n");
      IPC_freeByteArray(callData);
      return;
    }

  formatter = IPC_msgInstanceFormatter(msgRef);
  if(globalpos_message_pointer_internal[context_id])
    err = IPC_unmarshallData(formatter, callData, 
			     globalpos_message_pointer_internal[context_id],
                             sizeof(carmen_localize_globalpos_message));
  IPC_freeByteArray(callData);
  carmen_test_ipc_return(err, "Could not unmarshall", 
			 IPC_msgInstanceName(msgRef));
  if(globalpos_message_handler_internal[context_id])
    globalpos_message_handler_internal[context_id]
      (globalpos_message_pointer_internal[context_id]);
}

void
carmen_localize_subscribe_globalpos_message(carmen_localize_globalpos_message
					    *globalpos,
					    carmen_handler_t handler,
					    carmen_subscribe_t subscribe_how)
{
  IPC_RETURN_TYPE err = IPC_OK;  
  int context_id;

  err = IPC_defineMsg(CARMEN_LOCALIZE_GLOBALPOS_NAME, 
		      IPC_VARIABLE_LENGTH, 
		      CARMEN_LOCALIZE_GLOBALPOS_FMT);
  carmen_test_ipc_exit(err, "Could not define message", 
		       CARMEN_LOCALIZE_GLOBALPOS_NAME);

  if(subscribe_how == CARMEN_UNSUBSCRIBE) 
    {
      IPC_unsubscribe(CARMEN_LOCALIZE_GLOBALPOS_NAME, 
		      globalpos_interface_handler);
      return;
    }

  context_id = get_context_id();
  if (context_id < 0)
    context_id = add_context();

  if(globalpos)
    globalpos_message_pointer_internal[context_id] = globalpos;
  else if(globalpos_message_pointer_internal[context_id] == NULL) 
    {
      globalpos_message_pointer_internal[context_id] = 
	(carmen_localize_globalpos_message *)
	calloc(1, sizeof(carmen_localize_globalpos_message));
      carmen_test_alloc(globalpos_message_pointer_internal[context_id]);
    }
  globalpos_message_handler_internal[context_id] = handler;
  err = IPC_subscribe(CARMEN_LOCALIZE_GLOBALPOS_NAME, 
		      globalpos_interface_handler, NULL);
  if(subscribe_how == CARMEN_SUBSCRIBE_LATEST)
    IPC_setMsgQueueLength(CARMEN_LOCALIZE_GLOBALPOS_NAME, 1);
  else
    IPC_setMsgQueueLength(CARMEN_LOCALIZE_GLOBALPOS_NAME, 100);
  carmen_test_ipc(err, "Could not subscribe", CARMEN_LOCALIZE_GLOBALPOS_NAME);
}

static void 
particle_interface_handler(MSG_INSTANCE msgRef, BYTE_ARRAY callData,
			   void *clientData __attribute__ ((unused)))
{
  FORMATTER_PTR formatter;
  IPC_RETURN_TYPE err = IPC_OK;
  int context_id;
  
  context_id = get_context_id();

  if (context_id < 0) 
    {
      carmen_warn("Bug detected: invalid context\n");
      IPC_freeByteArray(callData);
      return;
    }

  if(particle_pointer_internal[context_id] &&
     particle_pointer_internal[context_id]->particles != NULL)
    free(particle_pointer_internal[context_id]->particles);
  formatter = IPC_msgInstanceFormatter(msgRef);
  if(particle_pointer_internal[context_id])
    err = IPC_unmarshallData(formatter, callData,
			     particle_pointer_internal[context_id],
                             sizeof(carmen_localize_particle_message));
  IPC_freeByteArray(callData);
  carmen_test_ipc_return(err, "Could not unmarshall", 
			 IPC_msgInstanceName(msgRef));
  if(particle_handler_internal[context_id])
    particle_handler_internal[context_id]
      (particle_pointer_internal[context_id]);
}

void 
carmen_localize_subscribe_particle_message(carmen_localize_particle_message 
					   *particle, carmen_handler_t handler,
					   carmen_subscribe_t subscribe_how)
{
  IPC_RETURN_TYPE err;
  int context_id;
  
  err = IPC_defineMsg(CARMEN_LOCALIZE_PARTICLE_NAME, 
		      IPC_VARIABLE_LENGTH, 
		      CARMEN_LOCALIZE_PARTICLE_FMT);
  carmen_test_ipc_exit(err, "Could not define message", 
		       CARMEN_LOCALIZE_PARTICLE_NAME);

  if(subscribe_how == CARMEN_UNSUBSCRIBE) 
    {
      IPC_unsubscribe(CARMEN_LOCALIZE_PARTICLE_NAME, 
		      particle_interface_handler);
      return;
    }

  context_id = get_context_id();
  if (context_id < 0)
    context_id = add_context();

  if(particle) 
    {
      particle_pointer_internal[context_id] = particle;
      memset(particle_pointer_internal[context_id], 0, 
	     sizeof(carmen_localize_particle_message));
    }
  else if(particle_pointer_internal[context_id] == NULL) 
    {
      particle_pointer_internal[context_id] = 
	(carmen_localize_particle_message *)calloc
	(1, sizeof(carmen_localize_particle_message));
      carmen_test_alloc(particle_pointer_internal[context_id]);
    }
  particle_handler_internal[context_id] = handler;
  err = IPC_subscribe(CARMEN_LOCALIZE_PARTICLE_NAME, 
		      particle_interface_handler, NULL);
  carmen_test_ipc(err, "Could not subscribe", CARMEN_LOCALIZE_PARTICLE_NAME);
  if (subscribe_how == CARMEN_SUBSCRIBE_LATEST)
    IPC_setMsgQueueLength(CARMEN_LOCALIZE_PARTICLE_NAME, 1);
  else
    IPC_setMsgQueueLength(CARMEN_LOCALIZE_PARTICLE_NAME, 100);
}

void 
sensor_interface_handler(MSG_INSTANCE msgRef, BYTE_ARRAY callData,
			 void *clientData __attribute__ ((unused)))
{
  FORMATTER_PTR formatter;
  IPC_RETURN_TYPE err = IPC_OK;
  int context_id;
  
  context_id = get_context_id();

  if (context_id < 0) 
    {
      carmen_warn("Bug detected: invalid context\n");
      IPC_freeByteArray(callData);
      return;
    }
  
  if(sensor_pointer_internal[context_id]) 
    {
      if(sensor_pointer_internal[context_id]->range != NULL)
	free(sensor_pointer_internal[context_id]->range);
      if(sensor_pointer_internal[context_id]->mask != NULL)
	free(sensor_pointer_internal[context_id]->mask);
    }
  formatter = IPC_msgInstanceFormatter(msgRef);
  if(sensor_pointer_internal[context_id])
    err = IPC_unmarshallData(formatter, callData,
			     sensor_pointer_internal[context_id],
                             sizeof(carmen_localize_sensor_message));
  IPC_freeByteArray(callData);
  carmen_test_ipc_return(err, "Could not unmarshall", 
			 IPC_msgInstanceName(msgRef));
  if(sensor_handler_internal[context_id])
    sensor_handler_internal[context_id]
      (sensor_pointer_internal[context_id]);
}

void 
carmen_localize_subscribe_sensor_message(carmen_localize_sensor_message 
					 *sensor_msg, carmen_handler_t handler,
					 carmen_subscribe_t subscribe_how)
{
  IPC_RETURN_TYPE err;
  int context_id;

  err = IPC_defineMsg(CARMEN_LOCALIZE_SENSOR_NAME, 
		      IPC_VARIABLE_LENGTH, 
		      CARMEN_LOCALIZE_SENSOR_FMT);
  carmen_test_ipc_exit(err, "Could not define message", 
		       CARMEN_LOCALIZE_SENSOR_NAME);

  if(subscribe_how == CARMEN_UNSUBSCRIBE) 
    {
      IPC_unsubscribe(CARMEN_LOCALIZE_SENSOR_NAME, sensor_interface_handler);
      return;
    }

  context_id = get_context_id();
  if (context_id < 0)
    context_id = add_context();

  if(sensor_msg) 
    {
      sensor_pointer_internal[context_id] = sensor_msg;
      memset(sensor_pointer_internal[context_id], 0, 
	     sizeof(carmen_localize_sensor_message));
    }
  else if(sensor_pointer_internal[context_id] == NULL) 
    {
      sensor_pointer_internal[context_id] = 
	(carmen_localize_sensor_message *)
	calloc(1, sizeof(carmen_localize_sensor_message));
      carmen_test_alloc(sensor_pointer_internal[context_id]);
    }
  sensor_handler_internal[context_id] = handler;
  err = IPC_subscribe(CARMEN_LOCALIZE_SENSOR_NAME,
		      sensor_interface_handler, NULL);
  carmen_test_ipc(err, "Could not subscribe", CARMEN_LOCALIZE_SENSOR_NAME);
  if(subscribe_how == CARMEN_SUBSCRIBE_LATEST)
    IPC_setMsgQueueLength(CARMEN_LOCALIZE_SENSOR_NAME, 1);
  else
    IPC_setMsgQueueLength(CARMEN_LOCALIZE_SENSOR_NAME, 100);
}

void 
localize_initialize_interface_handler
(MSG_INSTANCE msgRef, BYTE_ARRAY callData,
 void *clientData __attribute__ ((unused)))
{
  FORMATTER_PTR formatter;
  IPC_RETURN_TYPE err = IPC_OK;
  int context_id;
  
  context_id = get_context_id();

  if (context_id < 0) 
    {
      carmen_warn("Bug detected: invalid context\n");
      IPC_freeByteArray(callData);
      return;
    }
  
  if(localize_initialize_internal[context_id]) 
    {
      if(localize_initialize_internal[context_id]->mean != NULL)
	free(localize_initialize_internal[context_id]->mean);
      if(localize_initialize_internal[context_id]->std != NULL)
	free(localize_initialize_internal[context_id]->std);
    }
  formatter = IPC_msgInstanceFormatter(msgRef);
  if(localize_initialize_internal[context_id])
    err = IPC_unmarshallData(formatter, callData, 
			     localize_initialize_internal[context_id],
                             sizeof(carmen_localize_initialize_message));
  IPC_freeByteArray(callData);
  carmen_test_ipc_return(err, "Could not unmarshall", 
			 IPC_msgInstanceName(msgRef));
  
  if(localize_initialize_handler[context_id])
    localize_initialize_handler[context_id]
      (localize_initialize_internal[context_id]);
}

void 
carmen_localize_subscribe_initialize_message
(carmen_localize_initialize_message *init_msg,
 carmen_handler_t handler, 
 carmen_subscribe_t subscribe_how)
{
  IPC_RETURN_TYPE err;
  int context_id;
  
  err = IPC_defineMsg(CARMEN_LOCALIZE_INITIALIZE_NAME, 
		      IPC_VARIABLE_LENGTH, 
		      CARMEN_LOCALIZE_INITIALIZE_FMT);
  carmen_test_ipc_exit(err, "Could not define message", 
		       CARMEN_LOCALIZE_INITIALIZE_NAME);

  if(subscribe_how == CARMEN_UNSUBSCRIBE) 
    {
      IPC_unsubscribe(CARMEN_LOCALIZE_INITIALIZE_NAME, 
		      localize_initialize_interface_handler);
      return;
    }
  
  context_id = get_context_id();
  if (context_id < 0)
    context_id = add_context();

  if(init_msg) 
    localize_initialize_internal[context_id] = init_msg;
  else if(localize_initialize_internal[context_id] == NULL) 
    {
      localize_initialize_internal[context_id] = 
	(carmen_localize_initialize_message *)calloc
	(1, sizeof(carmen_localize_initialize_message));
      carmen_test_alloc(localize_initialize_internal[context_id]);
    }
  localize_initialize_handler[context_id] = handler;
  err = IPC_subscribe(CARMEN_LOCALIZE_INITIALIZE_NAME, 
		      localize_initialize_interface_handler, NULL);
  carmen_test_ipc(err, "Could not subscribe", CARMEN_LOCALIZE_INITIALIZE_NAME);
  if(subscribe_how == CARMEN_SUBSCRIBE_LATEST)
    IPC_setMsgQueueLength(CARMEN_LOCALIZE_INITIALIZE_NAME, 1);
  else
    IPC_setMsgQueueLength(CARMEN_LOCALIZE_INITIALIZE_NAME, 100);
}


void 
carmen_localize_initialize_gaussian_command(carmen_point_t mean,
					    carmen_point_t std)
{
  static carmen_localize_initialize_message init;
  static int first = 1;
  IPC_RETURN_TYPE err;
  
  if(first) 
    {
      err = IPC_defineMsg(CARMEN_LOCALIZE_INITIALIZE_NAME, 
			  IPC_VARIABLE_LENGTH, 
			  CARMEN_LOCALIZE_INITIALIZE_FMT);
      carmen_test_ipc_exit(err, "Could not define message", 
			   CARMEN_LOCALIZE_INITIALIZE_NAME);

      first = 0;
    }
  init.timestamp = carmen_get_time();
  init.host = carmen_get_host();

  init.distribution = CARMEN_INITIALIZE_GAUSSIAN;
  init.num_modes = 1;
  init.mean = &mean;
  init.std = &std;
  err = IPC_publishData(CARMEN_LOCALIZE_INITIALIZE_NAME, &init);
  carmen_test_ipc(err, "Could not publish", CARMEN_LOCALIZE_INITIALIZE_NAME);
}

void carmen_localize_initialize_uniform_command(void)
{
  static carmen_localize_initialize_message init;
  static int first = 1;
  IPC_RETURN_TYPE err;

  if(first) 
    {
      err = IPC_defineMsg(CARMEN_LOCALIZE_INITIALIZE_NAME, 
			  IPC_VARIABLE_LENGTH, 
			  CARMEN_LOCALIZE_INITIALIZE_FMT);
      carmen_test_ipc_exit(err, "Could not define message", 
			   CARMEN_LOCALIZE_INITIALIZE_NAME);

      first = 0;
    }
  init.timestamp = carmen_get_time();
  init.host = carmen_get_host();
    
  init.distribution = CARMEN_INITIALIZE_UNIFORM;
  init.num_modes = 0;
  init.mean = NULL;
  init.std = NULL;
  err = IPC_publishData(CARMEN_LOCALIZE_INITIALIZE_NAME, &init);
  carmen_test_ipc(err, "Could not publish", CARMEN_LOCALIZE_INITIALIZE_NAME);
}

void carmen_localize_initialize_placename_command(char *placename)
{
  static carmen_localize_initialize_placename_message init;
  static int first = 1;
  IPC_RETURN_TYPE err;
  
  if(first) {
    err = IPC_defineMsg(CARMEN_LOCALIZE_INITIALIZE_PLACENAME_NAME, 
			IPC_VARIABLE_LENGTH, 
			CARMEN_LOCALIZE_INITIALIZE_PLACENAME_FMT);
    carmen_test_ipc_exit(err, "Could not define message", 
			 CARMEN_LOCALIZE_INITIALIZE_PLACENAME_NAME);
    first = 0;
  }
  init.timestamp = carmen_get_time();
  init.host = carmen_get_host();
  init.placename = placename;
  err = IPC_publishData(CARMEN_LOCALIZE_INITIALIZE_PLACENAME_NAME, &init);
  carmen_test_ipc(err, "Could not publish", 
		  CARMEN_LOCALIZE_INITIALIZE_PLACENAME_NAME);
}

void
carmen_localize_correct_odometry(carmen_base_odometry_message *odometry, 
				 carmen_localize_globalpos_message *globalpos)
{
  int backwards;
  double dr1, dt, dr2;
  double dx, dy;

  dx = odometry->x - globalpos->odometrypos.x;
  dy = odometry->y - globalpos->odometrypos.y;
  dt = sqrt(dx * dx + dy * dy);
  backwards = (dx * cos(odometry->theta) + dy * sin(odometry->theta) < 0);
  
  /* The dr1/dr2 code becomes unstable if dt is too small. */
  if(dt < 0.05) 
    {
      dr1 = carmen_normalize_theta(odometry->theta - 
				   globalpos->odometrypos.theta) / 2.0;
      dr2 = dr1;
    }
  else 
    {
      if(backwards)
	dr1 = carmen_normalize_theta(atan2(globalpos->odometrypos.y - 
					   odometry->y, 
					   globalpos->odometrypos.x - 
					   odometry->x) - 
				     globalpos->odometrypos.theta);
      else
	dr1 = carmen_normalize_theta(atan2(odometry->y - 
					   globalpos->odometrypos.y, 
					   odometry->x - 
					   globalpos->odometrypos.x) - 
				     globalpos->odometrypos.theta);
      dr2 = carmen_normalize_theta(odometry->theta - 
				   globalpos->odometrypos.theta - dr1);
    }
  if(backwards) 
    dt = -dt;
  odometry->x = globalpos->globalpos.x + dt * 
    cos(globalpos->globalpos.theta + dr1);
  odometry->y = globalpos->globalpos.y + dt * 
    sin(globalpos->globalpos.theta + dr1);
  odometry->theta = carmen_normalize_theta(globalpos->globalpos.theta + 
					   dr1 + dr2);
}

void
carmen_localize_correct_laser(carmen_robot_laser_message *laser, 
			      carmen_localize_globalpos_message *globalpos)
{
  int backwards;
  double dr1, dt, dr2;
  double dx, dy;
  double dtheta;

  dx = laser->laser_location.x - globalpos->odometrypos.x;
  dy = laser->laser_location.y - globalpos->odometrypos.y;
  dtheta = laser->laser_location.theta - globalpos->odometrypos.theta;

  dt = sqrt(dx * dx + dy * dy);
  backwards = (dx * cos(laser->laser_location.theta) + 
	       dy * sin(laser->laser_location.theta) < 0);

  /* The dr1/dr2 code becomes unstable if dt is too small. */
  if(dt < 0.05) {
    dr1 = carmen_normalize_theta(laser->laser_location.theta - 
				 globalpos->odometrypos.theta) / 2.0;
    dr2 = dr1;
  } else {
    if(backwards)
      dr1 = carmen_normalize_theta(atan2(-dy, -dx)-
				   globalpos->odometrypos.theta);
    else
      dr1 = carmen_normalize_theta(atan2(dy, dx)-
				   globalpos->odometrypos.theta);
    dr2 = carmen_normalize_theta(dtheta - dr1);
    }
  if(backwards) 
    dt = -dt;
  laser->laser_location.x = globalpos->globalpos.x + dt * 
    cos(globalpos->globalpos.theta + dr1);
  laser->laser_location.y = globalpos->globalpos.y + dt * 
    sin(globalpos->globalpos.theta + dr1);
  laser->laser_location.theta = 
    carmen_normalize_theta(globalpos->globalpos.theta + dr1 + dr2);
}

int carmen_localize_get_map(int global, carmen_map_t *map) 
{
  IPC_RETURN_TYPE err;
  carmen_localize_query_message msg;
  carmen_localize_map_message *response = NULL;
  int index;

#ifndef NO_ZLIB
  int uncompress_return;
  int uncompress_size;
  int uncompress_size_result;
  unsigned char *uncompressed_data;
#endif

  static int initialized = 0;

  if (!initialized) 
    {
      err = IPC_defineMsg(CARMEN_LOCALIZE_QUERY_NAME, 
			  IPC_VARIABLE_LENGTH, 
			  CARMEN_LOCALIZE_QUERY_FMT);
      carmen_test_ipc_exit(err, "Could not define message", 
			   CARMEN_LOCALIZE_QUERY_NAME);
      initialized = 1;
    }

  msg.global = global;
  msg.timestamp = carmen_get_time();
  msg.host = carmen_get_host();

  err = IPC_queryResponseData(CARMEN_LOCALIZE_QUERY_NAME, &msg, 
			      (void **)&response, timeout);
  carmen_test_ipc(err, "Could not get map", CARMEN_LOCALIZE_QUERY_NAME);

#ifndef NO_ZLIB
  if (response && response->compressed) 
    {
      uncompress_size = response->config.x_size*
	response->config.y_size;
      uncompressed_data = (unsigned char *)
	calloc(uncompress_size, sizeof(float));
      carmen_test_alloc(uncompressed_data);
      uncompress_size_result = uncompress_size*sizeof(float);
      uncompress_return = uncompress((void *)uncompressed_data,   
				     (uLong *)&uncompress_size_result,
				     (void *)response->data, 
				     response->size);
      response->data = uncompressed_data;
      response->size = uncompress_size_result;
    }
#else
  if (response && response->compressed) 
    {
      carmen_warn("Received compressed map from server. This program was\n"
		  "compiled without zlib support, so this map cannot be\n"
		  "used. Sorry.\n");
      free(response->data);
      free(response);
      response = NULL;
    }
#endif

  if (response)
    {
      if (map)
	{
	  map->config = response->config;
	  map->complete_map = (float *)response->data;
	  map->map = (float **)calloc(map->config.x_size, sizeof(float));
	  carmen_test_alloc(map->map);
	  
	  for (index = 0; index < map->config.x_size; index++)
	    map->map[index] = map->complete_map+index*map->config.y_size;
	}
      else
	free(response->data);
      free(response);
    } 

  return 0;
}
