#ifndef CARMEN_IMU_MESSAGES_H
#define CARMEN_IMU_MESSAGES_H

typedef struct {
  double accX;
  double accY;
  double accZ;
  double q0;
  double q1;
  double q2;
  double q3;
  double timestamp;
  char *host;
} carmen_imu_message;

#define CARMEN_IMU_MESSAGE_NAME "carmen_imu"
#define CARMEN_IMU_MESSAGE_FMT  "{double, double, double, double, double, double, double, double, string}"


typedef struct {
  int isalive;
} carmen_imu_alive_message;
  
#define      CARMEN_IMU_ALIVE_NAME            "carmen_imu_alive"
#define      CARMEN_IMU_ALIVE_FMT             "{int}"


#endif
