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

static carmen_laser_laser_message *frontlaser_message_pointer_external = NULL;
static carmen_laser_laser_message *rearlaser_message_pointer_external = NULL;
static carmen_laser_laser_message *laser3_message_pointer_external = NULL;
static carmen_laser_laser_message *laser4_message_pointer_external = NULL;
static carmen_laser_alive_message *alive_message_pointer_external = NULL;
static carmen_handler_t frontlaser_message_handler_external = NULL;
static carmen_handler_t rearlaser_message_handler_external = NULL;
static carmen_handler_t laser3_message_handler_external = NULL;
static carmen_handler_t laser4_message_handler_external = NULL;
static carmen_handler_t alive_message_handler_external = NULL;

static void 
frontlaser_interface_handler(MSG_INSTANCE msgRef, BYTE_ARRAY callData,
			     void *clientData __attribute__ ((unused)))
{
  IPC_RETURN_TYPE err = IPC_OK;
  FORMATTER_PTR formatter;
  
  formatter = IPC_msgInstanceFormatter(msgRef);

  if(frontlaser_message_pointer_external) 
    {
      if(frontlaser_message_pointer_external->range != NULL)
	free(frontlaser_message_pointer_external->range);
      
      err = IPC_unmarshallData(formatter, callData, 
			       frontlaser_message_pointer_external,
			       sizeof(carmen_laser_laser_message));
    }
  
  IPC_freeByteArray(callData);
  carmen_test_ipc_return(err, "Could not unmarshall", 
			 IPC_msgInstanceName(msgRef));
  if(frontlaser_message_handler_external)
    frontlaser_message_handler_external(frontlaser_message_pointer_external);
}

void
carmen_laser_subscribe_frontlaser_message(carmen_laser_laser_message *laser,
					  carmen_handler_t handler,
					  carmen_subscribe_t subscribe_how)
{
  IPC_RETURN_TYPE err = IPC_OK;
  
  err = IPC_defineMsg(CARMEN_LASER_FRONTLASER_NAME, 
		      IPC_VARIABLE_LENGTH, 
		      CARMEN_LASER_FRONTLASER_FMT);
  carmen_test_ipc_exit(err, "Could not define message", 
		       CARMEN_LASER_FRONTLASER_NAME);

  if(subscribe_how == CARMEN_UNSUBSCRIBE) 
    {
      IPC_unsubscribe(CARMEN_LASER_FRONTLASER_NAME, 
		      frontlaser_interface_handler);
      return;
    }

  if(laser) 
    {
      frontlaser_message_pointer_external = laser;
      memset(frontlaser_message_pointer_external, 0, 
	     sizeof(carmen_laser_laser_message));
    }
  else if (frontlaser_message_pointer_external == NULL) 
    {
      frontlaser_message_pointer_external = (carmen_laser_laser_message *)
	calloc(1, sizeof(carmen_laser_laser_message));
      carmen_test_alloc(frontlaser_message_pointer_external);
    }
  
  frontlaser_message_handler_external = handler;
  err = IPC_subscribe(CARMEN_LASER_FRONTLASER_NAME, 
		      frontlaser_interface_handler, NULL);
  if (subscribe_how == CARMEN_SUBSCRIBE_LATEST)
    IPC_setMsgQueueLength(CARMEN_LASER_FRONTLASER_NAME, 1);
  else
    IPC_setMsgQueueLength(CARMEN_LASER_FRONTLASER_NAME, 100);
  carmen_test_ipc(err, "Could not subscribe", CARMEN_LASER_FRONTLASER_NAME);
}

static void 
rearlaser_interface_handler(MSG_INSTANCE msgRef, BYTE_ARRAY callData,
			    void *clientData __attribute__ ((unused)))
{
  IPC_RETURN_TYPE err = IPC_OK;
  FORMATTER_PTR formatter;
  
  formatter = IPC_msgInstanceFormatter(msgRef);
  if(rearlaser_message_pointer_external) 
    {
      if(rearlaser_message_pointer_external->range != NULL)
	free(rearlaser_message_pointer_external->range);
      err = IPC_unmarshallData(formatter, callData, 
			       rearlaser_message_pointer_external,
			       sizeof(carmen_laser_laser_message));
    }

  IPC_freeByteArray(callData);
  carmen_test_ipc_return(err, "Could not unmarshall", 
			 IPC_msgInstanceName(msgRef));
  if(rearlaser_message_handler_external)
    rearlaser_message_handler_external(rearlaser_message_pointer_external);
}

