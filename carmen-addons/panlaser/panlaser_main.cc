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
#include <carmen/laser_interface.h>
#include "panlaser_messages.h"
#include "../segway/segway_interface.h"
#include "../amtec/amtec_interface.h"
#include "GzVector.hh"

#define DEBUG 1

static bool g_laser_newp;
static carmen_laser_laser_message g_laser_msg;
static carmen_segway_pose_message g_segway_msg;
static carmen_amtec_status_message g_amtec_msg;

/* robot's pose in the world (updated as position data arrives) */
static GzVector  g_robot_pos;
static GzQuatern g_robot_rot;

/* ptu's pose on the robot (fixed; can be set in configuration file) */
static GzVector  g_ptu_mount_pos;
static GzQuatern g_ptu_mount_rot;

/* pose of ptu's end effector (updated as ptu data arrives) */
static GzVector  g_ptu_pos;
static GzQuatern g_ptu_rot;

/* pose of laser on ptu's end effector (fixed; can be set in 
 * configuration file) */
static GzVector  g_laser_mount_pos;
static GzQuatern g_laser_mount_rot;

static void
set_default_parameters()
{
  /* (x,y,z) in meters */
  g_robot_pos = GzVectorSet(0.0,0.0,0.0);
  /* (roll,pitch,yaw) in radians */
  g_robot_rot = GzQuaternFromEuler(0.0,0.0,0.0);
  
  /* (x,y,z) in meters */
  g_ptu_mount_pos = GzVectorSet(0.0,0.0,0.0);
  /* (roll,pitch,yaw) in radians */
  g_ptu_mount_rot = GzQuaternFromEuler(0.0,0.0,0.0);

  /* (x,y,z) in meters */
  g_ptu_pos = GzVectorSet(0.0,0.0,0.0);
  /* (roll,pitch,yaw) in radians */
  g_ptu_rot = GzQuaternFromEuler(0.0,0.0,0.0);

  /* (x,y,z) in meters */
  g_laser_mount_pos = GzVectorSet(0.0,0.0,0.0);
  /* (roll,pitch,yaw) in radians */
  g_laser_mount_rot = GzQuaternFromEuler(0.0,0.0,0.0);
}

static void
read_parameters(int argc, char **argv)
{
  double ptu_x,ptu_y,ptu_z,ptu_roll,ptu_pitch,ptu_yaw;
  double laser_x,laser_y,laser_z,laser_roll,laser_pitch,laser_yaw;

  carmen_param_t panlaser_offsets[] = {
    {"panlaser", "ptu_x", CARMEN_PARAM_DOUBLE, &ptu_x, 0, NULL},
    {"panlaser", "ptu_y", CARMEN_PARAM_DOUBLE, &ptu_y, 0, NULL},
    {"panlaser", "ptu_z", CARMEN_PARAM_DOUBLE, &ptu_z, 0, NULL},
    {"panlaser", "ptu_roll", CARMEN_PARAM_DOUBLE, &ptu_roll, 0, NULL},
    {"panlaser", "ptu_pitch", CARMEN_PARAM_DOUBLE, &ptu_pitch, 0, NULL},
    {"panlaser", "ptu_yaw", CARMEN_PARAM_DOUBLE, &ptu_yaw, 0, NULL},
    {"panlaser", "laser_x", CARMEN_PARAM_DOUBLE, &laser_x, 0, NULL},
    {"panlaser", "laser_y", CARMEN_PARAM_DOUBLE, &laser_y, 0, NULL},
    {"panlaser", "laser_z", CARMEN_PARAM_DOUBLE, &laser_z, 0, NULL},
    {"panlaser", "laser_roll", CARMEN_PARAM_DOUBLE, &laser_roll, 0, NULL},
    {"panlaser", "laser_pitch", CARMEN_PARAM_DOUBLE, &laser_pitch, 0, NULL},
    {"panlaser", "laser_yaw", CARMEN_PARAM_DOUBLE, &laser_yaw, 0, NULL}
  };

  carmen_param_install_params(argc, argv, panlaser_offsets, 
			      sizeof(panlaser_offsets) / 
                              sizeof(panlaser_offsets[0]));

  g_ptu_mount_pos = GzVectorSet(ptu_x,ptu_y,ptu_z);
  g_ptu_mount_rot = GzQuaternFromEuler(ptu_roll,ptu_pitch,ptu_yaw);
  g_laser_mount_pos = GzVectorSet(laser_x,laser_y,laser_z);
  g_laser_mount_rot = GzQuaternFromEuler(laser_roll,laser_pitch,laser_yaw);
}

