#ifndef WALKERSERIAL_MESSAGES_H
#define WALKERSERIAL_MESSAGES_H

#include <carmen/carmen.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
  int button;
  double timestamp;
  char host[10];
} carmen_walkerserial_button_msg;

#define CARMEN_WALKERSERIAL_BUTTON_MSG_NAME "carmen_walkerserial_button_msg"
#define CARMEN_WALKERSERIAL_BUTTON_MSG_FMT  "{int, double, [char:10]}"


#define CARMEN_WALKERSERIAL_CLUTCH_DISENGAGE 0
#define CARMEN_WALKERSERIAL_CLUTCH_ENGAGE 1

typedef struct {
  int action;
  double timestamp;
  char host[10];
} carmen_walkerserial_clutch_msg;

#define CARMEN_WALKERSERIAL_CLUTCH_MSG_NAME "carmen_walkerserial_clutch_msg"
#define CARMEN_WALKERSERIAL_CLUTCH_MSG_FMT "{int, double, [char:10]}"


#ifdef __cplusplus
}
#endif

#endif