void
carmen_laser_subscribe_rearlaser_message(carmen_laser_laser_message *laser,
					 carmen_handler_t handler,
					 carmen_subscribe_t subscribe_how)
{
  IPC_RETURN_TYPE err = IPC_OK;
  
  err = IPC_defineMsg(CARMEN_LASER_REARLASER_NAME, 
		      IPC_VARIABLE_LENGTH, 
		      CARMEN_LASER_REARLASER_FMT);
  carmen_test_ipc_exit(err, "Could not define message", 
		       CARMEN_LASER_REARLASER_NAME);

  if(subscribe_how == CARMEN_UNSUBSCRIBE) 
    {
      IPC_unsubscribe(CARMEN_LASER_REARLASER_NAME, 
		      rearlaser_interface_handler);
      return;
    }
  
  if(laser) 
    {
      rearlaser_message_pointer_external = laser;
      memset(rearlaser_message_pointer_external, 0, 
	     sizeof(carmen_laser_laser_message));
    }
  else if(rearlaser_message_pointer_external == NULL) 
    {
      rearlaser_message_pointer_external = (carmen_laser_laser_message *)
	calloc(1,sizeof(carmen_laser_laser_message));
      carmen_test_alloc(rearlaser_message_pointer_external);
    }
  rearlaser_message_handler_external = handler;
  err = IPC_subscribe(CARMEN_LASER_REARLASER_NAME, 
		      rearlaser_interface_handler, NULL);
  if(subscribe_how == CARMEN_SUBSCRIBE_LATEST)
    IPC_setMsgQueueLength(CARMEN_LASER_REARLASER_NAME, 1);
  else
    IPC_setMsgQueueLength(CARMEN_LASER_REARLASER_NAME, 100);
  carmen_test_ipc(err, "Could not subscribe", CARMEN_LASER_REARLASER_NAME);
}

void
carmen_laser_subscribe_laser1_message(carmen_laser_laser_message *laser,
				      carmen_handler_t handler,
				      carmen_subscribe_t subscribe_how)
{
  carmen_laser_subscribe_frontlaser_message(laser, handler, subscribe_how);
}

void
carmen_laser_subscribe_laser2_message(carmen_laser_laser_message *laser,
				      carmen_handler_t handler,
				      carmen_subscribe_t subscribe_how)
{
  carmen_laser_subscribe_rearlaser_message(laser, handler, subscribe_how);
}

static void 
laser3_interface_handler(MSG_INSTANCE msgRef, BYTE_ARRAY callData,
			 void *clientData __attribute__ ((unused)))
{
  IPC_RETURN_TYPE err = IPC_OK;
  FORMATTER_PTR formatter;
  
  formatter = IPC_msgInstanceFormatter(msgRef);
  if(laser3_message_pointer_external) 
    {
      if(laser3_message_pointer_external->range != NULL)
	free(laser3_message_pointer_external->range);
      err = IPC_unmarshallData(formatter, callData, 
			       laser3_message_pointer_external,
			       sizeof(carmen_laser_laser_message));
    }
  IPC_freeByteArray(callData);
  carmen_test_ipc_return(err, "Could not unmarshall", 
			 IPC_msgInstanceName(msgRef));
  if(laser3_message_handler_external)
    laser3_message_handler_external(laser3_message_pointer_external);
}

