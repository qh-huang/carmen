#include <carmen/carmen.h>
#include "roomnav_interface.h"
#include <carmen/localize_interface.h>

static carmen_roomnav_goal_changed_msg goal_msg;
static carmen_roomnav_path_msg path_msg; 
static carmen_roomnav_room_msg room_msg;
static carmen_localize_globalpos_message globalpos_msg;
static carmen_point_t last_cmd_pose;
static carmen_rooms_topology_p topology;
static carmen_map_t map;

static int goal_set = 0; //whether goal is set or not
static int room_set = 0; //whether room has been set or not
static int wait_for_path = 0;

const double PI_HALF = M_PI / 2.0;
const double PI_QUARTER = M_PI / 4.0;
const double PI_3QUARTER = 3.0 * M_PI / 4;
const double PI_TWELFTH = M_PI / 12.0;

#define NONE -1
#define FORWARD 0
#define LEFT 1
#define BACKWARD 2
#define RIGHT 3

//meta-directions
#define GO_THROUGH_DOORWAY 4
#define ENTER_HALLWAY 5
#define ENTER_ROOM_IN_FRONT 6
#define LEFT_CORNER 7
#define RIGHT_CORNER 8
#define GO_TO_END_OF_HALLWAY 9
#define TURN_LEFT_AT_INTERSECTION 10
#define TURN_RIGHT_AT_INTERSECTION 11

//util
#define INTERSECTION_THRESHOLD 2.0 //not currently in use
const float HALLWAY_ANGLE_DIFF_THRESHOLD = M_PI/4.0;
#define CORNER_DIST_THRESHOLD 3.0
#define NEAR_DOOR 2.0 //parameter for saying "go down the hallway"

const float PERPENDICULAR = M_PI / 12.0; 




static int last_dir = -1;





/*
  typedef struct {       [ROOMNAV.H]
  carmen_room_p rooms;
  int num_rooms;
  carmen_door_p doors;
  int num_doors;
  } carmen_rooms_topology_t, *carmen_rooms_topology_p;
  
  struct carmen_door {
  int room1;
  int room2;
  carmen_map_placelist_t points;
  carmen_point_t pose;  // in world coordinates
  double width;  // in meters
  int num;
  int is_real_door;  // or just a border between rooms
  };
  
  typedef struct carmen_door carmen_door_t, *carmen_door_p;
*/

/*
  typedef struct {             [GLOBAL.H]
  double x;
  double y;
  double theta;
  double t_vel;
  double r_vel;
  } carmen_traj_point_t, *carmen_traj_point_p;
  
*/


/////////////////////////////////////
// HELPER FUNCTIONS
/////////////////////////////////////

/* Rounds theta (in radians) to the nearest 90deg
   angle. Uses right-handed coord frame.
   Assumes theta is normalized.
*/
int round_dir(double theta)
{
  if(theta == 0) {
    return FORWARD;
  } else if(theta > 0) {
    if(theta < PI_QUARTER) {
      return FORWARD;
    } else if(theta < PI_3QUARTER) {
      return RIGHT;
    } else {
      return BACKWARD;
    }
  } else { // theta < 0
    if(theta > -1 * PI_QUARTER) {
      return FORWARD;
    } else if(theta > -1 * PI_3QUARTER) {
      return LEFT;
    } else {
      return BACKWARD;
    }
  }
  
  return -1;
}



static void fork_speech(char *s) {
  
  int speech_pid;
  
  //return; //REMOVE_ME

  speech_pid = fork();
  if (speech_pid == 0)
    {
      execlp("/opt/theta/bin/theta", "theta", s, NULL);
      carmen_die_syserror("Could not exec theta");
    }
  if (speech_pid == -1)
    carmen_die_syserror("Could not fork to start theta");
}


/* Prints direction for user.
 */
