#include <carmen/carmen.h>
#include <carmen/dot.h>
#include <carmen/dot_interface.h>
#include <carmen/dot_messages.h>
#include <assert.h>

#include "roadmap.h"
#include "dynamics.h"

static carmen_list_t *people = NULL, *trash = NULL, *doors = NULL;
static carmen_map_t *map;

static carmen_list_t *marked_edges = NULL;

static carmen_map_t *map;

static void correct(double *x, double *y)
{
  carmen_world_point_t wp;
  carmen_map_point_t mp;

  wp.pose.x = *x;
  wp.pose.y = *y;
  wp.map = map;
  carmen_world_to_map(&wp, &mp);
  *x = mp.x;
  *y = mp.y;
}

static void people_handler(carmen_dot_all_people_msg *people_msg)
{
  int i;

  people->length = 0;
  for (i = 0; i < people_msg->num_people; i++) {
    correct(&(people_msg->people[i].x), &(people_msg->people[i].y));
    carmen_list_add(people, people_msg->people+i);
  }
}

static void simulator_objects_handler(carmen_simulator_objects_message 
				      *objects_msg)
{
  carmen_dot_person_t person, *person_ptr;

  if (objects_msg->num_objects == 0) {
    if (people->length > 0) {
      people->length = 0;
      carmen_warn("Removed simulator person\n");
    }
    return;
  }

  person.x = objects_msg->objects_list[0].x;
  person.y = objects_msg->objects_list[0].y;
  person.vx = 0.375;
  person.vy = 0.375;
  person.vxy = 0.125;
  person.id = 0;

  correct(&(person.x), &(person.y));

  if (people->length > 0) {
    person_ptr = (carmen_dot_person_t *)carmen_list_get(people, 0);    
    *person_ptr = person;
  } else {
    carmen_list_add(people, &person);
    carmen_warn("Added simulator person\n");
  }
}


static void trash_handler(carmen_dot_all_trash_msg *trash_msg)
{
  int i;

  trash->length = 0;
  for (i = 0; i < trash_msg->num_trash; i++) {
    correct(&(trash_msg->trash[i].x), &(trash_msg->trash[i].y));
    carmen_list_add(trash, trash_msg->trash+i);
  }
}

static void doors_handler(carmen_dot_all_doors_msg *door_msg)
{
  int i;

  doors->length = 0;
  for (i = 0; i < door_msg->num_doors; i++) {
    correct(&(door_msg->doors[i].x), &(door_msg->doors[i].y));
    carmen_list_add(doors, door_msg->doors+i);
  }
}

void carmen_dynamics_initialize(carmen_map_t *new_map)
{
  map = new_map;

  people = carmen_list_create(sizeof(carmen_dot_person_t), 10);
  trash = carmen_list_create(sizeof(carmen_dot_trash_t), 10);
  doors = carmen_list_create(sizeof(carmen_dot_door_t), 10);

  carmen_dot_subscribe_all_people_message
    (NULL, (carmen_handler_t) people_handler,CARMEN_SUBSCRIBE_LATEST);
  carmen_dot_subscribe_all_trash_message
    (NULL, (carmen_handler_t) trash_handler, CARMEN_SUBSCRIBE_LATEST);
  carmen_dot_subscribe_all_doors_message
    (NULL, (carmen_handler_t) doors_handler,CARMEN_SUBSCRIBE_LATEST);

  //  if (0)
  carmen_simulator_subscribe_objects_message
    (NULL, (carmen_handler_t)simulator_objects_handler, 
     CARMEN_SUBSCRIBE_LATEST);

  marked_edges = carmen_list_create(sizeof(carmen_roadmap_marked_edge_t), 10);
}

void carmen_dynamics_initialize_no_ipc(carmen_map_t *new_map)
{
  map = new_map;
  people = carmen_list_create(sizeof(carmen_dot_person_t), 10);
  trash = carmen_list_create(sizeof(carmen_dot_trash_t), 10);
  doors = carmen_list_create(sizeof(carmen_dot_door_t), 10); 

  marked_edges = carmen_list_create(sizeof(carmen_roadmap_marked_edge_t), 10);
}

void carmen_dynamics_update_person(carmen_dot_person_t *update_person)
{
  int i;
  carmen_dot_person_t *person;
  carmen_world_point_t wp;
  carmen_map_point_t mp;

  wp.pose.x = update_person->x;
  wp.pose.y = update_person->y;
  wp.map = map;
  carmen_world_to_map(&wp, &mp);
  update_person->x = mp.x;
  update_person->y = mp.y;

  for (i = 0; i < people->length; i++) {
    person = (carmen_dot_person_t *)carmen_list_get(people, i);
    if (person->id == update_person->id) {
      *person = *update_person;
      return;
    }
  }

  carmen_list_add(people, update_person);
}


void coord_shift(double *x, double *y, double theta, 
			  double delta_x, double delta_y)
{
  *x -= delta_x;
  *y -= delta_y;
  
  *x = cos(theta) * *x + sin(theta) * *y;
  *y = -sin(theta) * *x + cos(theta) * *y;
}