void
carmen_laser_subscribe_laser3_message(carmen_laser_laser_message *laser,
				      carmen_handler_t handler,
				      carmen_subscribe_t subscribe_how)
{
  IPC_RETURN_TYPE err = IPC_OK;
  
  err = IPC_defineMsg(CARMEN_LASER_LASER3_NAME, 
		      IPC_VARIABLE_LENGTH, 
		      CARMEN_LASER_LASER3_FMT);
  carmen_test_ipc_exit(err, "Could not define message", 
		       CARMEN_LASER_LASER3_NAME);

  if(subscribe_how == CARMEN_UNSUBSCRIBE) 
    {
      IPC_unsubscribe(CARMEN_LASER_LASER3_NAME, 
		      laser3_interface_handler);
      return;
    }
  if(laser) 
    {
      laser3_message_pointer_external = laser;
      memset(laser3_message_pointer_external, 0, 
	     sizeof(carmen_laser_laser_message));
    }
  else if(laser3_message_pointer_external == NULL) 
    {
      laser3_message_pointer_external = (carmen_laser_laser_message *)
	calloc(1,sizeof(carmen_laser_laser_message));
      carmen_test_alloc(laser3_message_pointer_external);
    }
  laser3_message_handler_external = handler;
  err = IPC_subscribe(CARMEN_LASER_LASER3_NAME, 
		      laser3_interface_handler, NULL);
  if(subscribe_how == CARMEN_SUBSCRIBE_LATEST)
    IPC_setMsgQueueLength(CARMEN_LASER_LASER3_NAME, 1);
  else
    IPC_setMsgQueueLength(CARMEN_LASER_LASER3_NAME, 100);
  carmen_test_ipc(err, "Could not subscribe", CARMEN_LASER_LASER3_NAME);
}

static void 
laser4_interface_handler(MSG_INSTANCE msgRef, BYTE_ARRAY callData,
			 void *clientData __attribute__ ((unused)))
{
  IPC_RETURN_TYPE err = IPC_OK;
  FORMATTER_PTR formatter;
  
  formatter = IPC_msgInstanceFormatter(msgRef);
  if(laser4_message_pointer_external) 
    {
      if(laser4_message_pointer_external->range != NULL)
	free(laser4_message_pointer_external->range);
      err = IPC_unmarshallData(formatter, callData, 
			       laser4_message_pointer_external,
			       sizeof(carmen_laser_laser_message));
    }
  IPC_freeByteArray(callData);
  carmen_test_ipc_return(err, "Could not unmarshall", 
			 IPC_msgInstanceName(msgRef));
  if(laser4_message_handler_external)
    laser4_message_handler_external(laser4_message_pointer_external);
}

void
carmen_laser_subscribe_laser4_message(carmen_laser_laser_message *laser,
				      carmen_handler_t handler,
				      carmen_subscribe_t subscribe_how)
{
  IPC_RETURN_TYPE err = IPC_OK;
  
  err = IPC_defineMsg(CARMEN_LASER_LASER4_NAME, 
		      IPC_VARIABLE_LENGTH, 
		      CARMEN_LASER_LASER4_FMT);
  carmen_test_ipc_exit(err, "Could not define message", 
		       CARMEN_LASER_LASER4_NAME);

  if(subscribe_how == CARMEN_UNSUBSCRIBE) 
    {
      IPC_unsubscribe(CARMEN_LASER_LASER4_NAME, 
		      laser4_interface_handler);
      return;
    }

  if(laser) 
    {
      laser4_message_pointer_external = laser;
      memset(laser4_message_pointer_external, 0, 
	     sizeof(carmen_laser_laser_message));
    }
  else if(laser4_message_pointer_external == NULL) 
    {
      laser4_message_pointer_external = (carmen_laser_laser_message *)
	calloc(1,sizeof(carmen_laser_laser_message));
      carmen_test_alloc(laser4_message_pointer_external);
    }

  laser4_message_handler_external = handler;
  err = IPC_subscribe(CARMEN_LASER_LASER4_NAME, 
		      laser4_interface_handler, NULL);
  if(subscribe_how == CARMEN_SUBSCRIBE_LATEST)
    IPC_setMsgQueueLength(CARMEN_LASER_LASER4_NAME, 1);
  else
    IPC_setMsgQueueLength(CARMEN_LASER_LASER4_NAME, 100);
  carmen_test_ipc(err, "Could not subscribe", CARMEN_LASER_LASER4_NAME);
}

