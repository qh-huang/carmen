#ifndef __INC_robot_client_h
#define __INC_robot_client_h

#ifdef __cplusplus
extern "C" {
#endif

#include "extend.h"

long unsigned int RCLNT_GetTimeStamp();
void N_ResetMotionTimer();
int N_PollClient();

extern struct N_RobotState rclnt_rs; 
extern struct N_RobotStateExt rclnt_rs_ext; 

#ifdef __cplusplus
}
#endif

#endif