void print_dir_for_user(int dir)
{
  if (last_dir != dir) {
   
    if(dir == FORWARD) {
      printf("GO FORWARD!\n");
      fork_speech("\"go forward\"");
    } else if(dir == LEFT) {
      printf("GO LEFT!\n");
      fork_speech("\"turn left\"");
    } else if(dir == BACKWARD) {
      printf("TURN AROUND!\n");
      fork_speech("\"turn around\"");
    } else if(dir == RIGHT) {
      printf("GO RIGHT!\n");
      fork_speech("\"turn right\"");
    } else if(dir == GO_THROUGH_DOORWAY) {
      printf("GO THROUGH THE DOORWAY\n");
      fork_speech("\"go through the doorway\"");
    } else if(dir == ENTER_HALLWAY) {
      printf("ENTER THE HALLWAY\n");
      fork_speech("\"enter the hallway\"");
    } else if(dir == ENTER_ROOM_IN_FRONT) {
      printf("ENTER THE ROOM IN FRONT OF YOU\n");
      fork_speech("\"enter the room in front of you\"");
    } else if(dir == LEFT_CORNER) {
      printf("TURN LEFT AT THE CORNER\n");
      fork_speech("\"turn left at the corner\"");
    } else if (dir == RIGHT_CORNER) {
      printf("TURN RIGHT AT THE CORNER\n");
      fork_speech("\"turn right at the corner\"");
    } else if(dir == GO_TO_END_OF_HALLWAY) {
      printf("GO TO THE END OF THE HALLWAY\n");
      fork_speech("\"go to the end of the hallway\"");
    } else if(dir == TURN_LEFT_AT_INTERSECTION) {
      printf("TURN LEFT AT THE INTERSECTION\n");
      fork_speech("\"turn left at the intersection\"");
    } else if(dir == TURN_RIGHT_AT_INTERSECTION) {
      printf("TURN RIGHT AT THE INTERSECTION\n");
      fork_speech("\"turn right at the intersection\"");
    }
    


    last_dir = dir;
  }

  fflush(0);
}



/////////////////////////////////////
// UTIL FUNCTIONS
/////////////////////////////////////
void shutdown_module(int sig)
{
  if(sig == SIGINT) {
    //carmen_logger_fclose(outfile);
    close_ipc();
    fprintf(stderr, "\nDisconnecting.\n");
    exit(0);
  }
}



/** 
    Return whether roughly perpendicular or not.
*/
int perpendicular(double theta) {

  //printf("PERPENDICULAR: theta is: %f\n", theta);
  if(((theta > PI_HALF - PI_TWELFTH) && (theta < PI_HALF + PI_TWELFTH)) 
     || ((theta < -1*PI_HALF + PI_TWELFTH) && (theta > -1*PI_HALF - PI_TWELFTH))) {
    return 1;
  }
  return 0;

}



/**
   Return whether roughly parallel or not.
   Assumes theta is normalized. 
*/
int parallel(double theta) {

  //printf("PARALLEL: theta is: %f\n", theta);
  if(((theta > -1 * PI_TWELFTH) && (theta < PI_TWELFTH)) 
     || (theta > M_PI - PI_TWELFTH)
     || (theta < -1*M_PI + PI_TWELFTH)) {
    return 1;
  }
  return 0;

}


/**
   To call at the end of both cases:
   HALLWAY->ROOM and HALLWAY->HALLWAY
   @param door_struct The next door in the robot's path to goal
   @param robot_pose  Robot's current pose
   @param dir         Direction that the robot should be turning next to face door
   @return decision   Command decision
*/
int get_state_decision_general_hallway(carmen_point_p cur_room_end,
				       carmen_point_p next_door_pose)
{
  
  //for "go to end of hallway"
  //if the door we are going to next is within a certain distance
  //from the endpoint of the room closer to the door,
  //then say "go to the end of hallway"
  if(carmen_distance(cur_room_end, next_door_pose) < NEAR_DOOR) {
    return GO_TO_END_OF_HALLWAY;
  }
  
  return -1;
  

}



