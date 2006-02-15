
/** @addtogroup pantilt **/
// @{

/** \file pantilt_messages.h
 * \brief Definition of the messages for this module.
 *
 * This file specifies the messages for this modules used to transmit
 * data via ipc to other modules.
 **/

#ifndef CARMEN_PANTILT_MESSAGES_H
#define CARMEN_PANTILT_MESSAGES_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
  double  pan;
  double  tilt;
  double  timestamp;
  char*   host;
} carmen_pantilt_status_message;

#define      CARMEN_PANTILT_STATUS_MESSAGE_NAME  "pantilt_status_message"
#define      CARMEN_PANTILT_STATUS_MESSAGE_FMT   "{double, double, double, string}"


typedef struct {
  double  pan;
  double  tilt;
  double  timestamp;
  char*   host;
} carmen_pantilt_move_message;

#define      CARMEN_PANTILT_MOVE_MESSAGE_NAME  "pantilt_move_message"
#define      CARMEN_PANTILT_MOVE_MESSAGE_FMT   "{double,double,double,string}"

typedef struct {
  double pan;
  double timestamp;
  char*  host;
} carmen_pantilt_move_pan_message;

#define      CARMEN_PANTILT_MOVE_PAN_MESSAGE_NAME  "pantilt_move_pan_message"
#define      CARMEN_PANTILT_MOVE_PAN_MESSAGE_FMT   "{double,double,string}"

typedef struct {
  double  tilt;
  double  timestamp;
  char*   host;
 } carmen_pantilt_move_tilt_message;

#define      CARMEN_PANTILT_MOVE_TILT_MESSAGE_NAME  "pantilt_move_tilt_message"
#define      CARMEN_PANTILT_MOVE_TILT_MESSAGE_FMT   "{double,double,string}"

#ifdef __cplusplus
}
#endif

#endif

// @}
