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

carmen_simulator_truepos_message **carmen_simulator_interface_truepos_msg;
carmen_simulator_objects_message **carmen_simulator_interface_objects_msg;

carmen_handler_t *truepos_message_handler_external;
carmen_handler_t *objects_message_handler_external;

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

      carmen_simulator_interface_truepos_msg = (carmen_simulator_truepos_message **)
	calloc(10, sizeof(carmen_simulator_truepos_message *));
      carmen_test_alloc(carmen_simulator_interface_truepos_msg);

      carmen_simulator_interface_objects_msg = (carmen_simulator_objects_message **)
	calloc(10, sizeof(carmen_simulator_objects_message *));
      carmen_test_alloc(carmen_simulator_interface_objects_msg);

      truepos_message_handler_external = (carmen_handler_t *)
	calloc(10, sizeof(carmen_handler_t));
      carmen_test_alloc(truepos_message_handler_external);

      objects_message_handler_external = (carmen_handler_t *)
	calloc(10, sizeof(carmen_handler_t));
      carmen_test_alloc(objects_message_handler_external);

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
truepos_interface_handler(MSG_INSTANCE msgRef, BYTE_ARRAY callData,
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
  err = IPC_unmarshallData(formatter, callData, carmen_simulator_interface_truepos_msg[context_id], 
			   sizeof(carmen_simulator_truepos_message));
  IPC_freeByteArray(callData);

  carmen_test_ipc_return(err, "Could not unmarshall", 
			 IPC_msgInstanceName(msgRef));

  if (truepos_message_handler_external[context_id])
    truepos_message_handler_external[context_id](carmen_simulator_interface_truepos_msg[context_id]);
}

void 
carmen_simulator_subscribe_truepos_message(carmen_simulator_truepos_message
					   *truepos,
					   carmen_handler_t handler,
					   carmen_subscribe_t subscribe_how) 
{
  IPC_RETURN_TYPE err = IPC_OK;  
  int context_id;

  err = IPC_defineMsg(CARMEN_SIMULATOR_TRUEPOS_NAME, IPC_VARIABLE_LENGTH, 
		      CARMEN_SIMULATOR_TRUEPOS_FMT);
  carmen_test_ipc_exit(err, "Could not define message", 
		       CARMEN_SIMULATOR_TRUEPOS_NAME);

  if (subscribe_how == CARMEN_UNSUBSCRIBE) 
    {
      IPC_unsubscribe
	(CARMEN_SIMULATOR_TRUEPOS_NAME, truepos_interface_handler);
      return;
    }

  context_id = get_context_id();
  if (context_id < 0)
    context_id = add_context();

  if (truepos)
    carmen_simulator_interface_truepos_msg[context_id] = truepos;
  else if (carmen_simulator_interface_truepos_msg[context_id] == NULL)
    {
      carmen_simulator_interface_truepos_msg[context_id] = 
	(carmen_simulator_truepos_message *)calloc
	(1, sizeof(carmen_simulator_truepos_message));
      carmen_test_alloc(carmen_simulator_interface_truepos_msg[context_id]);
    }

  truepos_message_handler_external[context_id] = handler;

  err = IPC_subscribe(CARMEN_SIMULATOR_TRUEPOS_NAME, truepos_interface_handler,
		      NULL);
  if (subscribe_how == CARMEN_SUBSCRIBE_LATEST)
    IPC_setMsgQueueLength(CARMEN_SIMULATOR_TRUEPOS_NAME, 1);
  else
    IPC_setMsgQueueLength(CARMEN_SIMULATOR_TRUEPOS_NAME, 100);

  carmen_test_ipc(err, "Could not subscribe", CARMEN_SIMULATOR_TRUEPOS_NAME);
}

int
carmen_simulator_query_truepos(carmen_simulator_truepos_message **carmen_simulator_interface_truepos_msg) 
{
  IPC_RETURN_TYPE err;
  carmen_simulator_query_message msg;
  static int initialized = 0;

  msg.timestamp = carmen_get_time_ms();
  strcpy(msg.host, carmen_get_tenchar_host_name());

  if (!initialized) 
    {
      err = IPC_defineMsg(CARMEN_SIMULATOR_TRUEPOS_QUERY_NAME, 
			  IPC_VARIABLE_LENGTH, 
			  CARMEN_SIMULATOR_TRUEPOS_QUERY_FMT);
      carmen_test_ipc_exit(err, "Could not define message", 
			   CARMEN_SIMULATOR_TRUEPOS_QUERY_NAME);
      initialized = 1;
    }

  err = IPC_queryResponseData(CARMEN_SIMULATOR_TRUEPOS_QUERY_NAME, &msg, 
			      (void **)carmen_simulator_interface_truepos_msg, timeout);
  carmen_test_ipc_return_int(err, "Could not query simulator truepos", 
			     CARMEN_SIMULATOR_TRUEPOS_QUERY_NAME);

  return 0;
}

