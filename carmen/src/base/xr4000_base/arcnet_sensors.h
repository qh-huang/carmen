#ifndef ARCNET_SENSORS_H
#define ARCNET_SENSORS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "extend.h"

void SON_Initialize(struct N_RobotState *rs, struct N_RobotStateExt *rs_ext);

void INIT_InitializeSonars(long int robot_id);

void BUMP_Initialize(struct N_RobotState *rs, struct N_RobotStateExt *rs_ext);

void INF_Initialize(struct N_RobotState *rs, struct N_RobotStateExt *rs_ext);

#ifdef __cplusplus
}
#endif

#endif
