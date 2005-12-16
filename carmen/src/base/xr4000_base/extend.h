#ifndef __INC_extend_h
#define __INC_extend_h

#ifdef __cplusplus
extern "C" {
#endif

struct N_XSonar {
  char *ID;
  char *Reference;
  double *Configuration;
};

struct N_XInfrared {
  char *ID;
  char *Reference;
  double *Configuration;
};

struct N_XInfraredSet {
  long int Dependency;
  double MainLobe;
  double Range;
  struct N_XInfrared Infrared[16];
};

struct N_XInfraredController {
  struct N_XInfraredSet InfraredSet[6];
};

struct N_XBumper {
  char *ID;
  char *Reference;
  double *Configuration;
};

struct N_XBumperSet {
  struct N_XBumper Bumper[12];
};

struct N_XBumperController {
  struct N_XBumperSet BumperSet[6];
};

struct N_XSonarSet {
  double MainLobe;
  double BlindLobe;
  double SideLobe;
  double BlindLobeAttenuation;
  double SideLobeAttenuation;
  double Range;
  struct N_XSonar Sonar[16];
};

struct N_XSonarController {
  struct N_XSonarSet SonarSet[6];
};

struct N_RobotStateExt {
  struct N_XSonarController SonarController;
  struct N_XInfraredController InfraredController;
  struct N_XBumperController BumperController;
};

struct N_RobotStateExt *N_GetRobotStateExt(long RobotID);

#ifdef __cplusplus
}
#endif

#endif