static void
laser_frontlaser_handler(MSG_INSTANCE msgRef, BYTE_ARRAY callData,
                         void *clientData)
{
  // Use clientData so that the build doesn't barf on the unused variable
  // ( g++ doesn't support __attribute__ ((unused)) ).
  if(clientData);

  IPC_RETURN_TYPE err = IPC_OK;
  FORMATTER_PTR formatter;

  formatter = IPC_msgInstanceFormatter(msgRef);
  err = IPC_unmarshallData(formatter, callData, &g_laser_msg,
                           sizeof(carmen_laser_laser_message));
  IPC_freeByteArray(callData);

  carmen_test_ipc_return(err, "Could not unmarshall",
                         IPC_msgInstanceName(msgRef));

  g_laser_newp = true;
}

static void
segway_pose_handler(MSG_INSTANCE msgRef, BYTE_ARRAY callData,
                    void *clientData)
{
  // Use clientData so that the build doesn't barf on the unused variable
  // ( g++ doesn't support __attribute__ ((unused)) ).
  if(clientData);

  IPC_RETURN_TYPE err = IPC_OK;
  FORMATTER_PTR formatter;

  formatter = IPC_msgInstanceFormatter(msgRef);
  err = IPC_unmarshallData(formatter, callData, &g_segway_msg,
                           sizeof(carmen_segway_pose_message));
  IPC_freeByteArray(callData);

  carmen_test_ipc_return(err, "Could not unmarshall",
                         IPC_msgInstanceName(msgRef));

  /* store the robot's pose for use in carmen_panlaser_run() */
  g_robot_pos = GzVectorSet(g_segway_msg.x, g_segway_msg.y, 0.0);
  g_robot_rot = GzQuaternFromEuler(g_segway_msg.roll,
                                   g_segway_msg.pitch,
                                   g_segway_msg.theta);
}

static void
amtec_status_handler(MSG_INSTANCE msgRef, BYTE_ARRAY callData,
                     void *clientData)
{
  // Use clientData so that the build doesn't barf on the unused variable
  // ( g++ doesn't support __attribute__ ((unused)) ).
  if(clientData);

  IPC_RETURN_TYPE err = IPC_OK;
  FORMATTER_PTR formatter;

  formatter = IPC_msgInstanceFormatter(msgRef);
  err = IPC_unmarshallData(formatter, callData, &g_amtec_msg,
                           sizeof(carmen_amtec_status_message));
  IPC_freeByteArray(callData);

  carmen_test_ipc_return(err, "Could not unmarshall",
                         IPC_msgInstanceName(msgRef));

  /* store the amtec's pose for use in carmen_panlaser_run() */
  g_robot_pos = GzVectorSet(0.0,0.0,0.0);
  g_robot_rot = GzQuaternFromEuler(0.0, g_amtec_msg.tilt, g_amtec_msg.pan);
}

static void
ipc_initialize_messages(void)
{
  IPC_RETURN_TYPE err;

  /* outgoing message */
  err = IPC_defineMsg(CARMEN_PANLASER_SCAN_NAME,
                      IPC_VARIABLE_LENGTH,
                      CARMEN_PANLASER_SCAN_FMT);
  carmen_test_ipc_exit(err, "Could not define message",
                       CARMEN_PANLASER_SCAN_NAME);

  /* incoming messages */
  err = IPC_defineMsg(CARMEN_LASER_FRONTLASER_NAME,
                      IPC_VARIABLE_LENGTH,
                      CARMEN_LASER_FRONTLASER_FMT);
  carmen_test_ipc_exit(err,"Could not define",CARMEN_LASER_FRONTLASER_NAME);

  err = IPC_subscribe(CARMEN_LASER_FRONTLASER_NAME,
                      laser_frontlaser_handler,NULL);
  carmen_test_ipc_exit(err,"Could not subscribe",CARMEN_LASER_FRONTLASER_NAME);

  err = IPC_defineMsg(CARMEN_SEGWAY_POSE_NAME,
                      IPC_VARIABLE_LENGTH,
                      CARMEN_SEGWAY_POSE_FMT);
  carmen_test_ipc_exit(err, "Could not define", CARMEN_SEGWAY_POSE_NAME);

  err = IPC_subscribe(CARMEN_SEGWAY_POSE_NAME,
                      segway_pose_handler,NULL);
  carmen_test_ipc_exit(err,"Could not subscribe",CARMEN_SEGWAY_POSE_NAME);

  err = IPC_defineMsg(CARMEN_AMTEC_STATUS_NAME,
                      IPC_VARIABLE_LENGTH,
                      CARMEN_AMTEC_STATUS_FMT);
  carmen_test_ipc_exit(err, "Could not define", CARMEN_AMTEC_STATUS_NAME);

  err = IPC_subscribe(CARMEN_AMTEC_STATUS_NAME,
                      amtec_status_handler,NULL);
  carmen_test_ipc_exit(err,"Could not subscribe",CARMEN_AMTEC_STATUS_NAME);
}