static void 
alive_interface_handler(MSG_INSTANCE msgRef, BYTE_ARRAY callData,
			void *clientData __attribute__ ((unused)))
{
  IPC_RETURN_TYPE err = IPC_OK;
  
  if(alive_message_pointer_external) {
    err = IPC_unmarshallData(IPC_msgInstanceFormatter(msgRef), callData, 
			     alive_message_pointer_external,
			     sizeof(carmen_laser_alive_message));
  }
  IPC_freeByteArray(callData);
  carmen_test_ipc_return(err, "Could not unmarshall", 
			 IPC_msgInstanceName(msgRef));
  if(alive_message_handler_external)
    alive_message_handler_external(alive_message_pointer_external);
}

void
carmen_laser_subscribe_alive_message(carmen_laser_alive_message *alive,
				     carmen_handler_t handler,
				     carmen_subscribe_t subscribe_how)
{
  IPC_RETURN_TYPE err = IPC_OK;
  
  err = IPC_defineMsg(CARMEN_LASER_ALIVE_NAME, 
		      IPC_VARIABLE_LENGTH, 
		      CARMEN_LASER_ALIVE_FMT);
  carmen_test_ipc_exit(err, "Could not define message", 
		       CARMEN_LASER_ALIVE_NAME);

  if(subscribe_how == CARMEN_UNSUBSCRIBE) {
    IPC_unsubscribe(CARMEN_LASER_ALIVE_NAME, 
		    alive_interface_handler);
    return;
  }
  if(alive) {
    alive_message_pointer_external = alive;
    memset(alive_message_pointer_external, 0, 
	   sizeof(carmen_laser_alive_message));
  }
  else if(alive_message_pointer_external == NULL) {
    alive_message_pointer_external = (carmen_laser_alive_message *)
      calloc(1, sizeof(carmen_laser_alive_message));
    carmen_test_alloc(alive_message_pointer_external);
  }
  
  alive_message_handler_external = handler;
  err = IPC_subscribe(CARMEN_LASER_ALIVE_NAME, 
		      alive_interface_handler, NULL);
  if(subscribe_how == CARMEN_SUBSCRIBE_LATEST)
    IPC_setMsgQueueLength(CARMEN_LASER_ALIVE_NAME, 1);
  else
    IPC_setMsgQueueLength(CARMEN_LASER_ALIVE_NAME, 100);
  carmen_test_ipc(err, "Could not subscribe", CARMEN_LASER_ALIVE_NAME);
}


// *** REI - START *** //
static carmen_laser_remission_message *frontlaser_remission_message_pointer_external = NULL;
static carmen_laser_remission_message *rearlaser_remission_message_pointer_external = NULL;
static carmen_laser_remission_message *laser3_remission_message_pointer_external = NULL;
static carmen_laser_remission_message *laser4_remission_message_pointer_external = NULL;
static carmen_handler_t frontlaser_remission_message_handler_external = NULL;
static carmen_handler_t rearlaser_remission_message_handler_external = NULL;
static carmen_handler_t laser3_remission_message_handler_external = NULL;
static carmen_handler_t laser4_remission_message_handler_external = NULL;

static void 
frontlaser_remission_interface_handler(MSG_INSTANCE msgRef, BYTE_ARRAY callData,
			     void *clientData __attribute__ ((unused)))
{
  IPC_RETURN_TYPE err = IPC_OK;
  FORMATTER_PTR formatter;
  
  formatter = IPC_msgInstanceFormatter(msgRef);

  if(frontlaser_remission_message_pointer_external) 
    {
      if(frontlaser_remission_message_pointer_external->range != NULL)
	free(frontlaser_remission_message_pointer_external->range);
      
      err = IPC_unmarshallData(formatter, callData, 
			       frontlaser_remission_message_pointer_external,
			       sizeof(carmen_laser_remission_message));
    }
  
  IPC_freeByteArray(callData);
  carmen_test_ipc_return(err, "Could not unmarshall", 
			 IPC_msgInstanceName(msgRef));
  if(frontlaser_remission_message_handler_external)
    frontlaser_remission_message_handler_external(frontlaser_remission_message_pointer_external);
}

