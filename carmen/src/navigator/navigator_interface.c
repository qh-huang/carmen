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
#ifndef NO_ZLIB
#include <zlib.h>
#endif

carmen_navigator_status_message **status_msg;
carmen_navigator_plan_message **plan_msg;
carmen_navigator_autonomous_stopped_message **autonomous_stop_msg;

carmen_handler_t *status_message_handler_external;
carmen_handler_t *plan_message_handler_external;
carmen_handler_t *autonomous_stopped_handler_external;

static unsigned int timeout = 500;

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

      status_msg = (carmen_navigator_status_message **)
	calloc(10, sizeof(carmen_navigator_status_message *));
      carmen_test_alloc(status_msg);

      plan_msg = (carmen_navigator_plan_message **)
	calloc(10, sizeof(carmen_navigator_plan_message *));
      carmen_test_alloc(plan_msg);

      autonomous_stop_msg = (carmen_navigator_autonomous_stopped_message **)
	calloc(10, sizeof(carmen_navigator_autonomous_stopped_message *));
      carmen_test_alloc(autonomous_stop_msg);

      status_message_handler_external = (carmen_handler_t *)
	calloc(10, sizeof(carmen_handler_t));
      carmen_test_alloc(status_message_handler_external);

      plan_message_handler_external = (carmen_handler_t *)
	calloc(10, sizeof(carmen_handler_t));
      carmen_test_alloc(status_message_handler_external);

      autonomous_stopped_handler_external = (carmen_handler_t *)
	calloc(10, sizeof(carmen_handler_t));
      carmen_test_alloc(status_message_handler_external);

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
status_interface_handler(MSG_INSTANCE msgRef, BYTE_ARRAY callData,
			 void *clientData __attribute__ ((unused)))
{
  IPC_RETURN_TYPE err = IPC_OK;
  FORMATTER_PTR formatter;
  int context_id;

  context_id = get_context_id();

  if (context_id < 0) 
    {
      carmen_warn("Bug detected: invalid context\n");
      IPC_freeByteArray(callData);
      return;
    }

  formatter = IPC_msgInstanceFormatter(msgRef);
  if (status_msg[context_id]) 
    err = IPC_unmarshallData(formatter, callData, status_msg[context_id], 
			     sizeof(carmen_navigator_status_message));

  IPC_freeByteArray(callData);

  carmen_test_ipc_return(err, "Could not unmarshall", 
			 IPC_msgInstanceName(msgRef));

  if (status_message_handler_external[context_id])
    status_message_handler_external[context_id](status_msg[context_id]);
}

void 
carmen_navigator_subscribe_status_message(carmen_navigator_status_message 
					  *status,
					  carmen_handler_t handler,
					  carmen_subscribe_t subscribe_how) 
{
  IPC_RETURN_TYPE err = IPC_OK;  
  int context_id;

  err = IPC_defineMsg(CARMEN_NAVIGATOR_STATUS_NAME, IPC_VARIABLE_LENGTH, 
		      CARMEN_NAVIGATOR_STATUS_FMT);
  carmen_test_ipc_exit(err, "Could not define message", 
		       CARMEN_NAVIGATOR_STATUS_NAME);

  if (subscribe_how == CARMEN_UNSUBSCRIBE) 
    {
      IPC_unsubscribe(CARMEN_NAVIGATOR_STATUS_NAME, status_interface_handler);
      return;
    }

  context_id = get_context_id();
  if (context_id < 0)
    context_id = add_context();

  if (status)
    status_msg[context_id] = status;
  else if (status_msg[context_id] == NULL)
    {
      status_msg[context_id] = 
	(carmen_navigator_status_message *)calloc
	(1, sizeof(carmen_navigator_status_message));
      carmen_test_alloc(status_msg[context_id]);
    }

  status_message_handler_external[context_id] = handler;

  err = IPC_subscribe(CARMEN_NAVIGATOR_STATUS_NAME, status_interface_handler, 
		      NULL);
  if (subscribe_how == CARMEN_SUBSCRIBE_LATEST)
    IPC_setMsgQueueLength(CARMEN_NAVIGATOR_STATUS_NAME, 1);
  else
    IPC_setMsgQueueLength(CARMEN_NAVIGATOR_STATUS_NAME, 100);

  carmen_test_ipc(err, "Could not subscribe", CARMEN_NAVIGATOR_STATUS_NAME);
}

