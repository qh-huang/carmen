#ifndef DOT_H
#define DOT_H

#include <carmen/carmen.h>

#ifdef __cplusplus
extern "C" {
#endif


#define CARMEN_DOT_PERSON  0
#define CARMEN_DOT_TRASH   1
#define CARMEN_DOT_DOOR    2

#define MAX_PERSON_FILTER_VELOCITY_WINDOW 1000

typedef struct dot_person_filter {
  double vx[MAX_PERSON_FILTER_VELOCITY_WINDOW];
  double vy[MAX_PERSON_FILTER_VELOCITY_WINDOW];
  int vpos;
  int vlen;
  int hidden_cnt;
  double x;
  double y;
  double px;
  double py;
  double pxy;
  double a;
  double qx;
  double qy; 
  double qxy;
  double rx;
  double ry;
  double rxy;
} carmen_dot_person_filter_t, *carmen_dot_person_filter_p;

typedef struct dot_trash_filter {
  double x;
  double y;
  double px;
  double py;
  double pxy;
  double a;
  double qx;
  double qy;
  double qxy;
  double rx;
  double ry;
  double rxy;
} carmen_dot_trash_filter_t, *carmen_dot_trash_filter_p;

typedef struct dot_door_filter {
  double x;
  double y;
  double t;  //theta
  double px;
  double py;
  double pt;
  double pxy;
  double a;
  double qx;
  double qy;
  double qxy;
  double qt;
} carmen_dot_door_filter_t, *carmen_dot_door_filter_p;

typedef struct dot_filter {
  carmen_dot_person_filter_t person_filter;
  carmen_dot_trash_filter_t trash_filter;
  carmen_dot_door_filter_t door_filter;
  int type;
  int allow_change;
  int sensor_update_cnt;
  int do_motion_update;
} carmen_dot_filter_t, *carmen_dot_filter_p;


#ifdef __cplusplus
}
#endif

#endif