#if 0
static int is_blocked(carmen_roadmap_vertex_t *n1, carmen_roadmap_vertex_t *n2,
		      double x, double y, double vx, double vxy, double vy)
{
  double e1, e2;
  double ex, ey;
  double x1, x2, y1, y2;
  double root1, root2;
  double root1y, root2y;
  double m, b;
  double d, e, f;
  double theta;
  double dist_along_line1, dist_along_line2;

  x1 = n1->x;
  y1 = n1->y;
  x2 = n2->x;
  y2 = n2->y;  

  e1 = (vx + vy)/2.0 + sqrt(4*vxy*vxy + (vx-vy)*(vx-vy))/2.0;
  e2 = (vx + vy)/2.0 - sqrt(4*vxy*vxy + (vx-vy)*(vx-vy))/2.0;
  ex = vxy;
  ey = e1 - vx;

  theta = atan2(ey, ex);

  e1 = 3*sqrt(e1);
  e2 = 3*sqrt(e2);

  coord_shift(&x1, &y1, -theta, x, y);
  coord_shift(&x2, &y2, -theta, x, y);

  m = (y2-y1)/(x2-x1);
  b = m*x2+y2;

  d = e2*e2+e1*e1*m*m;
  e = e1*e1*2*m*b;
  f = e1*e1*b*b-e1*e1*e2*e2;

  if (e*e-4*d*f <= 0)
    return 0;

  root1 = (-e + sqrt(e*e-4*d*f)) / 2*d;
  root2 = (-e - sqrt(e*e-4*d*f)) / 2*d;

  root1y = m * root1 + b;
  root2y = m * root2 + b;

  dist_along_line1 = (root1 - x1) / (x2 - x1);
  dist_along_line2 = (root2 - x1) / (x2 - x1);

  if ((dist_along_line1 > 1 && dist_along_line2 > 1) || 
      (dist_along_line1 < 0 && dist_along_line2 < 0)) {
      return 0;
  }

  return 1;
}

#endif

static int is_blocked(int n1x, int n1y, int n2x, int n2y, double x, double y, 
		      double vx, double vxy, double vy)
{
  double numerator;
  double denominator;
  double i_x, i_y;
  double dist_along_line;
  carmen_world_point_t wp;
  carmen_map_point_t mp;
  double radius;
  double e1, e2;
  double theta;

  e1 = (vx + vy)/2.0 + sqrt(4*vxy*vxy + (vx-vy)*(vx-vy))/2.0;
  e2 = (vx + vy)/2.0 - sqrt(4*vxy*vxy + (vx-vy)*(vx-vy))/2.0;

  e1 = 3*sqrt(e1);
  e2 = 3*sqrt(e2);

  if (e1 < 1)
    e1 = .5;
  if (e2 < 1)
    e2 = .5;

  wp.pose.x = e1;
  wp.pose.y = e2;
  wp.map = map;
  carmen_world_to_map(&wp, &mp);

  radius = (mp.x + mp.y) / 2;

  numerator = (n2x - n1x)*(n1y - y) - (n1x - x)*(n2y - n1y);
  denominator = hypot(n2x-n1x, n2y-n1y);
  
  if (fabs(denominator) < 1e-9 || fabs(numerator)/denominator > radius) 
    return 0;

  theta = fabs(atan2(n2y-n1y, n2x - n1x));
  if (theta > M_PI/4 && theta < 3*M_PI/4) {
    i_y = numerator/denominator * (n2x - n1x)/denominator + y;
    dist_along_line = (i_y - n1y) / (n2y - n1y);
  } else {
    i_x = numerator/denominator * (n2y - n1y)/denominator + x;
    dist_along_line = (i_x - n1x) / (n2x - n1x);
  }

  if (dist_along_line > 1 || dist_along_line < 0) {
    if (hypot(n2x - x, n2y - y) > radius && 
	hypot(n1x - x, n1y - y) > radius)
      return 0;
  }

  return 1;
}

static int do_blocking(int n1x, int n1y, int n2x, int n2y, int avoid_people)
{
  carmen_dot_person_t *person;
  carmen_dot_trash_t *trash_bin;
  carmen_dot_door_t *door;

  int i;

  if (avoid_people) {
    for (i = 0; i < people->length; i++) {
      person = (carmen_dot_person_t *)carmen_list_get(people, i);
      if (is_blocked(n1x, n1y, n2x, n2y, person->x, person->y, 
		     person->vx, person->vxy, person->vy))
	return 1;
    }
  }

  for (i = 0; i < trash->length; i++) {
    trash_bin = (carmen_dot_trash_t *)carmen_list_get(trash, i);
    if (is_blocked(n1x, n1y, n2x, n2y, trash_bin->x, trash_bin->y, 
		   trash_bin->vx, trash_bin->vxy, trash_bin->vy))
      return 1;
  }

  for (i = 0; i < doors->length; i++) {
    door = (carmen_dot_door_t *)carmen_list_get(doors, i);
    if (is_blocked(n1x, n1y, n2x, n2y, door->x, door->y, door->vx, 
		   door->vxy, door->vy))
      return 1;
  }

  return 0;
}