int
carmen_navigator_query_status(carmen_navigator_status_message **status_msg) 
{
  IPC_RETURN_TYPE err;
  carmen_navigator_query_message msg;
  static int initialized = 0;

  msg.timestamp = carmen_get_time_ms();
  strcpy(msg.host, carmen_get_tenchar_host_name());

  if (!initialized) 
    {
      err = IPC_defineMsg(CARMEN_NAVIGATOR_STATUS_QUERY_NAME, 
			  IPC_VARIABLE_LENGTH, 
			  CARMEN_NAVIGATOR_STATUS_QUERY_FMT);
      carmen_test_ipc_exit(err, "Could not define message", 
			   CARMEN_NAVIGATOR_STATUS_QUERY_NAME);
      initialized = 1;
    }

  err = IPC_queryResponseData(CARMEN_NAVIGATOR_STATUS_QUERY_NAME, &msg, 
			      (void **)status_msg, timeout);
  carmen_test_ipc_return_int(err, "Could not query navigator status", 
			     CARMEN_NAVIGATOR_STATUS_QUERY_NAME);

  return 0;
}

static void 
plan_interface_handler(MSG_INSTANCE msgRef, BYTE_ARRAY callData,
		       void *clientData __attribute__ ((unused)) )
{
  IPC_RETURN_TYPE err = IPC_OK;
  FORMATTER_PTR formatter;
  int context_id;

  context_id = get_context_id();

  if (context_id < 0) 
    {
      carmen_warn("Bug detected: invalid context\n");
      IPC_freeByteArray(callData);
      return;
    }

  formatter = IPC_msgInstanceFormatter(msgRef);
  if (plan_msg[context_id])    
    err = IPC_unmarshallData(formatter, callData, plan_msg[context_id], 
			     sizeof(carmen_navigator_plan_message));
  IPC_freeByteArray(callData);

  carmen_test_ipc_return(err, "Could not unmarshall", 
			 IPC_msgInstanceName(msgRef));

  if (plan_message_handler_external[context_id]) 
    plan_message_handler_external[context_id](plan_msg[context_id]);
}

void 
carmen_navigator_subscribe_plan_message(carmen_navigator_plan_message *plan,
					carmen_handler_t handler,
					carmen_subscribe_t subscribe_how) 
{
  IPC_RETURN_TYPE err = IPC_OK;
  int context_id;

  err = IPC_defineMsg(CARMEN_NAVIGATOR_PLAN_NAME, IPC_VARIABLE_LENGTH, 
		      CARMEN_NAVIGATOR_PLAN_FMT);
  carmen_test_ipc_exit(err, "Could not define message", 
		       CARMEN_NAVIGATOR_PLAN_NAME);

  if (subscribe_how == CARMEN_UNSUBSCRIBE) 
    {
      IPC_unsubscribe(CARMEN_NAVIGATOR_PLAN_NAME, plan_interface_handler);
      return;
    }

  context_id = get_context_id();
  if (context_id < 0)
    context_id = add_context();

  if (plan)
    plan_msg[context_id] = plan;
  else if (plan_msg[context_id] == NULL)
    {
      plan_msg[context_id] = 
	(carmen_navigator_plan_message *)calloc
	(1, sizeof(carmen_navigator_plan_message));
      carmen_test_alloc(plan_msg[context_id]);
    }

  plan_message_handler_external[context_id] = handler;

  err = IPC_subscribe(CARMEN_NAVIGATOR_PLAN_NAME, 
		      plan_interface_handler, NULL);
  if (subscribe_how == CARMEN_SUBSCRIBE_LATEST)
    IPC_setMsgQueueLength(CARMEN_NAVIGATOR_PLAN_NAME, 1);
  else
    IPC_setMsgQueueLength(CARMEN_NAVIGATOR_PLAN_NAME, 100);
  carmen_test_ipc(err, "Could not subscribe", CARMEN_NAVIGATOR_PLAN_NAME);
}