void
carmen_laser_subscribe_frontlaser_remission_message(carmen_laser_remission_message *laser,
					  carmen_handler_t handler,
					  carmen_subscribe_t subscribe_how)
{
  IPC_RETURN_TYPE err = IPC_OK;
  
  err = IPC_defineMsg(CARMEN_LASER_FRONTLASER_REMISSION_NAME, 
		      IPC_VARIABLE_LENGTH, 
		      CARMEN_LASER_FRONTLASER_REMISSION_FMT);
  carmen_test_ipc_exit(err, "Could not define message", 
		       CARMEN_LASER_FRONTLASER_REMISSION_NAME);

  if(subscribe_how == CARMEN_UNSUBSCRIBE) 
    {
      IPC_unsubscribe(CARMEN_LASER_FRONTLASER_REMISSION_NAME, 
		      frontlaser_remission_interface_handler);
      return;
    }

  if(laser) 
    {
      frontlaser_remission_message_pointer_external = laser;
      memset(frontlaser_remission_message_pointer_external, 0, 
	     sizeof(carmen_laser_remission_message));
    }
  else if (frontlaser_remission_message_pointer_external == NULL) 
    {
      frontlaser_remission_message_pointer_external = (carmen_laser_remission_message *)
	calloc(1, sizeof(carmen_laser_remission_message));
      carmen_test_alloc(frontlaser_remission_message_pointer_external);
    }
  
  frontlaser_remission_message_handler_external = handler;
  err = IPC_subscribe(CARMEN_LASER_FRONTLASER_REMISSION_NAME, 
		      frontlaser_remission_interface_handler, NULL);
  if (subscribe_how == CARMEN_SUBSCRIBE_LATEST)
    IPC_setMsgQueueLength(CARMEN_LASER_FRONTLASER_REMISSION_NAME, 1);
  else
    IPC_setMsgQueueLength(CARMEN_LASER_FRONTLASER_REMISSION_NAME, 100);
  carmen_test_ipc(err, "Could not subscribe", CARMEN_LASER_FRONTLASER_REMISSION_NAME);
}

static void 
rearlaser_remission_interface_handler(MSG_INSTANCE msgRef, BYTE_ARRAY callData,
			    void *clientData __attribute__ ((unused)))
{
  IPC_RETURN_TYPE err = IPC_OK;
  FORMATTER_PTR formatter;
  
  formatter = IPC_msgInstanceFormatter(msgRef);
  if(rearlaser_remission_message_pointer_external) 
    {
      if(rearlaser_remission_message_pointer_external->range != NULL)
	free(rearlaser_remission_message_pointer_external->range);
      err = IPC_unmarshallData(formatter, callData, 
			       rearlaser_remission_message_pointer_external,
			       sizeof(carmen_laser_remission_message));
    }

  IPC_freeByteArray(callData);
  carmen_test_ipc_return(err, "Could not unmarshall", 
			 IPC_msgInstanceName(msgRef));
  if(rearlaser_remission_message_handler_external)
    rearlaser_remission_message_handler_external(rearlaser_remission_message_pointer_external);
}

void
carmen_laser_subscribe_rearlaser_remission_message(carmen_laser_remission_message *laser,
					 carmen_handler_t handler,
					 carmen_subscribe_t subscribe_how)
{
  IPC_RETURN_TYPE err = IPC_OK;
  
  err = IPC_defineMsg(CARMEN_LASER_REARLASER_REMISSION_NAME, 
		      IPC_VARIABLE_LENGTH, 
		      CARMEN_LASER_REARLASER_REMISSION_FMT);
  carmen_test_ipc_exit(err, "Could not define message", 
		       CARMEN_LASER_REARLASER_REMISSION_NAME);

  if(subscribe_how == CARMEN_UNSUBSCRIBE) 
    {
      IPC_unsubscribe(CARMEN_LASER_REARLASER_REMISSION_NAME, 
		      rearlaser_remission_interface_handler);
      return;
    }
  
  if(laser) 
    {
      rearlaser_remission_message_pointer_external = laser;
      memset(rearlaser_remission_message_pointer_external, 0, 
	     sizeof(carmen_laser_remission_message));
    }
  else if(rearlaser_remission_message_pointer_external == NULL) 
    {
      rearlaser_remission_message_pointer_external = (carmen_laser_remission_message *)
	calloc(1,sizeof(carmen_laser_remission_message));
      carmen_test_alloc(rearlaser_remission_message_pointer_external);
    }
  rearlaser_remission_message_handler_external = handler;
  err = IPC_subscribe(CARMEN_LASER_REARLASER_REMISSION_NAME, 
		      rearlaser_remission_interface_handler, NULL);
  if(subscribe_how == CARMEN_SUBSCRIBE_LATEST)
    IPC_setMsgQueueLength(CARMEN_LASER_REARLASER_REMISSION_NAME, 1);
  else
    IPC_setMsgQueueLength(CARMEN_LASER_REARLASER_REMISSION_NAME, 100);
  carmen_test_ipc(err, "Could not subscribe", CARMEN_LASER_REARLASER_REMISSION_NAME);
}

