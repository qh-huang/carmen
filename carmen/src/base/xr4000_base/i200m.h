#ifndef I200M_H
#define I200M_H

#ifdef __cplusplus
extern "C" {
#endif

#include "Nclient.h"

int I200M_Initialize(struct N_RobotState *rs);

int I200M_SetAxes(struct N_RobotState *rs);

int I200M_GetAxes(struct N_RobotState *rs);

int I200M_SetJoystick(struct N_RobotState *rs);

int I200M_SetLift(struct N_RobotState *rs);

int I200M_GetLift(struct N_RobotState *rs);

int I200M_ZeroLift(struct N_RobotState *rs __attribute__ ((unused)),
		   unsigned char force);

int I200M_DeployLift(struct N_RobotState *rs __attribute__ ((unused)));

int I200M_RetractLift(struct N_RobotState *rs __attribute__ ((unused)));

int I200M_SetIntegratedConfiguration(struct N_RobotState *rs);

int I200M_GetIntegratedConfiguration(struct N_RobotState *rs);

int I200M_ResetClient();

long unsigned int I200M_MSecSinceBoot();

void I200M_ResetMotionTimer();

#ifdef __cplusplus
}
#endif

#endif