int 
carmen_navigator_query_plan(carmen_navigator_plan_message **plan_msg) 
{
  IPC_RETURN_TYPE err;
  carmen_navigator_query_message msg;
  static int initialized = 0;

  if (!initialized) 
    {
      err = IPC_defineMsg(CARMEN_NAVIGATOR_PLAN_QUERY_NAME, 
			  IPC_VARIABLE_LENGTH, 
			  CARMEN_NAVIGATOR_PLAN_QUERY_FMT);
      carmen_test_ipc_exit(err, "Could not define message", 
			   CARMEN_NAVIGATOR_PLAN_QUERY_NAME);
      initialized = 1;
    }

  msg.timestamp = carmen_get_time_ms();
  strcpy(msg.host, carmen_get_tenchar_host_name());

  err = IPC_queryResponseData(CARMEN_NAVIGATOR_PLAN_QUERY_NAME, &msg, 
			      (void **)plan_msg, timeout);
  carmen_test_ipc_return_int(err, "Could not query plans", 
			     CARMEN_NAVIGATOR_PLAN_QUERY_NAME);
	
  return 0;
}

static void 
autonomous_stopped_interface_handler(MSG_INSTANCE msgRef, BYTE_ARRAY callData,
				     void *clientData __attribute__ ((unused)))
{
  IPC_RETURN_TYPE err = IPC_OK;
  FORMATTER_PTR formatter;
  int context_id;

  context_id = get_context_id();

  if (context_id < 0) 
    {
      carmen_warn("Bug detected: invalid context\n");
      IPC_freeByteArray(callData);
      return;
    }

  formatter = IPC_msgInstanceFormatter(msgRef);
  if (autonomous_stop_msg[context_id])
    err = IPC_unmarshallData
      (formatter, callData, autonomous_stop_msg[context_id], 
       sizeof(carmen_navigator_autonomous_stopped_message));

  IPC_freeByteArray(callData);

  carmen_test_ipc_return(err, "Could not unmarshall", 
			 IPC_msgInstanceName(msgRef));

  if (autonomous_stopped_handler_external[context_id])
    autonomous_stopped_handler_external[context_id]
      (autonomous_stop_msg[context_id]);
}

void 
carmen_navigator_subscribe_autonomous_stopped_message
(carmen_navigator_autonomous_stopped_message *autonomous_stopped,
 carmen_handler_t handler,
 carmen_subscribe_t subscribe_how) 
{
  IPC_RETURN_TYPE err = IPC_OK;  
  int context_id;

  err = IPC_defineMsg(CARMEN_NAVIGATOR_AUTONOMOUS_STOPPED_NAME, 
		      IPC_VARIABLE_LENGTH, 
		      CARMEN_NAVIGATOR_AUTONOMOUS_STOPPED_FMT);
  carmen_test_ipc_exit(err, "Could not define message", 
		       CARMEN_NAVIGATOR_AUTONOMOUS_STOPPED_NAME);

  if (subscribe_how == CARMEN_UNSUBSCRIBE) 
    {
      IPC_unsubscribe(CARMEN_NAVIGATOR_AUTONOMOUS_STOPPED_NAME, 
		      autonomous_stopped_interface_handler);
      return;
    }

  context_id = get_context_id();
  if (context_id < 0)
    context_id = add_context();

  if (autonomous_stopped)
    autonomous_stop_msg[context_id] = autonomous_stopped;
  else if (autonomous_stop_msg[context_id] == NULL)
    {
      autonomous_stop_msg[context_id] = 
	(carmen_navigator_autonomous_stopped_message *)
	calloc(1, sizeof(carmen_navigator_autonomous_stopped_message));
      carmen_test_alloc(autonomous_stop_msg[context_id]);
    }
  autonomous_stopped_handler_external[context_id] = handler;

  err = IPC_subscribe(CARMEN_NAVIGATOR_AUTONOMOUS_STOPPED_NAME, 
		      autonomous_stopped_interface_handler, NULL);

  if (subscribe_how == CARMEN_SUBSCRIBE_LATEST)
    IPC_setMsgQueueLength(CARMEN_NAVIGATOR_AUTONOMOUS_STOPPED_NAME, 1);
  else
    IPC_setMsgQueueLength(CARMEN_NAVIGATOR_AUTONOMOUS_STOPPED_NAME, 100);

  carmen_test_ipc(err, "Could not subscribe", 
		  CARMEN_NAVIGATOR_AUTONOMOUS_STOPPED_NAME);
}

