/*********************************************************
 *
 * This source code is part of the Carnegie Mellon Robot
 * Navigation Toolkit (CARMEN)
 *
 * CARMEN Copyright (c) 2002 Michael Montemerlo, Nicholas
 * Roy, and Sebastian Thrun
 *
 * This module (panlaser) Copyright (c) 2003 Brian Gerkey
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
#include "panlaser_messages.h"

static carmen_panlaser_scan_message* panlaser_scan_message_pointer_external=NULL;
static carmen_handler_t scan_message_handler_external=NULL;

static void
scan_interface_handler(MSG_INSTANCE msgRef, BYTE_ARRAY callData,
                                void* clientData __attribute__ ((unused)))
{
  IPC_RETURN_TYPE err = IPC_OK;
  FORMATTER_PTR formatter;

  formatter = IPC_msgInstanceFormatter(msgRef);

  if(panlaser_scan_message_pointer_external) 
  {
    if(panlaser_scan_message_pointer_external->range != NULL)
      free(panlaser_scan_message_pointer_external->range);

    err = IPC_unmarshallData(formatter, callData, 
                             panlaser_scan_message_pointer_external,
                             sizeof(carmen_panlaser_scan_message));
  }

  IPC_freeByteArray(callData);
  carmen_test_ipc_return(err, "Could not unmarshall",
                         IPC_msgInstanceName(msgRef));
  if(scan_message_handler_external)
    scan_message_handler_external(panlaser_scan_message_pointer_external);
}

void
carmen_panlaser_subscribe_scan_message(carmen_panlaser_scan_message* scan,
                                       carmen_handler_t handler,
                                       carmen_subscribe_t subscribe_how)
{
  IPC_RETURN_TYPE err = IPC_OK;

  err = IPC_defineMsg(CARMEN_PANLASER_SCAN_NAME,
                      IPC_VARIABLE_LENGTH,
                      CARMEN_PANLASER_SCAN_FMT);
  carmen_test_ipc_exit(err, "Could not define message",
                       CARMEN_PANLASER_SCAN_NAME);

  if(subscribe_how == CARMEN_UNSUBSCRIBE)
  {
    IPC_unsubscribe(CARMEN_PANLASER_SCAN_NAME,
                    scan_interface_handler);
    return;
  }

  if(scan)
  {
    panlaser_scan_message_pointer_external = scan;
    memset(panlaser_scan_message_pointer_external, 0,
           sizeof(carmen_panlaser_scan_message));
  }
  else if(panlaser_scan_message_pointer_external == NULL)
  {
    panlaser_scan_message_pointer_external = (carmen_panlaser_scan_message*)
            calloc(1, sizeof(carmen_panlaser_scan_message));
    carmen_test_alloc(panlaser_scan_message_pointer_external);
  }

  scan_message_handler_external = handler;
  err = IPC_subscribe(CARMEN_PANLASER_SCAN_NAME,
                      scan_interface_handler, NULL);
  if(subscribe_how == CARMEN_SUBSCRIBE_LATEST)
    IPC_setMsgQueueLength(CARMEN_PANLASER_SCAN_NAME, 1);
  else
    IPC_setMsgQueueLength(CARMEN_PANLASER_SCAN_NAME, 100);
  carmen_test_ipc(err, "Could not subscribe", CARMEN_PANLASER_SCAN_NAME);
}


