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
 * Public License along with Foobar; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place, 
 * Suite 330, Boston, MA  02111-1307 USA
 *
 ********************************************************/

/*********************************************************
This file contains a code for gps-related functions that can be used
by other modules.
**********************************************************/

#include <carmen/carmen.h>
#include "gps_messages.h"

static carmen_gps_position_message *gps_position_pointer_external = NULL;
static carmen_handler_t gps_position_handler_external = NULL;

static void gps_position_interface_handler(MSG_INSTANCE msgRef,
					   BYTE_ARRAY callData,
			 void *clientData __attribute__ ((unused)))
{
  IPC_RETURN_TYPE err = IPC_OK;
  FORMATTER_PTR formatter;
  
  formatter = IPC_msgInstanceFormatter(msgRef);
  if (gps_position_pointer_external)
    err = IPC_unmarshallData(formatter, callData, 
			     gps_position_pointer_external, 
			     sizeof(carmen_gps_position_message));
  IPC_freeByteArray(callData);

  carmen_test_ipc_return(err, "Could not unmarshall data", 
		       IPC_msgInstanceName(msgRef));

  if (gps_position_handler_external)
    gps_position_handler_external(gps_position_pointer_external);
}


void carmen_gps_subscribe_position_message(carmen_gps_position_message 
					   *gps_position_msg,
					   carmen_handler_t handler,
					   carmen_subscribe_t subscribe_how)
{
  IPC_RETURN_TYPE err = IPC_OK;

  if (subscribe_how == CARMEN_UNSUBSCRIBE) 
    {
      IPC_unsubscribe(CARMEN_GPS_POSITION_NAME, 
		      gps_position_interface_handler);
      return;
    }

  if(gps_position_msg)
    {
      gps_position_pointer_external = gps_position_msg;
      memset(gps_position_pointer_external, 0, 
	     sizeof(carmen_gps_position_message));
    }
  else if (gps_position_pointer_external == NULL) 
    {			
      gps_position_pointer_external =
	(carmen_gps_position_message *)calloc(1, sizeof(carmen_gps_position_message));
      carmen_test_alloc(gps_position_pointer_external);
    }

  gps_position_handler_external = handler;

  err=IPC_subscribe(CARMEN_GPS_POSITION_NAME, gps_position_interface_handler, 
		    NULL);
  if(subscribe_how==CARMEN_SUBSCRIBE_LATEST)
    IPC_setMsgQueueLength(CARMEN_GPS_POSITION_NAME,1);

  carmen_test_ipc(err, "Could not subscribe", CARMEN_GPS_POSITION_NAME);
}

