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

#include "segway_messages.h"

static carmen_segway_pose_message *segway_pose_pointer_external = NULL;
static carmen_segway_battery_message *segway_battery_pointer_external = NULL;
static carmen_handler_t segway_pose_handler_external = NULL;
static carmen_handler_t segway_battery_handler_external = NULL;
static carmen_handler_t alive_message_handler_external = NULL;

static void 
segway_pose_interface_handler(MSG_INSTANCE msgRef, BYTE_ARRAY callData,
			      void *clientData __attribute__ ((unused)))
{
  IPC_RETURN_TYPE err = IPC_OK;
  
  if(segway_pose_pointer_external)
    err = IPC_unmarshallData(IPC_msgInstanceFormatter(msgRef), callData, 
			     segway_pose_pointer_external,
			     sizeof(carmen_segway_pose_message));
  IPC_freeByteArray(callData);
  carmen_test_ipc_return(err, "Could not unmarshall data", 
                         IPC_msgInstanceName(msgRef));
  if(segway_pose_handler_external)
    segway_pose_handler_external(segway_pose_pointer_external);
}

void
carmen_segway_subscribe_pose_message(carmen_segway_pose_message *pose,
				     carmen_handler_t handler,
				     carmen_subscribe_t subscribe_how)
{
  IPC_RETURN_TYPE err = IPC_OK;

  err = IPC_defineMsg(CARMEN_SEGWAY_POSE_NAME, IPC_VARIABLE_LENGTH, 
                      CARMEN_SEGWAY_POSE_FMT);
  carmen_test_ipc_exit(err, "Could not define message", 
                       CARMEN_SEGWAY_POSE_NAME);
  if(subscribe_how == CARMEN_UNSUBSCRIBE) {
    IPC_unsubscribe(CARMEN_SEGWAY_POSE_NAME, segway_pose_interface_handler);
    return;
  }

  if(pose)
    segway_pose_pointer_external = pose;
  else if(segway_pose_pointer_external == NULL) {                   
    segway_pose_pointer_external =
      (carmen_segway_pose_message *)
      calloc(1, sizeof(carmen_segway_pose_message));
    carmen_test_alloc(segway_pose_pointer_external);
  }
  segway_pose_handler_external = handler;
  err = IPC_subscribe(CARMEN_SEGWAY_POSE_NAME, 
		      segway_pose_interface_handler, NULL);
  if(subscribe_how == CARMEN_SUBSCRIBE_LATEST)
    IPC_setMsgQueueLength(CARMEN_SEGWAY_POSE_NAME, 1);
  else
    IPC_setMsgQueueLength(CARMEN_SEGWAY_POSE_NAME, 100);
  carmen_test_ipc(err, "Could not subscribe", CARMEN_SEGWAY_POSE_NAME);
}

static void 
segway_battery_interface_handler(MSG_INSTANCE msgRef, BYTE_ARRAY callData,
				 void *clientData __attribute__ ((unused)))
{
  IPC_RETURN_TYPE err = IPC_OK;
  
  if(segway_battery_pointer_external)
    err = IPC_unmarshallData(IPC_msgInstanceFormatter(msgRef), callData, 
			     segway_battery_pointer_external,
			     sizeof(carmen_segway_battery_message));
  IPC_freeByteArray(callData);
  carmen_test_ipc_return(err, "Could not unmarshall data", 
                         IPC_msgInstanceName(msgRef));
  if(segway_battery_handler_external)
    segway_battery_handler_external(segway_battery_pointer_external);
}

void
carmen_segway_subscribe_battery_message(carmen_segway_battery_message *battery,
					carmen_handler_t handler,
					carmen_subscribe_t subscribe_how)
{
  IPC_RETURN_TYPE err = IPC_OK;

  err = IPC_defineMsg(CARMEN_SEGWAY_BATTERY_NAME, IPC_VARIABLE_LENGTH, 
                      CARMEN_SEGWAY_BATTERY_FMT);
  carmen_test_ipc_exit(err, "Could not define message", 
                       CARMEN_SEGWAY_BATTERY_NAME);
  if(subscribe_how == CARMEN_UNSUBSCRIBE) {
    IPC_unsubscribe(CARMEN_SEGWAY_BATTERY_NAME, 
		    segway_battery_interface_handler);
    return;
  }
  
  if(battery)
    segway_battery_pointer_external = battery;
  else if(segway_battery_pointer_external == NULL) {                   
    segway_battery_pointer_external =
      (carmen_segway_battery_message *)
      calloc(1, sizeof(carmen_segway_battery_message));
    carmen_test_alloc(segway_battery_pointer_external);
  }
  segway_battery_handler_external = handler;
  err = IPC_subscribe(CARMEN_SEGWAY_BATTERY_NAME, 
		      segway_battery_interface_handler, NULL);
  if(subscribe_how == CARMEN_SUBSCRIBE_LATEST)
    IPC_setMsgQueueLength(CARMEN_SEGWAY_BATTERY_NAME, 1);
  else
    IPC_setMsgQueueLength(CARMEN_SEGWAY_BATTERY_NAME, 100);
  carmen_test_ipc(err, "Could not subscribe", CARMEN_SEGWAY_BATTERY_NAME);
}

static void 
alive_interface_handler(MSG_INSTANCE msgRef __attribute__ ((unused)),
                         BYTE_ARRAY callData __attribute__ ((unused)),
                         void *clientData __attribute__ ((unused)))
{
  if(alive_message_handler_external)
    alive_message_handler_external(NULL);
}

void
carmen_segway_subscribe_alive_message(carmen_handler_t handler,
				      carmen_subscribe_t subscribe_how)
{
  IPC_RETURN_TYPE err = IPC_OK;
  
  err = IPC_defineMsg(CARMEN_SEGWAY_ALIVE_NAME, IPC_VARIABLE_LENGTH, 
                      CARMEN_SEGWAY_ALIVE_FMT);
  carmen_test_ipc_exit(err, "Could not define message", 
                       CARMEN_SEGWAY_ALIVE_NAME);
  if(subscribe_how == CARMEN_UNSUBSCRIBE) {
    IPC_unsubscribe(CARMEN_SEGWAY_ALIVE_NAME, 
                    alive_interface_handler);
    return;
  }
  
  alive_message_handler_external = handler;
  err = IPC_subscribe(CARMEN_SEGWAY_ALIVE_NAME, 
                      alive_interface_handler, NULL);
  if (subscribe_how == CARMEN_SUBSCRIBE_LATEST)
    IPC_setMsgQueueLength(CARMEN_SEGWAY_ALIVE_NAME, 1);
  else
    IPC_setMsgQueueLength(CARMEN_SEGWAY_ALIVE_NAME, 100);
  carmen_test_ipc(err, "Could not subscribe", CARMEN_SEGWAY_ALIVE_NAME);
}

