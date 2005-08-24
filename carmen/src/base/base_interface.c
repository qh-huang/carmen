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

static carmen_base_odometry_message *base_odometry_pointer_external = NULL;
static carmen_base_sonar_message *base_sonar_pointer_external = NULL;
static carmen_base_reset_message *base_reset_pointer_external = NULL;
static carmen_handler_t base_odometry_handler_external = NULL;
static carmen_handler_t base_sonar_handler_external = NULL;
static carmen_handler_t base_reset_handler_external = NULL;

static void 
odometry_interface_handler(MSG_INSTANCE msgRef, BYTE_ARRAY callData,
			   void *clientData __attribute__ ((unused)))
{
  IPC_RETURN_TYPE err = IPC_OK;
  FORMATTER_PTR formatter;
  
  formatter = IPC_msgInstanceFormatter(msgRef);
  if (base_odometry_pointer_external)
    err = IPC_unmarshallData(formatter, callData, 
			     base_odometry_pointer_external, 
			     sizeof(carmen_base_odometry_message));
  IPC_freeByteArray(callData);

  carmen_test_ipc_return(err, "Could not unmarshall data", 
			 IPC_msgInstanceName(msgRef));

  if (base_odometry_handler_external)
    base_odometry_handler_external(base_odometry_pointer_external);
}

void
carmen_base_subscribe_odometry_message(carmen_base_odometry_message *odometry,
				       carmen_handler_t handler,
				       carmen_subscribe_t subscribe_how)
{
  IPC_RETURN_TYPE err = IPC_OK;

  err = IPC_defineMsg(CARMEN_BASE_ODOMETRY_NAME, IPC_VARIABLE_LENGTH, 
		      CARMEN_BASE_ODOMETRY_FMT);
  carmen_test_ipc_exit(err, "Could not define message", 
		       CARMEN_BASE_ODOMETRY_NAME);

  if (subscribe_how == CARMEN_UNSUBSCRIBE) 
    {
      IPC_unsubscribe(CARMEN_BASE_ODOMETRY_NAME, odometry_interface_handler);
      return;
    }

  if(odometry)
    base_odometry_pointer_external=odometry;
  else if (base_odometry_pointer_external == NULL) 
    {			
      base_odometry_pointer_external=
	(carmen_base_odometry_message *)
	calloc(1, sizeof(carmen_base_odometry_message));
      carmen_test_alloc(base_odometry_pointer_external);
    }

  base_odometry_handler_external=handler;

  err=IPC_subscribe(CARMEN_BASE_ODOMETRY_NAME, 
		    odometry_interface_handler, NULL);
  if(subscribe_how==CARMEN_SUBSCRIBE_LATEST)
    IPC_setMsgQueueLength(CARMEN_BASE_ODOMETRY_NAME,1);
  else
    IPC_setMsgQueueLength(CARMEN_BASE_ODOMETRY_NAME, 100);

  carmen_test_ipc(err, "Could not subscribe", CARMEN_BASE_ODOMETRY_NAME);
}

static void 
sonar_interface_handler(MSG_INSTANCE msgRef, BYTE_ARRAY callData,
			void *clientData __attribute__ ((unused)))
{
  IPC_RETURN_TYPE err = IPC_OK;
  FORMATTER_PTR formatter;
  
  formatter = IPC_msgInstanceFormatter(msgRef);
  if (base_sonar_pointer_external)
    err = IPC_unmarshallData(formatter, callData, base_sonar_pointer_external,
			     sizeof(carmen_base_sonar_message));
  IPC_freeByteArray(callData);
	
  carmen_test_ipc_return(err, "Could not unmarshall data", 
			 CARMEN_BASE_SONAR_NAME);
  
  if (base_sonar_handler_external)
    base_sonar_handler_external(base_sonar_pointer_external);
}

