#include <carmen/carmen.h>
#include <carmen/dot.h>
#include <carmen/dot_interface.h>
#include <carmen/dot_messages.h>
#include <assert.h>

#include "roadmap.h"
#include "dynamics.h"

static carmen_list_t *people = NULL, *trash = NULL, *doors = NULL;
static carmen_map_t *map;

static carmen_list_t *stored_people = NULL, *stored_trash = NULL, *stored_doors = NULL;
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

  stored_people = carmen_list_create(sizeof(carmen_dot_person_t), 10);
  stored_trash = carmen_list_create(sizeof(carmen_dot_trash_t), 10);
  stored_doors = carmen_list_create(sizeof(carmen_dot_door_t), 10);

  carmen_dot_subscribe_all_people_message
    (NULL, (carmen_handler_t) people_handler,CARMEN_SUBSCRIBE_LATEST);
  carmen_dot_subscribe_all_trash_message
    (NULL, (carmen_handler_t) trash_handler, CARMEN_SUBSCRIBE_LATEST);
  carmen_dot_subscribe_all_doors_message
    (NULL, (carmen_handler_t) doors_handler,CARMEN_SUBSCRIBE_LATEST);

  if (0)
  carmen_simulator_subscribe_objects_message
    (NULL, (carmen_handler_t)simulator_objects_handler, 
     CARMEN_SUBSCRIBE_LATEST);
}

void carmen_dynamics_initialize_no_ipc(carmen_map_t *new_map)
{
  map = new_map;
  people = carmen_list_create(sizeof(carmen_dot_person_t), 10);
  trash = carmen_list_create(sizeof(carmen_dot_trash_t), 10);
  doors = carmen_list_create(sizeof(carmen_dot_door_t), 10); 
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


void carmen_dynamics_update(void)
{
  int i;
  carmen_dot_person_t *person;
  carmen_dot_trash_t *trash_bin;
  carmen_dot_door_t *door;

  carmen_warn("Erasing %d %d %d\n", stored_people->length,
	      stored_trash->length, stored_doors->length);

  stored_people->length = 0;
  stored_trash->length = 0;
  stored_doors->length = 0;

  for (i = 0; i < people->length; i++) {
    person = (carmen_dot_person_t *)carmen_list_get(people, i);
    carmen_list_add(stored_people, person);
  }

  for (i = 0; i < trash->length; i++) {
    trash_bin = (carmen_dot_trash_t *)carmen_list_get(trash, i);
    carmen_list_add(stored_trash, trash_bin);
  }

  for (i = 0; i < doors->length; i++) {
    door = (carmen_dot_door_t *)carmen_list_get(doors, i);
    carmen_list_add(stored_doors, door);
  }

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

static int is_blocked(carmen_roadmap_vertex_t *n1, carmen_roadmap_vertex_t *n2,
		      double x, double y, double vx, double vxy, double vy)
{
  double numerator;
  double denominator;
  double i_x;
  double dist_along_line;
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

  numerator = (n2->x - n1->x)*(n1->y - y) - (n1->x - x)*(n2->y - n1->y);
  denominator = hypot(n2->x-n1->x, n2->y-n1->y);
  
  if (fabs(numerator)/denominator > radius) 
    return 0;

  i_x = numerator/denominator * (n2->y - n1->y)/denominator + x;

  dist_along_line = (i_x - n1->x) / (n2->x - n1->x);

  if (dist_along_line > 1 || dist_along_line < 0) {
    if (hypot(n2->x - x, n2->y - y) > radius && 
	hypot(n1->x - x, n1->y - y) > radius)
      return 0;
  }

  return 1;
}

int carmen_dynamics_test_for_block(carmen_roadmap_vertex_t *n1, 
				   carmen_roadmap_vertex_t *n2,
				   int avoid_people)
{
  carmen_dot_person_t *person;
  carmen_dot_trash_t *trash_bin;
  carmen_dot_door_t *door;

  int i;

  if (avoid_people) {
    for (i = 0; i < people->length; i++) {
      person = (carmen_dot_person_t *)carmen_list_get(stored_people, i);
      if (is_blocked(n1, n2, person->x, person->y, person->vx,
		     person->vxy, person->vy))
	return 1;
    }

    for (i = 0; i < people->length; i++) {
      person = (carmen_dot_person_t *)carmen_list_get(people, i);
      if (is_blocked(n1, n2, person->x, person->y, person->vx,
		     person->vxy, person->vy))
	return 1;
    }
  }

  for (i = 0; i < trash->length; i++) {
    trash_bin = (carmen_dot_trash_t *)carmen_list_get(trash, i);
    if (is_blocked(n1, n2, trash_bin->x, trash_bin->y, trash_bin->vx,
		   trash_bin->vxy, trash_bin->vy))
      return 1;
  }

  for (i = 0; i < doors->length; i++) {
    door = (carmen_dot_door_t *)carmen_list_get(doors, i);
    if (is_blocked(n1, n2, door->x, door->y, door->vx, door->vxy,
		   door->vy))
      return 1;
  }

  for (i = 0; i < trash->length; i++) {
    trash_bin = (carmen_dot_trash_t *)carmen_list_get(stored_trash, i);
    if (is_blocked(n1, n2, trash_bin->x, trash_bin->y, trash_bin->vx,
		   trash_bin->vxy, trash_bin->vy))
      return 1;
  }

  for (i = 0; i < doors->length; i++) {
    door = (carmen_dot_door_t *)carmen_list_get(stored_doors, i);
    if (is_blocked(n1, n2, door->x, door->y, door->vx, door->vxy,
		   door->vy))
      return 1;
  }

  return 0;
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