/**
   Retrieve the current state of things
   @param door_struct The next door in the robot's path to goal
   @param robot_pose  Robot's current pose
   @param dir         Direction that the robot should be turning next to face door
   @return decision   Command decision
*/
int get_state_decision(carmen_point_t robot_pose, int dir) 
{
  carmen_door_t next_door = (topology->doors[path_msg.path[0]]);
  carmen_door_t following_door;
  double room_angle_diff, door_angle_diff;
  double door_theta;
  double distance_from_door = -1;
  double tmp;
  double tmp_x, tmp_y;

  //end-points of hallway to rotate, for hallway->hallway case
  carmen_point_t room_to_rotate_e1;
  carmen_point_t room_to_rotate_e2;  

  carmen_room_p cur_room = NULL;
  carmen_room_p next_room = NULL;
  carmen_room_p following_room = NULL;
  int is_side_door; //identify location of door wrt corner
  int corner; //identify left or right corner
  carmen_point_t cur_room_end; //endpoint of cur_room closer to next_room
  int is_hallway_bend = 1; 


  carmen_world_point_t world_point; 
  carmen_map_point_t map_point; 
  int room_actual;

  double theta_robot_door; //angle between robot's pose and next_door's pose

  world_point.map = &map; 
  map_point.map = &map; 
    
  world_point.pose.x = globalpos_msg.globalpos.x;
  world_point.pose.y = globalpos_msg.globalpos.y;
  carmen_world_to_map(&world_point, &map_point); 
  
  room_actual = carmen_roomnav_get_room_from_point(map_point);
  if(room_actual != room_msg.room) {
    wait_for_path = 4;
    return -1;
  }

  //Detecting ROOM - HALLWAY combinations of next door  
  cur_room = &(topology->rooms[room_msg.room]);
  if (room_msg.room == next_door.room1) {
    next_room = &(topology->rooms[next_door.room2]);
  } else {
    next_room = &(topology->rooms[next_door.room1]);
  }
  
  distance_from_door = carmen_distance(&robot_pose, &(next_door.pose));
  
  theta_robot_door = carmen_normalize_theta(robot_pose.theta - next_door.pose.theta);
  //printf("theta_robot_door: %f\n", theta_robot_door);
  

  //case-by-case
  switch(cur_room->type) {
  case CARMEN_ROOM_TYPE_ROOM:
    switch(next_room->type) {
    case CARMEN_ROOM_TYPE_ROOM: //ROOM->ROOM
      
      //CASE: if you are 2 metres from a real door and facing it
      if(next_door.is_real_door) {
	if ((distance_from_door < 2.0) && (dir == FORWARD)) {
	  return GO_THROUGH_DOORWAY;
	}
      }
      
      break;
    case CARMEN_ROOM_TYPE_HALLWAY: //ROOM->HALLWAY
      
      //CASE: if you are 2 metres from a real door and facing it
      if(next_door.is_real_door) {
	if ((distance_from_door < 2.0) && (dir == FORWARD)) {
	  return ENTER_HALLWAY;
	}
      }
      break;
      
    }
    break;
    
  case CARMEN_ROOM_TYPE_HALLWAY:

    //figure out which of the current_room's endpoints
    //is the one facing the (real) next room
    //know by figuring out which one is nearer to next_door
    if(carmen_distance(&(cur_room->e1), &(next_door.pose)) < 
       carmen_distance(&(cur_room->e2), &(next_door.pose))) {
      cur_room_end = cur_room->e1;
    } else {
      cur_room_end = cur_room->e2;
    }
        
    switch(next_room->type) {
    case CARMEN_ROOM_TYPE_ROOM: //HALLWAY->ROOM
      
      //CASE: if you are 2 metres from a real door and facing it
      if(next_door.is_real_door) {
	if ((distance_from_door < 2.0) && (dir == FORWARD)) {
	  return ENTER_ROOM_IN_FRONT;
	}
      }
      
      if(dir == FORWARD) {
	return get_state_decision_general_hallway(&cur_room_end, &(next_door.pose));
      }

      break;
    case CARMEN_ROOM_TYPE_HALLWAY: //HALLWAY->HALLWAY

      if (distance_from_door > 5.0) 
	is_hallway_bend = 0;
      //return -1;
	

      room_angle_diff = fabs(carmen_normalize_theta(next_room->theta - cur_room->theta));
      if (room_angle_diff > M_PI/2.0)
	room_angle_diff = M_PI - room_angle_diff;
      if (room_angle_diff < HALLWAY_ANGLE_DIFF_THRESHOLD)  // no intersection or corner detected ~ parallel
	is_hallway_bend = 0;
	//return -1;
      
      // otherwise, classify as T-intersection, corner, or X-intersection
      
      if(is_hallway_bend) {

	//check if side door or right in front -- is_side_door
	door_angle_diff = fabs(carmen_normalize_theta(next_door.pose.theta - cur_room->theta));
	if (door_angle_diff > M_PI/2.0)
	  door_angle_diff = M_PI - door_angle_diff;
	is_side_door = (door_angle_diff > M_PI/4.0);
	
	door_theta = next_door.pose.theta;
	
	if (is_side_door) {
	  room_to_rotate_e1 = cur_room->e1;
	  room_to_rotate_e2 = cur_room->e2;	
	  
	  if (room_msg.room == next_door.room1)
	    door_theta = carmen_normalize_theta(door_theta + M_PI);
	}
	else {
	  room_to_rotate_e1 = next_room->e1;
	  room_to_rotate_e2 = next_room->e2;
	  if (room_msg.room == next_door.room2)
	    door_theta = carmen_normalize_theta(door_theta + M_PI);
	}
	
	room_to_rotate_e1.x -= next_door.pose.x;
	room_to_rotate_e1.y -= next_door.pose.y;
	room_to_rotate_e2.x -= next_door.pose.x;
	room_to_rotate_e2.y -= next_door.pose.y;
	
	// rotate by -theta of next_door
	carmen_rotate_2d(&(room_to_rotate_e1.x), &(room_to_rotate_e1.y), -door_theta);
	carmen_rotate_2d(&(room_to_rotate_e2.x), &(room_to_rotate_e2.y), -door_theta);
	
	// this ensures room_to_rotate_e1.y < room_to_rotate_e2.y
	if (room_to_rotate_e2.y < room_to_rotate_e1.y) {
	  tmp = room_to_rotate_e2.y;
	  room_to_rotate_e2.y = room_to_rotate_e1.y;
	  room_to_rotate_e1.y = tmp;
	}
	
	corner = 0;

	//printf("rtr_e1.y: %f, rtr_e2.y: %f\n", room_to_rotate_e1.y, room_to_rotate_e2.y);
	if (room_to_rotate_e1.y > -CORNER_DIST_THRESHOLD) {
	  corner = 1;
	} else if (room_to_rotate_e2.y < CORNER_DIST_THRESHOLD) {
	  corner = 2;
	} else { //is an intersection -- always not side door
	  corner = 3; //intersection
	}


	if (is_side_door) {
	  
	  //if in region of cur_room that is facing next_room, give F,B,R,L 
	  //guidance instead of with reference to corner.
	  //double tmp2 = carmen_distance(&robot_pose, &cur_room_end);
	  //printf("width of door is: %f, tmp2: %f\n", next_door.width, tmp2);
	  //if((tmp2 = carmen_distance(&robot_pose, &cur_room_end)) < next_door.width) {
	  if(carmen_distance(&robot_pose, &cur_room_end) < next_door.width) {
	    //printf("NEAR DOOR, width of door is: %f\n", next_door.width);
	    break;
	  }
	  
	  if(perpendicular(theta_robot_door)) {
	    if (corner == 1) {
	      return RIGHT_CORNER;
	    } else if (corner == 2) {
	      return LEFT_CORNER;
	    }
	  }
	}
	else { //not side door
	  
	  if(parallel(theta_robot_door)) {
	    if (corner == 1) {
	      return LEFT_CORNER;
	    } else if (corner == 2) {
	      return RIGHT_CORNER;	    
	    } else if(corner == 3) {
	      
	      if(path_msg.pathlen == 1) { //not doing anything bout this
		break; 
	      }

	      following_door = (topology->doors[path_msg.path[1]]);
	      
	      if(next_room->num == following_door.room1) {
		following_room = &(topology->rooms[following_door.room2]); 
	      } else {
		following_room = &(topology->rooms[following_door.room1]); 
	      }
	      
	      tmp_x = following_room->ux;
	      tmp_y = following_room->uy;
	      tmp_x -= next_door.pose.x;
	      tmp_y -= next_door.pose.y;
	      // rotate by -theta of next_door
	      carmen_rotate_2d(&tmp_x, &tmp_y, -door_theta);

	      //printf("tmp_x: %f, tmp_y: %f\n", tmp_x, tmp_y);

	      if(tmp_y > 0) {
			return TURN_LEFT_AT_INTERSECTION;
	      } else {
		return TURN_RIGHT_AT_INTERSECTION;
	      }


	    }


	  }
	  
	}

	
      } //end is_hallway_bend

      if(dir == FORWARD) {
	return get_state_decision_general_hallway(&cur_room_end, &(next_door.pose));
      }


      break;
      
    }
    break;
    
  default:
    printf("cur_room is not a room or hallway\n");
    break;
    
  }
  
  
  
  
  return -1;
}