void
carmen_base_subscribe_sonar_message(carmen_base_sonar_message *sonar,
				    carmen_handler_t handler,
				    carmen_subscribe_t subscribe_how)
{
  IPC_RETURN_TYPE err = IPC_OK;

  err = IPC_defineMsg(CARMEN_BASE_SONAR_NAME, IPC_VARIABLE_LENGTH, 
		      CARMEN_BASE_SONAR_FMT);
  carmen_test_ipc_exit(err, "Could not define message", 
		       CARMEN_BASE_SONAR_NAME);

  if (subscribe_how == CARMEN_UNSUBSCRIBE) 
    {
      IPC_unsubscribe(CARMEN_BASE_SONAR_NAME, sonar_interface_handler);
      return;
    }

  if(sonar)
    base_sonar_pointer_external=sonar;
  else if (base_sonar_pointer_external == NULL) 
    {
      base_sonar_pointer_external=
	(carmen_base_sonar_message *)
	calloc(1, sizeof(carmen_base_sonar_message));
      carmen_test_alloc(base_odometry_pointer_external);
    }

  base_sonar_handler_external=handler;

  err=IPC_subscribe(CARMEN_BASE_SONAR_NAME, sonar_interface_handler, NULL);
  if(subscribe_how==CARMEN_SUBSCRIBE_LATEST)
    IPC_setMsgQueueLength(CARMEN_BASE_SONAR_NAME,1);
  else
    IPC_setMsgQueueLength(CARMEN_BASE_SONAR_NAME,100);
  carmen_test_ipc(err, "Could not subscribe", CARMEN_BASE_SONAR_NAME);
}

static void 
reset_interface_handler(MSG_INSTANCE msgRef, BYTE_ARRAY callData,
			void *clientData __attribute__ ((unused)))
{
  IPC_RETURN_TYPE err = IPC_OK;
  FORMATTER_PTR formatter;
  
  formatter = IPC_msgInstanceFormatter(msgRef);
  if (base_reset_pointer_external)
    err = IPC_unmarshallData(formatter, callData, base_reset_pointer_external,
			     sizeof(carmen_base_reset_message));
  IPC_freeByteArray(callData);
	
  carmen_test_ipc_return(err, "Could not unmarshall data", 
			 CARMEN_BASE_RESET_NAME);
  
  if (base_reset_handler_external)
    base_reset_handler_external(base_reset_pointer_external);
}

void
carmen_base_subscribe_reset_message(carmen_base_reset_message *reset,
				    carmen_handler_t handler,
				    carmen_subscribe_t subscribe_how)
{
  IPC_RETURN_TYPE err = IPC_OK;

  err = IPC_defineMsg(CARMEN_BASE_RESET_NAME, IPC_VARIABLE_LENGTH, 
		      CARMEN_BASE_RESET_FMT);
  carmen_test_ipc_exit(err, "Could not define message", 
		       CARMEN_BASE_RESET_NAME);

  if (subscribe_how == CARMEN_UNSUBSCRIBE) 
    {
      IPC_unsubscribe(CARMEN_BASE_RESET_NAME, reset_interface_handler);
      return;
    }

  if(reset)
    base_reset_pointer_external=reset;
  else if (base_reset_pointer_external == NULL) 
    {
      base_reset_pointer_external=
	(carmen_base_reset_message *)
	calloc(1, sizeof(carmen_base_reset_message));
      carmen_test_alloc(base_odometry_pointer_external);
    }

  base_reset_handler_external=handler;

  err=IPC_subscribe(CARMEN_BASE_RESET_NAME, reset_interface_handler, NULL);
  if(subscribe_how==CARMEN_SUBSCRIBE_LATEST)
    IPC_setMsgQueueLength(CARMEN_BASE_RESET_NAME,1);
  else
    IPC_setMsgQueueLength(CARMEN_BASE_RESET_NAME,100);
  carmen_test_ipc(err, "Could not subscribe", CARMEN_BASE_RESET_NAME);
}

void 
carmen_base_reset(void)
{
  IPC_RETURN_TYPE err;
  char *host;
  static carmen_base_reset_message msg;
  static int first = 1;

  if(first) {
    host = carmen_get_tenchar_host_name();
    strcpy(msg.host, host);

    err = IPC_defineMsg(CARMEN_BASE_RESET_COMMAND_NAME, 
			IPC_VARIABLE_LENGTH, 
			CARMEN_BASE_RESET_COMMAND_FMT);
    carmen_test_ipc_exit(err, "Could not define message", 
			 CARMEN_BASE_RESET_COMMAND_NAME);
    
    first = 0;
  }

  msg.timestamp = carmen_get_time();

  err = IPC_publishData(CARMEN_BASE_RESET_COMMAND_NAME, &msg);
  carmen_test_ipc(err, "Could not publish", CARMEN_BASE_RESET_COMMAND_NAME);
}
