#ifndef SEGWAY_MESSAGES_H
#define SEGWAY_MESSAGES_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
  double pitch, pitch_rate;
  double roll, roll_rate;
  double lw_velocity, rw_velocity;
  double x, y, theta;
  double timestamp;
  char host[10];
} carmen_segway_pose_message;

#define CARMEN_SEGWAY_POSE_NAME "carmen_segway_pose"
#define CARMEN_SEGWAY_POSE_FMT "{double,double,double,double,double,double,double,double,double,double,[char:10]}"

#ifdef __cplusplus
}
#endif

#endif