////////////////////////////////////
// EVENT HANDLERS
////////////////////////////////////

static void print_path(int *p, int plen) {

  int i, r;

  r = room_msg.room;

  printf("path = %d", r);
  for (i = 0; i < plen; i++) {
    r = (topology->doors[p[i]].room1 == r ? topology->doors[p[i]].room2 : topology->doors[p[i]].room1);
    printf(" %d", r);
  }
  printf("\n");
}

/*
  typedef struct {
  int *path;  // array of door #'s
  int pathlen;
  double timestamp;
  char host[10];
  } carmen_roomnav_path_msg;
  
*/
void path_msg_handler()
{
  printf(" --> got path msg\n");
  print_path(path_msg.path, path_msg.pathlen);
  if (wait_for_path == 4)
    wait_for_path = 3;
}

void goal_msg_handler()
{
  //have goal
  goal_set = 1;
  //printf("in goal_msg_handler\n");
}

void room_msg_handler()
{
  //have set room at least once
  room_set = 1;
  printf(" --> got room message\n");
}


/*
  typedef struct {
  carmen_point_t globalpos, globalpos_std;
  carmen_point_t odometrypos;
  double globalpos_xy_cov;
  int converged;
  double timestamp;
  char host[10];
  } carmen_localize_globalpos_message;
  
  typedef struct {   [CARMEN.H]
  double x;
  double y;
  double theta;
  } carmen_point_t, *carmen_point_p;
  
*/
void globalpos_msg_handler()
{
  carmen_point_t door_pose, robot_pose;
  double robot_to_door, theta_diff;
  int dir; // NONE, FORWARD, LEFT, BACKWARD, or RIGHT
  int decision;
  //printf("in globalpos_msg_handler\n");
  
  //if goal is not set, quit!
  if(goal_set == 0) {
    printf("Goal is not set, quitting\n");
    fflush(NULL);
    return;
  }
  //if room is not set, quit!
  if(room_set == 0) {
    printf("Room is not set, quitting\n");
    fflush(NULL);
    return;
  }
  
  //printf("goal_msg.goal: %d\n", goal_msg.goal);
  //printf("room_msg.room: %d\n", room_msg.room);
  
  //if we have reached the goal, quit!
  //else get pose of the next door (there should still be at least one)
  if(goal_msg.goal == room_msg.room) {
    printf("You've reached! DONE!!! =)\n");
    goal_set = 0; //unset
    last_dir = NONE;
    
    printf("goal: %d\n", goal_msg.goal);
    printf("room: %d\n", room_msg.room);
    fflush(NULL);
    return;
  }
  door_pose = (topology->doors[path_msg.path[0]]).pose;

  // make sure we've moved before we issue a new command
  if (carmen_distance(&globalpos_msg.globalpos, &last_cmd_pose) < 0.5 &&
      fabs(carmen_normalize_theta(globalpos_msg.globalpos.theta - last_cmd_pose.theta)) < M_PI/16.0)
    return;
  
  //get robot pose. robot_pose.theta is the direction the robot's facing
  robot_pose = globalpos_msg.globalpos;
  
  //calculate angle between robot & door
  robot_to_door = 0.0;
  if (carmen_distance(&robot_pose, &door_pose) < 2.0) {
    if (room_msg.room == topology->doors[path_msg.path[0]].room1)
      robot_to_door = door_pose.theta;
    else
      robot_to_door = carmen_normalize_theta(door_pose.theta + M_PI);
  }
  else
    robot_to_door = atan2(door_pose.y - robot_pose.y, door_pose.x - robot_pose.x);
  
  //calculate difference between robot's angle and the door's angle
  theta_diff = carmen_normalize_theta(robot_pose.theta - robot_to_door);    
  
  //printf("Printing robot's pose: %f\n", robot_pose.theta);
  //printf("Printing robot_to_door: %f\n", robot_to_door);
  //printf("Printing normalized theta_diff: %f\n", theta_diff);
  //print_dir_for_user(round_dir(theta_diff));
  
  dir = round_dir(theta_diff);
  
  //figure out what state of affairs we're in
  //and apply based on that
  decision = get_state_decision( robot_pose, dir);

  if (wait_for_path == 4) {
    printf(" --> waiting for path\n");
    fflush(0);
    return;
  }
  else if (wait_for_path > 0) {
    wait_for_path--;
    return;
  }

  if(decision != -1) {
    dir = decision;
  }

  print_dir_for_user(dir);

  last_cmd_pose = globalpos_msg.globalpos;
  
  fflush(NULL);
}