static void 
objects_interface_handler(MSG_INSTANCE msgRef, BYTE_ARRAY callData,
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
  err = IPC_unmarshallData(formatter, callData, carmen_simulator_interface_objects_msg[context_id], 
			   sizeof(carmen_simulator_objects_message));
  IPC_freeByteArray(callData);

  carmen_test_ipc_return(err, "Could not unmarshall", 
			 IPC_msgInstanceName(msgRef));

  if (objects_message_handler_external[context_id]) 
    objects_message_handler_external[context_id](carmen_simulator_interface_objects_msg[context_id]);
}

void 
carmen_simulator_subscribe_objects_message(carmen_simulator_objects_message 
					   *objects,
					   carmen_handler_t handler,
					   carmen_subscribe_t subscribe_how) 
{
  IPC_RETURN_TYPE err = IPC_OK;
  int context_id;

  err = IPC_defineMsg(CARMEN_SIMULATOR_OBJECTS_NAME, IPC_VARIABLE_LENGTH, 
		      CARMEN_SIMULATOR_OBJECTS_FMT);
  carmen_test_ipc_exit(err, "Could not define message", 
		       CARMEN_SIMULATOR_OBJECTS_NAME);

  if (subscribe_how == CARMEN_UNSUBSCRIBE) 
    {
      IPC_unsubscribe(CARMEN_SIMULATOR_OBJECTS_NAME, 
		      objects_interface_handler);
      return;
    }

  context_id = get_context_id();
  if (context_id < 0)
    context_id = add_context();

  if (objects)
    carmen_simulator_interface_objects_msg[context_id] = objects;
  else if (carmen_simulator_interface_objects_msg[context_id] == NULL)
    {
      carmen_simulator_interface_objects_msg[context_id] = 
	(carmen_simulator_objects_message *)calloc
	(1, sizeof(carmen_simulator_objects_message));
      carmen_test_alloc(carmen_simulator_interface_objects_msg[context_id]);
    }

  objects_message_handler_external[context_id] = handler;

  err = IPC_subscribe(CARMEN_SIMULATOR_OBJECTS_NAME, 
		      objects_interface_handler, NULL);
  if (subscribe_how == CARMEN_SUBSCRIBE_LATEST)
    IPC_setMsgQueueLength(CARMEN_SIMULATOR_OBJECTS_NAME, 1);
  else
    IPC_setMsgQueueLength(CARMEN_SIMULATOR_OBJECTS_NAME, 100);
  carmen_test_ipc(err, "Could not subscribe", CARMEN_SIMULATOR_OBJECTS_NAME);
}

int 
carmen_simulator_query_objects(carmen_simulator_objects_message **carmen_simulator_interface_objects_msg) 
{
  IPC_RETURN_TYPE err;
  carmen_simulator_query_message msg;
  static int initialized = 0;

  if (!initialized) 
    {
      err = IPC_defineMsg(CARMEN_SIMULATOR_OBJECTS_QUERY_NAME, 
			  IPC_VARIABLE_LENGTH, 
			  CARMEN_SIMULATOR_OBJECTS_QUERY_FMT);
      carmen_test_ipc_exit(err, "Could not define message", 
			   CARMEN_SIMULATOR_OBJECTS_QUERY_NAME);
      initialized = 1;
    }

  msg.timestamp = carmen_get_time_ms();
  strcpy(msg.host, carmen_get_tenchar_host_name());

  err = IPC_queryResponseData(CARMEN_SIMULATOR_OBJECTS_QUERY_NAME, &msg, 
			      (void **)carmen_simulator_interface_objects_msg, timeout);
  carmen_test_ipc_return_int(err, "Could not query objects", 
			     CARMEN_SIMULATOR_OBJECTS_QUERY_NAME);
	
  return 0;
}

int 
carmen_simulator_set_object(carmen_point_t *point, double speed,
			    carmen_simulator_object_t type) 
{
  IPC_RETURN_TYPE err = IPC_OK;
  carmen_simulator_set_object_message msg;
  static int initialized = 0;

  if (!initialized) 
    {
      err = IPC_defineMsg(CARMEN_SIMULATOR_SET_OBJECT_NAME, 
			  IPC_VARIABLE_LENGTH, 
			  CARMEN_SIMULATOR_SET_OBJECT_FMT);
      carmen_test_ipc_exit(err, "Could not define message", 
			   CARMEN_SIMULATOR_SET_OBJECT_NAME);
      initialized = 1;
    }

  msg.pose = *point;
  msg.speed = speed;
  msg.type = type;
  msg.timestamp = carmen_get_time_ms();
  strcpy(msg.host, carmen_get_tenchar_host_name());

  err = IPC_publishData(CARMEN_SIMULATOR_SET_OBJECT_NAME, &msg);
  carmen_test_ipc(err, "Could not publish", CARMEN_SIMULATOR_SET_OBJECT_NAME);

  return 0;
}

