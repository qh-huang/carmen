#ifndef CARMEN_PANTILT_MESSAGES_H
#define CARMEN_PANTILT_MESSAGES_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
  double  timestamp;
  char*   host;
  double  pan;
  double  tilt;
} carmen_pantilt_status_message;

#define      CARMEN_PANTILT_STATUS_MESSAGE_NAME  "pantilt_status_message"
#define      CARMEN_PANTILT_STATUS_MESSAGE_FMT   "{ double,string,double,double }"


typedef struct {
  double  timestamp;
  char*   host;
  double  pan;
  double  tilt;
} carmen_pantilt_move_message;

#define      CARMEN_PANTILT_MOVE_MESSAGE_NAME  "pantilt_move_message"
#define      CARMEN_PANTILT_MOVE_MESSAGE_FMT   "{ double,string,double,double }"

typedef struct {
  double timestamp;
  char*  host;
  double pan;
} carmen_pantilt_move_pan_message;

#define      CARMEN_PANTILT_MOVE_PAN_MESSAGE_NAME  "pantilt_move_pan_message"
#define      CARMEN_PANTILT_MOVE_PAN_MESSAGE_FMT   "{ double,string,double }"

typedef struct {
  double  timestamp;
  char*   host;
  double  tilt;
 } carmen_pantilt_move_tilt_message;

#define      CARMEN_PANTILT_MOVE_TILT_MESSAGE_NAME  "pantilt_move_tilt_message"
#define      CARMEN_PANTILT_MOVE_TILT_MESSAGE_FMT   "{ double,string,double }"

#ifdef __cplusplus
}
#endif

#endif
