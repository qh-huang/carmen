#ifndef SEGWAY_IPC_H
#define SEGWAY_IPC_H

#include "segwaycore.h"

void carmen_segway_register_messages(void);

void carmen_segway_publish_odometry(segway_p segway, double timestamp);

void carmen_segway_publish_pose(segway_p segway, double timestamp);

#endif