void initialize_printstate_ipc()
{
  topology = carmen_roomnav_get_rooms_topology();
  
  //FOR DEBUG ONLY
  printf("------Printing some facts about the topology------\n");
  printf("number of doors: %d\n", topology->num_doors);
  printf("number of rooms: %d\n", topology->num_rooms);
  printf("----End printing some facts about the topology----\n");
  //END FOR DEBUG ONLY
  
  room_msg.room = carmen_roomnav_get_room();
  if (room_msg.room >= 0)
    room_set = 1;
  goal_msg.goal = carmen_roomnav_get_goal();
  if (goal_msg.goal >= 0)
    goal_set = 1;
  if (goal_set)
    path_msg.pathlen = carmen_roomnav_get_path(&path_msg.path);
  
  carmen_roomnav_subscribe_goal_changed_message
    (&goal_msg, (carmen_handler_t)goal_msg_handler, CARMEN_SUBSCRIBE_LATEST);
  
  carmen_roomnav_subscribe_path_message
    (&path_msg, (carmen_handler_t)path_msg_handler, CARMEN_SUBSCRIBE_LATEST);
  
  carmen_roomnav_subscribe_room_message
    (&room_msg, (carmen_handler_t)room_msg_handler, CARMEN_SUBSCRIBE_LATEST);
  
  carmen_localize_subscribe_globalpos_message(&globalpos_msg, (carmen_handler_t)globalpos_msg_handler,
					      CARMEN_SUBSCRIBE_LATEST);    
  
}


int main(int argc __attribute__ ((unused)), char **argv)
{
  //printf("PI_QUAT: %f\n", PI_QUARTER);
  //printf("PI_3QUAT: %f\n", PI_3QUARTER);
  //printf("- PI_QUAT: %f\n", -1 * PI_QUARTER); 
  
  carmen_initialize_ipc(argv[0]);
  carmen_param_check_version(argv[0]);
  
  if (carmen_map_get_gridmap(&map) < 0) {
    fprintf(stderr, "error getting map from mapserver\n");
    exit(0);
  } 
  
  initialize_printstate_ipc();
  
  signal(SIGINT, shutdown_module); 
  
  IPC_dispatch();
  
  return 0;
}
