#ifndef DOT_MESSAGES_H
#define DOT_MESSAGES_H

#include <carmen/carmen.h>
#include "dot.h"

#ifdef __cplusplus
extern "C" {
#endif


typedef struct {
  carmen_dot_filter_p filters;
  int num_filters;
  double timestamp;
  char host[10];
} carmen_dot_filters_msg;

#define CARMEN_DOT_FILTERS_MSG_NAME "carmen_dot_filters_msg"
#define CARMEN_DOT_FILTERS_MSG_FMT  "{<{{<double:4>, <double:4>, int, int, int, double, double, double, double, double, double, double, double, double, double, double, double}, {double, double, double, double, double, double, double, double, double, double, double, double}, {double, double, double, double, double, double, double, double, double, double, double, double}, int, int, int, int}:2>, int, double, [char:10]}"


typedef struct {
  double timestamp;
  char host[10];
} carmen_dot_start_msg;

#define CARMEN_DOT_START_MSG_NAME "carmen_dot_start_msg"
#define CARMEN_DOT_START_MSG_FMT  "{double, [char:10]}"


#ifdef __cplusplus
}
#endif

#endif