void
carmen_laser_subscribe_laser1_remission_message(carmen_laser_remission_message *laser,
				      carmen_handler_t handler,
				      carmen_subscribe_t subscribe_how)
{
  carmen_laser_subscribe_frontlaser_remission_message(laser, handler, subscribe_how);
}

void
carmen_laser_subscribe_laser2_remission_message(carmen_laser_remission_message *laser,
				      carmen_handler_t handler,
				      carmen_subscribe_t subscribe_how)
{
  carmen_laser_subscribe_rearlaser_remission_message(laser, handler, subscribe_how);
}

static void 
laser3_remission_interface_handler(MSG_INSTANCE msgRef, BYTE_ARRAY callData,
			 void *clientData __attribute__ ((unused)))
{
  IPC_RETURN_TYPE err = IPC_OK;
  FORMATTER_PTR formatter;
  
  formatter = IPC_msgInstanceFormatter(msgRef);
  if(laser3_remission_message_pointer_external) 
    {
      if(laser3_remission_message_pointer_external->range != NULL)
	free(laser3_remission_message_pointer_external->range);
      err = IPC_unmarshallData(formatter, callData, 
			       laser3_remission_message_pointer_external,
			       sizeof(carmen_laser_remission_message));
    }
  IPC_freeByteArray(callData);
  carmen_test_ipc_return(err, "Could not unmarshall", 
			 IPC_msgInstanceName(msgRef));
  if(laser3_remission_message_handler_external)
    laser3_remission_message_handler_external(laser3_remission_message_pointer_external);
}

void
carmen_laser_subscribe_laser3_remission_message(carmen_laser_remission_message *laser,
				      carmen_handler_t handler,
				      carmen_subscribe_t subscribe_how)
{
  IPC_RETURN_TYPE err = IPC_OK;
  
  err = IPC_defineMsg(CARMEN_LASER_LASER3_REMISSION_NAME, 
		      IPC_VARIABLE_LENGTH, 
		      CARMEN_LASER_LASER3_REMISSION_FMT);
  carmen_test_ipc_exit(err, "Could not define message", 
		       CARMEN_LASER_LASER3_REMISSION_NAME);

  if(subscribe_how == CARMEN_UNSUBSCRIBE) 
    {
      IPC_unsubscribe(CARMEN_LASER_LASER3_REMISSION_NAME, 
		      laser3_remission_interface_handler);
      return;
    }
  if(laser) 
    {
      laser3_remission_message_pointer_external = laser;
      memset(laser3_remission_message_pointer_external, 0, 
	     sizeof(carmen_laser_remission_message));
    }
  else if(laser3_remission_message_pointer_external == NULL) 
    {
      laser3_remission_message_pointer_external = (carmen_laser_remission_message *)
	calloc(1,sizeof(carmen_laser_remission_message));
      carmen_test_alloc(laser3_remission_message_pointer_external);
    }
  laser3_remission_message_handler_external = handler;
  err = IPC_subscribe(CARMEN_LASER_LASER3_REMISSION_NAME, 
		      laser3_remission_interface_handler, NULL);
  if(subscribe_how == CARMEN_SUBSCRIBE_LATEST)
    IPC_setMsgQueueLength(CARMEN_LASER_LASER3_REMISSION_NAME, 1);
  else
    IPC_setMsgQueueLength(CARMEN_LASER_LASER3_REMISSION_NAME, 100);
  carmen_test_ipc(err, "Could not subscribe", CARMEN_LASER_LASER3_REMISSION_NAME);
}

