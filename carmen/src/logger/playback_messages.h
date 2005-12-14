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

#ifdef __cplusplus
extern "C" {
#endif

#ifndef    CARMEN_PLAYBACK_MESSAGES_H
#define    CARMEN_PLAYBACK_MESSAGES_H

#define    CARMEN_PLAYBACK_COMMAND_PLAY         0
#define    CARMEN_PLAYBACK_COMMAND_STOP         1
#define    CARMEN_PLAYBACK_COMMAND_RESET        2
#define    CARMEN_PLAYBACK_COMMAND_FORWARD      3
#define    CARMEN_PLAYBACK_COMMAND_REWIND       4
#define    CARMEN_PLAYBACK_COMMAND_FWD_SINGLE   5
#define    CARMEN_PLAYBACK_COMMAND_RWD_SINGLE   6
#define    CARMEN_PLAYBACK_COMMAND_SET_SPEED    7

typedef struct {
  int cmd, arg;
  float speed;
} carmen_playback_command_message;

#define CARMEN_PLAYBACK_COMMAND_NAME     "carmen_playback_command"
#define CARMEN_PLAYBACK_COMMAND_FMT      "{int,int,float}"

#endif

#ifdef __cplusplus
}
#endif