void
carmen_panlaser_start(int argc, char** argv)
{
  ipc_initialize_messages();
  set_default_parameters();
  read_parameters(argc,argv);
}

void
carmen_panlaser_run(void)
{
  double roll, pitch, yaw;
  GzVector  laser_pos;
  GzQuatern laser_rot;
  carmen_panlaser_scan_message msg;

#if DEBUG
  static char firsttime = 1;
  if(firsttime)
  {
    puts("ptu offsets:");
    printf("x:\t%lf\n", g_ptu_mount_pos.x);
    printf("y:\t%lf\n", g_ptu_mount_pos.y);
    printf("z:\t%lf\n", g_ptu_mount_pos.z);
    GzQuaternToEuler(&roll, &pitch, &yaw, g_ptu_mount_rot);
    printf("roll:\t%lf\n", carmen_radians_to_degrees(roll));
    printf("pitch:\t%lf\n", carmen_radians_to_degrees(pitch));
    printf("yaw:\t%lf\n", carmen_radians_to_degrees(yaw));

    puts("laser offsets:");
    printf("x:\t%lf\n", g_laser_mount_pos.x);
    printf("y:\t%lf\n", g_laser_mount_pos.y);
    printf("z:\t%lf\n", g_laser_mount_pos.z);
    GzQuaternToEuler(&roll, &pitch, &yaw, g_laser_mount_rot);
    printf("roll:\t%lf\n", carmen_radians_to_degrees(roll));
    printf("pitch:\t%lf\n", carmen_radians_to_degrees(pitch));
    printf("yaw:\t%lf\n", carmen_radians_to_degrees(yaw));

    firsttime=0;
  }
#endif

  // only do something if we've gotten new laser data
  if(g_laser_newp)
  {

    // TODO (maybe): interpolate amtec pose to synch with laser scans.

    // account for ptu's current position
    laser_pos = GzCoordPositionAdd(g_laser_mount_pos, g_ptu_pos, g_ptu_rot);
    laser_rot = GzCoordRotationAdd(g_laser_mount_rot, g_ptu_rot);

    // account for ptu's mounting on robot
    laser_pos = GzCoordPositionAdd(laser_pos, g_ptu_mount_pos, g_ptu_mount_rot);
    laser_rot = GzCoordRotationAdd(laser_rot, g_ptu_mount_rot);

    // account for robot's pose in world
    laser_pos = GzCoordPositionAdd(laser_pos, g_robot_pos, g_robot_rot);
    laser_rot = GzCoordRotationAdd(laser_rot, g_robot_rot);

    // fill in laser's pose
    msg.pose.x = laser_pos.x;
    msg.pose.y = laser_pos.y;
    msg.pose.z = laser_pos.z;
    GzQuaternToEuler(&roll, &pitch, &yaw, laser_rot);
    msg.pose.roll = roll;
    msg.pose.pitch = pitch;
    msg.pose.yaw = yaw;

    // copy other data from the latest laser message
    msg.num_readings = g_laser_msg.num_readings;
    msg.range = g_laser_msg.range;
    msg.timestamp = g_laser_msg.timestamp;
    memcpy(msg.host,g_laser_msg.host,sizeof(msg.host));

    // publish the result
    IPC_publishData(CARMEN_PANLASER_SCAN_NAME, &msg);

    g_laser_newp = false;
  }
}