int 
carmen_navigator_set_goal(double x, double y) 
{
  IPC_RETURN_TYPE err = IPC_OK;
  carmen_navigator_set_goal_message msg;
  static int initialized = 0;

  if (!initialized) 
    {
      err = IPC_defineMsg(CARMEN_NAVIGATOR_SET_GOAL_NAME, 
			  IPC_VARIABLE_LENGTH, 
			  CARMEN_NAVIGATOR_SET_GOAL_FMT);
      carmen_test_ipc_exit(err, "Could not define message", 
			   CARMEN_NAVIGATOR_SET_GOAL_NAME);
      initialized = 1;
    }

  msg.x = x;
  msg.y = y;
  msg.timestamp = carmen_get_time_ms();
  strcpy(msg.host, carmen_get_tenchar_host_name());

  err = IPC_publishData(CARMEN_NAVIGATOR_SET_GOAL_NAME, &msg);
  carmen_test_ipc(err, "Could not publish", CARMEN_NAVIGATOR_SET_GOAL_NAME);

  return 0;
}

int 
carmen_navigator_set_goal_triplet(carmen_point_p goal) 
{
  IPC_RETURN_TYPE err = IPC_OK;
  carmen_navigator_set_goal_triplet_message msg;
  static int initialized = 0;

  if (!initialized) 
    {
      err = IPC_defineMsg(CARMEN_NAVIGATOR_SET_GOAL_TRIPLET_NAME, 
			  IPC_VARIABLE_LENGTH, 
			  CARMEN_NAVIGATOR_SET_GOAL_TRIPLET_FMT);
      carmen_test_ipc_exit(err, "Could not define message", 
			   CARMEN_NAVIGATOR_SET_GOAL_TRIPLET_NAME);
      initialized = 1;
    }


  msg.goal = *goal;
  msg.timestamp = carmen_get_time_ms();
  strcpy(msg.host, carmen_get_tenchar_host_name());

  err = IPC_publishData(CARMEN_NAVIGATOR_SET_GOAL_TRIPLET_NAME, &msg);
  carmen_test_ipc(err, "Could not publish", 
		  CARMEN_NAVIGATOR_SET_GOAL_TRIPLET_NAME);

  return 0;
}