void
carmen_simulator_connect_robots(char *other_central)
{
  IPC_RETURN_TYPE err = IPC_OK;
  carmen_simulator_connect_robots_message msg;
  static int initialized = 0;

  if (!initialized) 
    {
      err = IPC_defineMsg(CARMEN_SIMULATOR_CONNECT_ROBOTS_NAME, 
			  IPC_VARIABLE_LENGTH, 
			  CARMEN_SIMULATOR_CONNECT_ROBOTS_FMT);
      carmen_test_ipc_exit(err, "Could not define message", 
			   CARMEN_SIMULATOR_CONNECT_ROBOTS_NAME);
      initialized = 1;
    }


  msg.other_central = other_central;
  msg.timestamp = carmen_get_time_ms();
  strcpy(msg.host, carmen_get_tenchar_host_name());

  err = IPC_publishData(CARMEN_SIMULATOR_CONNECT_ROBOTS_NAME, &msg);
  carmen_test_ipc(err, "Could not publish", 
		  CARMEN_SIMULATOR_CONNECT_ROBOTS_NAME);
}

void
carmen_simulator_clear_objects(void)
{
  carmen_simulator_clear_objects_message msg;
  static int initialized = 0;
  IPC_RETURN_TYPE err = IPC_OK;

  if (!initialized) 
    {
      err = IPC_defineMsg(CARMEN_SIMULATOR_CLEAR_OBJECTS_NAME, 
			  IPC_VARIABLE_LENGTH, 
			  CARMEN_SIMULATOR_CLEAR_OBJECTS_FMT);
      carmen_test_ipc_exit(err, "Could not define message", 
			   CARMEN_SIMULATOR_CLEAR_OBJECTS_NAME);
      initialized = 1;
    }


  msg.timestamp = carmen_get_time_ms();
  strcpy(msg.host, carmen_get_tenchar_host_name());

  err = IPC_publishData(CARMEN_SIMULATOR_CLEAR_OBJECTS_NAME, &msg);
  carmen_test_ipc(err, "Could not publish", 
		  CARMEN_SIMULATOR_CLEAR_OBJECTS_NAME);
}

void
carmen_simulator_next_tick(void)
{
  IPC_RETURN_TYPE err = IPC_OK;
  carmen_simulator_query_message msg;
  static int initialized = 0;

  if (!initialized) 
    {
      err = IPC_defineMsg(CARMEN_SIMULATOR_NEXT_TICK_NAME, 
			  IPC_VARIABLE_LENGTH, 
			  CARMEN_SIMULATOR_NEXT_TICK_FMT);
      carmen_test_ipc_exit(err, "Could not define message", 
			   CARMEN_SIMULATOR_NEXT_TICK_NAME);
      initialized = 1;
    }

  err = IPC_publishData(CARMEN_SIMULATOR_NEXT_TICK_NAME, &msg);
  carmen_test_ipc(err, "Could not publish", 
		  CARMEN_SIMULATOR_NEXT_TICK_NAME);
}

int 
carmen_simulator_set_truepose(carmen_point_t *point)
{
  IPC_RETURN_TYPE err = IPC_OK;
  carmen_simulator_set_truepose_message msg;
  static int initialized = 0;

  if (!initialized) 
    {
      err = IPC_defineMsg(CARMEN_SIMULATOR_SET_TRUEPOSE_NAME, 
			  IPC_VARIABLE_LENGTH, 
			  CARMEN_SIMULATOR_SET_TRUEPOSE_FMT);
      carmen_test_ipc_exit(err, "Could not define message", 
			   CARMEN_SIMULATOR_SET_TRUEPOSE_NAME);
      initialized = 1;
    }

  msg.pose = *point;
  msg.timestamp = carmen_get_time_ms();
  strcpy(msg.host, carmen_get_tenchar_host_name());

  err = IPC_publishData(CARMEN_SIMULATOR_SET_TRUEPOSE_NAME, &msg);
  carmen_test_ipc(err, "Could not publish", 
		  CARMEN_SIMULATOR_SET_TRUEPOSE_NAME);

  return 0;
}
