
#ifndef ROOMNAV_TYPES_H
#define ROOMNAV_TYPES_H

#include <carmen/carmen.h>

#ifdef __cplusplus
extern "C" {
#endif


/*
 * (pos.x, pos.y) is the center of the door.
 * pos.theta is between 0 and 2*pi and indicates the 
 * angle of the vector from room 1 to room 2
 * normal to the door at the center point.
*/
struct carmen_door {
  int room1;
  int room2;
  carmen_map_placelist_t points;
  carmen_point_t pose;  // in world coordinates
  double width;  // in meters
  int num;
};

struct carmen_room {
  int num;
  char *name;
  int *doors;
  int num_doors;
};

typedef struct carmen_door carmen_door_t, *carmen_door_p;
typedef struct carmen_room carmen_room_t, *carmen_room_p;

typedef struct {
  carmen_room_p rooms;
  int num_rooms;
  carmen_door_p doors;
  int num_doors;
} carmen_rooms_topology_t, *carmen_rooms_topology_p;

#define GRID_NONE     -1
#define GRID_UNKNOWN  -2
#define GRID_WALL     -3
#define GRID_DOOR     -4
#define GRID_PROBE    -5


#ifdef __cplusplus
}
#endif

#endif
