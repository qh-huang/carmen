
#ifndef GNAV_MESSAGES_H
#define GNAV_MESSAGES_H

#include "gnav_types.h"

#ifdef __cplusplus
extern "C" {
#endif


typedef struct {
  int room;
  double timestamp;
  char host[10];
} carmen_gnav_room_msg;

#define CARMEN_GNAV_ROOM_MSG_NAME "carmen_gnav_room_msg"
#define CARMEN_GNAV_ROOM_MSG_FMT  "{int, double, [char:10]}"

typedef struct {
  carmen_rooms_topology_t topology;
  double timestamp;
  char host[10];
} carmen_gnav_rooms_topology_msg;

#define CARMEN_GNAV_ROOMS_TOPOLOGY_MSG_NAME "carmen_gnav_rooms_topology_msg"
#define CARMEN_GNAV_ROOMS_TOPOLOGY_MSG_FMT "{{<{int, string, <int:4>, int}:2>, int, <{int, int, {<{int,int,[char:22],double,double,double,double,double,double}:2>,int},{double,double,double},double,int}:4>, int}, double, [char:10]}"

typedef struct {
  int *path;  // array of door #'s
  int pathlen;
  double timestamp;
  char host[10];
} carmen_gnav_path_msg;

#define CARMEN_GNAV_PATH_MSG_NAME "carmen_gnav_path_msg"
#define CARMEN_GNAV_PATH_MSG_FMT "{<int:2>,int,double,[char:10]}"


#ifdef __cplusplus
}
#endif

#endif