int carmen_dynamics_test_for_block(carmen_roadmap_vertex_t *n1, 
				   carmen_roadmap_vertex_t *n2,
				   int avoid_people)
{
  return do_blocking(n1->x, n1->y, n2->x, n2->y, avoid_people);
}

int carmen_dynamics_test_point_for_block(carmen_roadmap_vertex_t *n, 
					 carmen_world_point_t *point,
					 int avoid_people)
{
  carmen_map_point_t map_pt;
  carmen_world_to_map(point, &map_pt);

  return do_blocking(n->x, n->y, map_pt.x, map_pt.y, avoid_people);  
}

static int is_too_close(carmen_roadmap_vertex_t *n1, double x, double y, 
			double vx, double vxy, double vy)
{
  carmen_world_point_t wp;
  carmen_map_point_t mp;
  double radius;
  double e1, e2;

  e1 = (vx + vy)/2.0 + sqrt(4*vxy*vxy + (vx-vy)*(vx-vy))/2.0;
  e2 = (vx + vy)/2.0 - sqrt(4*vxy*vxy + (vx-vy)*(vx-vy))/2.0;

  e1 = 3*sqrt(e1);
  e2 = 3*sqrt(e2);

  if (e1 < 1)
    e1 = .5;
  if (e2 < 1)
    e2 = .5;

  wp.pose.x = e1;
  wp.pose.y = e2;
  wp.map = map;
  carmen_world_to_map(&wp, &mp);

  radius = (mp.x + mp.y) / 2;

  if (hypot(n1->x-x, n1->y-y) > radius) 
    return 0;

  return 1;
}


int carmen_dynamics_test_node(carmen_roadmap_vertex_t *n1, 
				   int avoid_people)
{
  carmen_dot_person_t *person;
  carmen_dot_trash_t *trash_bin;
  carmen_dot_door_t *door;

  int i;

  if (avoid_people) {
    for (i = 0; i < people->length; i++) {
      person = (carmen_dot_person_t *)carmen_list_get(people, i);
      if (is_too_close(n1, person->x, person->y, person->vx,
		       person->vxy, person->vy))
	return 1;
    }
  }

  for (i = 0; i < trash->length; i++) {
    trash_bin = (carmen_dot_trash_t *)carmen_list_get(trash, i);
    if (is_too_close(n1, trash_bin->x, trash_bin->y, trash_bin->vx,
		   trash_bin->vxy, trash_bin->vy))
      return 1;
  }

  for (i = 0; i < doors->length; i++) {
    door = (carmen_dot_door_t *)carmen_list_get(doors, i);
    if (is_too_close(n1, door->x, door->y, door->vx, door->vxy,
		   door->vy))
      return 1;
  }

  return 0;
}

int carmen_dynamics_find_edge_and_block(carmen_roadmap_vertex_t *parent_node, 
					int child_id)
{
  carmen_roadmap_edge_t *edges;
  int length;
  int j;

  length = parent_node->edges->length;
  edges = (carmen_roadmap_edge_t *)
    parent_node->edges->list;
  for (j = 0; j < length; j++) {
    if (edges[j].id == child_id)
      break;
  }
  assert (j < length);
  edges[j].blocked = 1;

  return j;
}


void carmen_dynamics_mark_blocked(int node1_id, int edge1_id, 
				  int node2_id, int edge2_id)
{
  carmen_roadmap_marked_edge_t marked_edge;

  marked_edge.n1 = node1_id;
  marked_edge.e1 = edge1_id;
  marked_edge.n2 = node2_id;
  marked_edge.e2 = edge2_id;

  carmen_list_add(marked_edges, &marked_edge);
}

void carmen_dynamics_clear_all_blocked(carmen_roadmap_t *roadmap)
{  
  carmen_roadmap_marked_edge_t *edge_list;
  carmen_roadmap_vertex_t *node_list;
  int i;
  int n1, e1, n2, e2;

  if (!marked_edges || !marked_edges->list)
    return;

  edge_list = (carmen_roadmap_marked_edge_t *)marked_edges->list;
  node_list = (carmen_roadmap_vertex_t *)roadmap->nodes->list;
  for (i = 0; i < marked_edges->length; i++) {    
    n1 = edge_list[i].n1;
    e1 = edge_list[i].e1;
    n2 = edge_list[i].n2;
    e2 = edge_list[i].e2;
    ((carmen_roadmap_edge_t *)(node_list[n1].edges->list))[e1].blocked = 0;
    ((carmen_roadmap_edge_t *)(node_list[n2].edges->list))[e2].blocked = 0;
  }
  marked_edges->length = 0;
}