static void 
laser4_remission_interface_handler(MSG_INSTANCE msgRef, BYTE_ARRAY callData,
			 void *clientData __attribute__ ((unused)))
{
  IPC_RETURN_TYPE err = IPC_OK;
  FORMATTER_PTR formatter;
  
  formatter = IPC_msgInstanceFormatter(msgRef);
  if(laser4_remission_message_pointer_external) 
    {
      if(laser4_remission_message_pointer_external->range != NULL)
	free(laser4_remission_message_pointer_external->range);
      err = IPC_unmarshallData(formatter, callData, 
			       laser4_remission_message_pointer_external,
			       sizeof(carmen_laser_remission_message));
    }
  IPC_freeByteArray(callData);
  carmen_test_ipc_return(err, "Could not unmarshall", 
			 IPC_msgInstanceName(msgRef));
  if(laser4_remission_message_handler_external)
    laser4_remission_message_handler_external(laser4_remission_message_pointer_external);
}

void
carmen_laser_subscribe_laser4_remission_message(carmen_laser_remission_message *laser,
				      carmen_handler_t handler,
				      carmen_subscribe_t subscribe_how)
{
  IPC_RETURN_TYPE err = IPC_OK;
  
  err = IPC_defineMsg(CARMEN_LASER_LASER4_REMISSION_NAME, 
		      IPC_VARIABLE_LENGTH, 
		      CARMEN_LASER_LASER4_REMISSION_FMT);
  carmen_test_ipc_exit(err, "Could not define message", 
		       CARMEN_LASER_LASER4_REMISSION_NAME);

  if(subscribe_how == CARMEN_UNSUBSCRIBE) 
    {
      IPC_unsubscribe(CARMEN_LASER_LASER4_REMISSION_NAME, 
		      laser4_remission_interface_handler);
      return;
    }

  if(laser) 
    {
      laser4_remission_message_pointer_external = laser;
      memset(laser4_remission_message_pointer_external, 0, 
	     sizeof(carmen_laser_remission_message));
    }
  else if(laser4_remission_message_pointer_external == NULL) 
    {
      laser4_remission_message_pointer_external = (carmen_laser_remission_message *)
	calloc(1,sizeof(carmen_laser_remission_message));
      carmen_test_alloc(laser4_remission_message_pointer_external);
    }

  laser4_remission_message_handler_external = handler;
  err = IPC_subscribe(CARMEN_LASER_LASER4_REMISSION_NAME, 
		      laser4_remission_interface_handler, NULL);
  if(subscribe_how == CARMEN_SUBSCRIBE_LATEST)
    IPC_setMsgQueueLength(CARMEN_LASER_LASER4_REMISSION_NAME, 1);
  else
    IPC_setMsgQueueLength(CARMEN_LASER_LASER4_REMISSION_NAME, 100);
  carmen_test_ipc(err, "Could not subscribe", CARMEN_LASER_LASER4_REMISSION_NAME);
}

// *** REI - END *** //


double 
carmen_laser_get_fov(int num_beams) {

  if (num_beams == 181) 
    return M_PI;                  /* 180 degrees */
  else if (num_beams == 180) 
    return M_PI / 180.0 * 179.0;  /* 179 degrees (last beam ignored)*/
  else if (num_beams == 361) 
    return M_PI;                  /* 180 degrees */
  else if (num_beams == 360) 
    return M_PI / 180.0 * 179.5 ; /* 179.5 degrees (last beam ignored)*/
  else if (num_beams == 401) 
    return M_PI / 180.0 * 100.0 ; /* 100.0 degrees */
  else if (num_beams == 400) 
    return M_PI / 100.0 * 99.75 ; /* 99.75 degrees (last beam ignored)*/
  else  
    return M_PI;                  /* assume 180 degrees */
}

double 
carmen_laser_get_angle_increment(int num_beams) {
  
  if (num_beams == 181 || num_beams == 180) 
    return  M_PI / 180.0; /* 1 degree = M_PI/180 */
  else if (num_beams == 361 || num_beams == 360) 
    return M_PI / 360.0;  /* 0.5 degrees = M_PI/360 */
  else if (num_beams == 401 || num_beams == 400) 
    return M_PI / 720.0;  /* 0.25 degrees = M_PI/720 */
  else 
    return carmen_laser_get_fov(num_beams) / 
      ((double) (num_beams-1));
}