int 
carmen_navigator_set_goal_place(char *placename)
{
  IPC_RETURN_TYPE err = IPC_OK;
  carmen_navigator_placename_message msg;
  carmen_navigator_return_code_message *return_msg;
  int return_code;
  static int initialized = 0;

  if (!initialized) 
    {
      err = IPC_defineMsg(CARMEN_NAVIGATOR_SET_GOAL_PLACE_NAME, 
			  IPC_VARIABLE_LENGTH, 
			  CARMEN_NAVIGATOR_SET_GOAL_PLACE_FMT);
      carmen_test_ipc_exit(err, "Could not define message", 
			   CARMEN_NAVIGATOR_SET_GOAL_PLACE_NAME);
      initialized = 1;
    }


  msg.placename = placename;
  msg.timestamp = carmen_get_time_ms();
  strcpy(msg.host, carmen_get_tenchar_host_name());

  err = IPC_queryResponseData(CARMEN_NAVIGATOR_SET_GOAL_PLACE_NAME, &msg, 
			      (void **)&return_msg, timeout);
  carmen_test_ipc(err, "Could not set goal by place", 
		  CARMEN_NAVIGATOR_SET_GOAL_PLACE_NAME);

  if (err == IPC_OK) {
    if (return_msg->code) {
      carmen_warn(return_msg->error);
      free(return_msg->error);
    }

    return_code = return_msg->code;
    free(return_msg);
  } else 
    return_code = err;

  return return_code;
}

int 
carmen_navigator_stop(void) 
{
  IPC_RETURN_TYPE err;
  carmen_navigator_stop_message msg;
  static int initialized = 0;

  if (!initialized) 
    {
      err = IPC_defineMsg(CARMEN_NAVIGATOR_STOP_NAME, 
			  IPC_VARIABLE_LENGTH, 
			  CARMEN_NAVIGATOR_STOP_FMT);
      carmen_test_ipc_exit(err, "Could not define message", 
			   CARMEN_NAVIGATOR_STOP_NAME);
      initialized = 1;
    }

  msg.timestamp = carmen_get_time_ms();
  strcpy(msg.host, carmen_get_tenchar_host_name());

  err = IPC_publishData(CARMEN_NAVIGATOR_STOP_NAME, &msg);
  carmen_test_ipc(err, "Could not publish", CARMEN_NAVIGATOR_STOP_NAME);

  return 0;
}

int 
carmen_navigator_go(void) 
{
  IPC_RETURN_TYPE err;
  carmen_navigator_go_message msg;
  static int initialized = 0;

  if (!initialized) 
    {
      err = IPC_defineMsg(CARMEN_NAVIGATOR_GO_NAME, 
			  IPC_VARIABLE_LENGTH, 
			  CARMEN_NAVIGATOR_GO_FMT);
      carmen_test_ipc_exit(err, "Could not define message", 
			   CARMEN_NAVIGATOR_GO_NAME);
      initialized = 1;
    }

  msg.timestamp = carmen_get_time_ms();
  strcpy(msg.host, carmen_get_tenchar_host_name());

  err = IPC_publishData(CARMEN_NAVIGATOR_GO_NAME, &msg);
  carmen_test_ipc(err, "Could not publish", CARMEN_NAVIGATOR_GO_NAME);

  return 0;
}


int 
carmen_navigator_get_map(carmen_navigator_map_t map_type, 
			 carmen_map_t *map) 
{
  IPC_RETURN_TYPE err;
  carmen_navigator_map_request_message msg;
  carmen_navigator_map_message *response = NULL;
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
      err = IPC_defineMsg(CARMEN_NAVIGATOR_MAP_REQUEST_NAME, 
			  IPC_VARIABLE_LENGTH, 
			  CARMEN_NAVIGATOR_MAP_REQUEST_FMT);
      carmen_test_ipc_exit(err, "Could not define message", 
			   CARMEN_NAVIGATOR_MAP_REQUEST_NAME);
      initialized = 1;
    }

  msg.map_type = map_type;
  msg.timestamp = carmen_get_time_ms();
  strcpy(msg.host, carmen_get_tenchar_host_name());

  err = IPC_queryResponseData(CARMEN_NAVIGATOR_MAP_REQUEST_NAME, &msg, 
			      (void **)&response, timeout);
  carmen_test_ipc(err, "Could not get map", CARMEN_NAVIGATOR_MAP_REQUEST_NAME);

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
