#ifndef CANON_MESSAGES_H
#define CANON_MESSAGES_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
  int thumbnail_over_ipc;
  int image_over_ipc;
  int image_to_drive;
  int flash_mode;
  double timestamp;
  char host[10];
} carmen_canon_image_request;

#define      CARMEN_CANON_IMAGE_REQUEST_NAME   "carmen_canon_image_request"
#define      CARMEN_CANON_IMAGE_REQUEST_FMT    "{int,int,int,int,double,[char:10]}"

typedef struct {
  int thumbnail_length;
  char *thumbnail;
  int image_length;
  char *image;
  double timestamp;
  char host[10];
} carmen_canon_image_message;

#define      CARMEN_CANON_IMAGE_NAME       "carmen_canon_image"
#define      CARMEN_CANON_IMAGE_FMT        "{int,<char:1>,int,<char:3>,double,[char:10]}"

#define      CARMEN_CANON_PREVIEW_START_NAME   "canon_preview_start"
#define      CARMEN_CANON_PREVIEW_START_FMT    ""

#define      CARMEN_CANON_PREVIEW_STOP_NAME    "canon_preview_stop"
#define      CARMEN_CANON_PREVIEW_STOP_FMT     ""

typedef struct {
  int preview_length;
  char *preview;
  double timestamp;
  char host[10];
} carmen_canon_preview_message;

#define      CARMEN_CANON_PREVIEW_NAME     "carmen_canon_preview"
#define      CARMEN_CANON_PREVIEW_FMT      "{int,<char:1>,double,[char:10]}"

#ifdef __cplusplus
}
#endif

#endif
