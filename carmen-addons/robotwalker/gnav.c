//#define NO_GRAPHICS 1

#ifndef NO_GRAPHICS
#include <carmen/carmen_graphics.h>
#else
#include <carmen/carmen.h>
#endif

#include <carmen/map_io.h>

#include "gnav_messages.h"


#define GRID_NONE     -1
#define GRID_UNKNOWN  -2
#define GRID_WALL     -3
#define GRID_DOOR     -4
#define GRID_PROBE    -5

#define DEFAULT_GRID_RESOLUTION 0.1

typedef struct point_node {
  int x;
  int y;
  struct point_node *next;
} point_node_t, *point_node_p;

typedef struct path_node {
  int *path;
  int pathlen;
  int cost;
  struct path_node *next;
} path_node_t, *path_node_p;

typedef struct fill_node {
  int x;
  int y;
  int fill;
  struct fill_node *next;
} fill_node_t, *fill_node_p;

typedef struct {
  point_node_p probe_stack;
  point_node_p room_stack;
  int num;
} blob_t, *blob_p;

static carmen_map_t map;
static int **grid;
static int grid_width, grid_height;
static double grid_resolution;
static carmen_room_p rooms;
static int num_rooms;
static carmen_door_p doors;
static int num_doors;

carmen_localize_globalpos_message global_pos;
static int room = -1;
static int goal = -1;
static int *path = NULL;
static int pathlen = -1;  // 0 = goal reached, -1 = goal unreachable

#ifndef NO_GRAPHICS

static GdkGC *drawing_gc = NULL;
static GdkPixmap *pixmap = NULL;
static GtkWidget *window, *canvas;
static int canvas_width = 300, canvas_height = 300;

static void draw_grid(int x0, int y0, int width, int height);
static void grid_to_image(int x, int y, int width, int height);

#endif

static int fast = 0;

static int get_farthest(int door, int place);


static inline double dist(double x, double y) {

  return sqrt(x*x + y*y);
}

static void print_doors() {

  int i;

  for (i = 0; i < num_doors; i++) {
    printf("door %d:  ", i);
    printf("rooms(%d, %d), ", doors[i].room1 != -1 ? rooms[doors[i].room1].num : -1,
	   doors[i].room2 != -1 ? rooms[doors[i].room2].num : -1);
    printf("pose(%.2f, %.2f, %.0f), ", doors[i].pose.x, doors[i].pose.y,
	   carmen_radians_to_degrees(doors[i].pose.theta));
    printf("width(%.2f)", doors[i].width);
    printf("\n");
  }
}

// fmt of place names: "<door>.<place>" 
static void get_doors(carmen_map_placelist_p placelist) {

  int num_doornames, num_places, *num_doorplaces;
  int i, j, k, n, e1, e2;
  double e1x, e1y, e2x, e2y;
  char **doornames, *placename;

  num_doornames = 0;
  num_places = placelist->num_places;

  doornames = (char **) calloc(num_places, sizeof(char *));
  carmen_test_alloc(doornames);
  num_doorplaces = (int *) calloc(num_places, sizeof(int));
  carmen_test_alloc(num_doorplaces);

  // get doornames & num_doors
  for (i = 0; i < num_places; i++) {
    placename = placelist->places[i].name;
    n = strcspn(placename, ".");
    for (j = 0; j < num_doornames; j++)
      if ((n == (int)strlen(doornames[j])) &&
	  !strncmp(doornames[j], placename, n))
	break;
    if (j < num_doornames)
      num_doorplaces[j]++;
    else {
      doornames[j] = (char *) calloc(n+1, sizeof(char));
      carmen_test_alloc(doornames[j]);
      strncpy(doornames[j], placename, n);
      doornames[j][n] = '\0';
      num_doorplaces[j] = 1;
      num_doornames++;
    }
  }
  
  num_doors = num_doornames;
  doors = (carmen_door_p) calloc(num_doors, sizeof(carmen_door_t));
  carmen_test_alloc(doors);

  // get door places
  for (j = 0; j < num_doors; j++) {
    doors[j].num = j;
    doors[j].points.num_places = num_doorplaces[j];
    doors[j].points.places =
      (carmen_place_p) calloc(num_doorplaces[j], sizeof(carmen_place_t));
    carmen_test_alloc(doors[j].points.places);
    k = 0;
    for (i = 0; i < num_places; i++) {
      placename = placelist->places[i].name;
      n = strcspn(placename, ".");
      if ((n == (int)strlen(doornames[j])) &&
	  !strncmp(doornames[j], placename, n))
	memcpy(&doors[j].points.places[k++],
	       &placelist->places[i], sizeof(carmen_place_t));
    }
    free(doornames[j]);

    e1 = get_farthest(j, 0);
    e2 = get_farthest(j, e1);
    e1x = doors[j].points.places[e1].x;
    e1y = doors[j].points.places[e1].y;
    e2x = doors[j].points.places[e2].x;
    e2y = doors[j].points.places[e2].y;
    doors[j].pose.x = (e1x + e2x) / 2.0;
    doors[j].pose.y = (e1y + e2y) / 2.0;
    doors[j].width = dist(e2x - e1x, e2y - e1y);
  }

  free(doornames);
  free(num_doorplaces);
}

/* returns room number */
static int probe(int px, int py) {

  blob_p blob;
  point_node_p tmp;
  int x, y, i, j, step, cell, num;
  int bbx0, bby0, bbx1, bby1;

  blob = (blob_p) calloc(1, sizeof(blob_t));
  carmen_test_alloc(blob);

  blob->num = -1;
  blob->probe_stack = (point_node_p) calloc(1, sizeof(point_node_t));
  carmen_test_alloc(blob->probe_stack);
  blob->probe_stack->x = px;
  blob->probe_stack->y = py;
  blob->probe_stack->next = NULL;
  blob->room_stack = NULL;

  step = (int) (0.4 / grid_resolution);
  if (step == 0)
    step = 1;

  bbx0 = px;
  bby0 = py;
  bbx1 = px + step;
  bby1 = py + step;
  
  while (blob->probe_stack != NULL) {

    x = blob->probe_stack->x;
    y = blob->probe_stack->y;      

    if (x < bbx0)
      bbx0 = x;
    else if (x + step > bbx1)
      bbx1 = x + step;
    if (y < bby0)
      bby0 = y;
    else if (y + step > bby1)
      bby1 = y + step;

    tmp = blob->probe_stack;
    blob->probe_stack = blob->probe_stack->next;
    free(tmp);

    if (x < 0 || x >= grid_width || y < 0 || y >= grid_height)
      continue;

    for (i = x; (i < x + step) && (i < grid_width); i++) {
      for (j = y; (j < y + step) && (j < grid_height); j++) {
	grid[i][j] = GRID_PROBE;
	tmp = (point_node_p) calloc(1, sizeof(point_node_t));
	carmen_test_alloc(tmp);
	tmp->x = i;
	tmp->y = j;
	tmp->next = blob->room_stack;
	blob->room_stack = tmp;
      }
    }
    
    // up
    cell = GRID_NONE;
    for (i = x; (i < x + step) && (i < grid_width) && (cell == GRID_NONE); i++)
      for (j = y + step; j < y + 2*step && j < grid_height && cell == GRID_NONE; j++)
	cell = grid[i][j];
    if (cell >= 0)
      blob->num = cell;
    else if (cell == GRID_NONE) {
      tmp = (point_node_p) calloc(1, sizeof(point_node_t));
      carmen_test_alloc(tmp);
      tmp->x = x;
      tmp->y = y + step;
      tmp->next = blob->probe_stack;
      blob->probe_stack = tmp;
    }
    
    // down
    cell = GRID_NONE;
    for (i = x; (i < x + step) && (i < grid_width) && (cell == GRID_NONE); i++)
      for (j = y - 1; (j >= y - step) && (j >= 0) && (cell == GRID_NONE); j--)
	cell = grid[i][j];
    if (cell >= 0)
      blob->num = cell;
    else if (cell == GRID_NONE) {
      tmp = (point_node_p) calloc(1, sizeof(point_node_t));
      carmen_test_alloc(tmp);
      tmp->x = x;
      tmp->y = y - step;
      tmp->next = blob->probe_stack;
      blob->probe_stack = tmp;
    }
  
    // right
    cell = GRID_NONE;
    for (i = x + step; i < x + 2*step && i < grid_width && cell == GRID_NONE; i++)
      for (j = y; j < y + step && j < grid_height && cell == GRID_NONE; j++)
	cell = grid[i][j];
    if (cell >= 0)
      blob->num = cell;
    else if (cell == GRID_NONE) {
      tmp = (point_node_p) calloc(1, sizeof(point_node_t));
      carmen_test_alloc(tmp);
      tmp->x = x + step;
      tmp->y = y;
      tmp->next = blob->probe_stack;
      blob->probe_stack = tmp;
    }
    
    // left
    cell = GRID_NONE;
    for (i = x-1; i >= x - step && i >= 0 && cell == GRID_NONE; i--)
      for (j = y; j < y + step && j < grid_height && cell == GRID_NONE; j++)
	cell = grid[i][j];
    if (cell >= 0)
      blob->num = cell;
    else if (cell == GRID_NONE) {
      tmp = (point_node_p) calloc(1, sizeof(point_node_t));
      carmen_test_alloc(tmp);
      tmp->x = x - step;
      tmp->y = y;
      tmp->next = blob->probe_stack;
      blob->probe_stack = tmp;
    }

#ifndef NO_GRAPHICS
    if (!fast) {
      grid_to_image(x-2, y-2, step+4, step+4);
      draw_grid(x-2, y-2, step+4, step+4);
    }
#endif

  }

  if (blob->num == -1)
    blob->num = num_rooms;

  num = blob->num;

  while(blob->room_stack != NULL) {
    tmp = blob->room_stack;
    grid[tmp->x][tmp->y] = blob->num;
    blob->room_stack = blob->room_stack->next;
    free(tmp);
  }

  free(blob);

  bbx0 = carmen_clamp(0, bbx0-2, grid_width);
  bby0 = carmen_clamp(0, bby0-2, grid_height);
  bbx1 = carmen_clamp(0, bbx1+2, grid_width);
  bby1 = carmen_clamp(0, bby1+2, grid_height);

#ifndef NO_GRAPHICS
  grid_to_image(bbx0, bby0, bbx1 - bbx0, bby1 - bby0);
  draw_grid(bbx0, bby0, bbx1 - bbx0, bby1 - bby0);
#endif

  return num;
}

static int get_farthest(int door, int place) {

  int i, n, f;
  double x, y, d, d2;
  carmen_place_p p;

  n = doors[door].points.num_places;
  p = doors[door].points.places;

  x = p[place].x;
  y = p[place].y;

  d = f = 0;

  for (i = 0; i < n; i++) {
    if (i == place)
      continue;
    d2 = dist(x - p[i].x, y - p[i].y);
    if (d < d2) {
      d = d2;
      f = i;
    }
  }

  return f;
}

static void get_rooms() {

  int i, j, n, e1, e2;
  double x, y;
  double dx, dy, d;
  double e1x, e1y;  // endpoint 1
  double e2x, e2y;  // endpoint 2
  double mx, my;    // midpoint
  double p1x, p1y;  // probe 1
  double p2x, p2y;  // probe 2
  carmen_world_point_t world_point;
  carmen_map_point_t map_point;

  rooms = (carmen_room_p) calloc(num_doors + 1, sizeof(carmen_room_t));
  carmen_test_alloc(rooms);
  num_rooms = 0;

  world_point.map = &map;
  map_point.map = &map;

  // mark off door
  for (i = 0; i < num_doors; i++) {

    e1 = get_farthest(i, 0);
    e2 = get_farthest(i, e1);

    e1x = doors[i].points.places[e1].x;
    e1y = doors[i].points.places[e1].y;
    e2x = doors[i].points.places[e2].x;
    e2y = doors[i].points.places[e2].y;

    dx = e2x - e1x;
    dy = e2y - e1y;
    d = dist(dx, dy);

    for (x = e1x, y = e1y;
	 (dx < 0 ? x >= e2x : x <= e2x) && (dy < 0 ? y >= e2y : y <= e2y);
	 x += dx*grid_resolution/d, y += dy*grid_resolution/d) {
      world_point.pose.x = x;
      world_point.pose.y = y;
      carmen_world_to_map(&world_point, &map_point);
      grid[map_point.x][map_point.y] = GRID_DOOR;
    }
  }

#ifndef NO_GRAPHICS

  grid_to_image(0, 0, grid_width, grid_height);
  draw_grid(0, 0, grid_width, grid_height);

#endif

  // get probes
  for (i = 0; i < num_doors; i++) {

    e1 = get_farthest(i, 0);
    e2 = get_farthest(i, e1);

    e1x = doors[i].points.places[e1].x;
    e1y = doors[i].points.places[e1].y;
    e2x = doors[i].points.places[e2].x;
    e2y = doors[i].points.places[e2].y;

    mx = (e1x + e2x) / 2.0;
    my = (e1y + e2y) / 2.0;

    dx = e2x - e1x;
    dy = e2y - e1y;
    d = dist(dx, dy);

    p1x = mx + dy / d;
    p1y = my - dx / d;

    p2x = mx - dy / d;
    p2y = my + dx / d;

    doors[i].pose.theta = atan2(p2y - p1y, p2x - p1x);

    world_point.pose.x = p1x;
    world_point.pose.y = p1y;
    carmen_world_to_map(&world_point, &map_point);
    n = probe(map_point.x, map_point.y);

    doors[i].room1 = n;

    if (n == num_rooms) {
      rooms[n].num = n;
      rooms[n].doors = (int *) calloc(num_doors, sizeof(int));
      carmen_test_alloc(rooms[n].doors);
      rooms[n].num_doors = 0;
      rooms[n].name = (char *) calloc(10, sizeof(char));
      carmen_test_alloc(rooms[n].name);
      sprintf(rooms[n].name, "room %d", n);
      num_rooms++;
    }

    for (j = 0; j < rooms[n].num_doors; j++)
      if (doors[rooms[n].doors[j]].num == i)
	break;

    if (j == rooms[n].num_doors)
      rooms[n].doors[rooms[n].num_doors++] = i;

    world_point.pose.x = p2x;
    world_point.pose.y = p2y;
    carmen_world_to_map(&world_point, &map_point);
    n = probe(map_point.x, map_point.y);

    doors[i].room2 = n;

    if (n == num_rooms) {
      rooms[n].num = n;
      rooms[n].doors = (int *) calloc(num_doors, sizeof(int));
      carmen_test_alloc(rooms[n].doors);
      rooms[n].num_doors = 0;
      rooms[n].name = (char *) calloc(10, sizeof(char));
      carmen_test_alloc(rooms[n].name);
      sprintf(rooms[n].name, "room %d", n);
      num_rooms++;
    }

    for (j = 0; j < rooms[n].num_doors; j++)
      if (doors[rooms[n].doors[j]].num == i)
	break;

    if (j == rooms[n].num_doors)
      rooms[n].doors[rooms[n].num_doors++] = i;
  }
}

static void grid_init() {

  int i, j;

  grid_resolution = map.config.resolution; //dbug

  grid_width = (int) map.config.x_size;
  grid_height = (int) map.config.y_size;

  grid = (int **) calloc(grid_width, sizeof(int *));
  carmen_test_alloc(grid);

  for (i = 0; i < grid_width; i++) {
    grid[i] = (int *) calloc(grid_height, sizeof(int));
    carmen_test_alloc(grid[i]);
  }

  for (i = 0; i < grid_width; i++) {
    for (j = 0; j < grid_height; j++) {
      if (map.map[i][j] < 0.0)
	grid[i][j] = GRID_UNKNOWN;
      else if (map.map[i][j] < 0.5)
	grid[i][j] = GRID_NONE;
      else
	grid[i][j] = GRID_WALL;
    }
  }

#ifndef NO_GRAPHICS

  canvas_width = grid_width;
  canvas_height = grid_height;

  gtk_drawing_area_size(GTK_DRAWING_AREA(canvas), canvas_width, canvas_height);

  while(gtk_events_pending())
    gtk_main_iteration_do(TRUE);

#endif
}

static int closest_room(int x, int y, int max_shell) {

  int i, j, x2, y2, shell;

  if (x < 0 || x >= grid_width || y < 0 || y >= grid_height)
    return -1;

  if (grid[x][y] >= 0)
    return grid[x][y];

  for (i = 0; i < 4*max_shell; i++) {
    shell = i/4+1;
    for (j = 0; j < 2*shell; j++) {
      switch (i % 4) {
      case 0:
	x2 = carmen_clamp(0, x-shell+j, grid_width-1);
	y2 = carmen_clamp(0, y+shell, grid_height-1);
	if (grid[x2][y2] >= 0)
	  return grid[x2][y2];
	break;
      case 1:
	x2 = carmen_clamp(0, x+shell, grid_width-1);
	y2 = carmen_clamp(0, y+shell-j, grid_height-1);
	if (grid[x2][y2] >= 0)
	  return grid[x2][y2];
	break;
      case 2:
	x2 = carmen_clamp(0, x+shell-j, grid_width-1);
	y2 = carmen_clamp(0, y-shell, grid_height-1);
	if (grid[x2][y2] >= 0)
	  return grid[x2][y2];
	break;
      case 3:
	x2 = carmen_clamp(0, x-shell, grid_width-1);
	y2 = carmen_clamp(0, y-shell+j, grid_height-1);
	if (grid[x2][y2] >= 0)
	  return grid[x2][y2];
      }
    }
  }

  return -1;
}

static void cleanup_map() {

  int i, j;
  fill_node_p tmp, fill_stack;

  for (i = 0; i < grid_width; i++)
    for (j = 0; j < grid_height; j++)
      if (closest_room(i,j,7) == -1)
	grid[i][j] = GRID_UNKNOWN;

  fill_stack = NULL;
  
  for (i = 0; i < grid_width; i++) {
    for (j = 0; j < grid_height; j++) {
      if (grid[i][j] == GRID_NONE) {
	tmp = (fill_node_p) calloc(1, sizeof(fill_node_t));
	carmen_test_alloc(tmp);
	tmp->x = i;
	tmp->y = j;
	tmp->fill = closest_room(i,j,7);
	tmp->next = fill_stack;
	fill_stack = tmp;
      }
    }
  }

  while (fill_stack != NULL) {
    grid[fill_stack->x][fill_stack->y] = fill_stack->fill;
    tmp = fill_stack;
    fill_stack = fill_stack->next;
    free(tmp);
  }
}

static void get_map() {

  carmen_map_placelist_t placelist;

  carmen_map_get_gridmap(&map);
  carmen_map_get_placelist(&placelist);
  
  if (placelist.num_places == 0) {
    fprintf(stderr, "no places found\n");
    exit(1);
  }

  grid_init();
  get_doors(&placelist);
  get_rooms();
  print_doors();

  cleanup_map();

#ifndef NO_GRAPHICS
  grid_to_image(0, 0, grid_width, grid_height);
  draw_grid(0, 0, grid_width, grid_height);
#endif

}

#ifndef NO_GRAPHICS

static GdkColor grid_color(int cell) {

  switch (cell) {
  case GRID_NONE:    return carmen_white;
  case GRID_UNKNOWN: return carmen_blue;
  case GRID_WALL:    return carmen_black;
  case GRID_DOOR:    return carmen_red;
  case GRID_PROBE:   return carmen_yellow;
  }

  return carmen_green;
}

static void grid_to_image(int x0, int y0, int width, int height) {

  int x, y, x2, y2, cx, cy, cw, ch;
  GdkColor color;

  if (x0 < 0 || x0+width > grid_width || y0 < 0 || y0+height > grid_height)
    return;

  cx = (int) ((x0 / (double) grid_width) * canvas_width);
  cy = (int) ((1.0 - ((y0+height) / (double) grid_height)) * canvas_height);
  cw = (int) ((width / (double) grid_width) * canvas_width);
  ch = (int) ((height / (double) grid_height) * canvas_height);

  for(x = cx; x < cx + cw; x++) {
    for(y = cy; y < cy + ch; y++) {
      x2 = (int) ((x / (double) canvas_width) * grid_width);
      y2 = (int) ((1.0 - ((y+1) / (double) canvas_height)) * grid_height);
      color = grid_color(grid[x2][y2]);
      gdk_gc_set_foreground(drawing_gc, &color);
      gdk_draw_point(pixmap, drawing_gc, x, y);
    }
  }
}

static void draw_grid(int x, int y, int width, int height) {

  int cx, cy, cw, ch;

  if (x < 0 || x+width > grid_width || y < 0 || y+height > grid_height)
    return;

  cx = (int) ((x / (double) grid_width) * canvas_width);
  cy = (int) ((1.0 - ((y+height+1) / (double) grid_height)) * canvas_height);
  cw = (int) ((width / (double) grid_width) * canvas_width);
  ch = (int) ((height / (double) grid_height) * canvas_height);

  gdk_draw_pixmap(canvas->window,
		  canvas->style->fg_gc[GTK_WIDGET_STATE(canvas)],
		  pixmap, cx, cy, cx, cy, cw, ch);

  while(gtk_events_pending())
    gtk_main_iteration_do(TRUE);
}

static void vector2d_shift(GdkPoint *dst, GdkPoint *src, int n, int x, int y) {

  int i;

  for (i = 0; i < n; i++) {
    dst[i].x = src[i].x + x;
    dst[i].y = src[i].y + y;
  }
}

/*
 * rotate n points of src theta radians about (x,y) and put result in dst.
 */
static void vector2d_rotate(GdkPoint *dst, GdkPoint *src, int n,
			    int x, int y, double theta) {

  int i, x2, y2;
  double cos_theta, sin_theta;

  cos_theta = cos(theta);
  sin_theta = sin(theta);

  for (i = 0; i < n; i++) {
    x2 = src[i].x - x;
    y2 = src[i].y - y;
    dst[i].x = x + cos_theta*x2 - sin_theta*y2;
    dst[i].y = y + sin_theta*x2 + cos_theta*y2;
  }
}

static void vector2d_scale(GdkPoint *dst, GdkPoint *src, int n, int x, int y,
			   int width_percent, int height_percent) {

  int i, x2, y2;

  for (i = 0; i < n; i++) {
    x2 = src[i].x - x;
    y2 = src[i].y - y;
    dst[i].x = x + x2*width_percent/100.0;
    dst[i].y = y + y2*height_percent/100.0;
  }
}

static void draw_arrow(int x, int y, double theta, int width, int height,
		       GdkColor color) {

  // default arrow shape with x = y = theta = 0, width = 100, & height = 100
  static const GdkPoint arrow_shape[7] = {{0,0}, {-40, -50}, {-30, -10}, {-100, -10},
					  {-100, 10}, {-30, 10}, {-40, 50}};
  static const GdkPoint dim_shape[2] = {{-100, -50}, {0, 50}};
  GdkPoint arrow[7];
  GdkPoint dim[2];
  int dim_x, dim_y, dim_width, dim_height;

  printf("draw_arrow(%d, %d, %.2f, %d, %d, ...)\n", x, y, theta, width, height);
  
  vector2d_scale(arrow, (GdkPoint *) arrow_shape, 7, 0, 0, width, height);
  vector2d_rotate(arrow, arrow, 7, 0, 0, -theta);
  vector2d_shift(arrow, arrow, 7, x, grid_height - y);

  vector2d_scale(dim, (GdkPoint *) dim_shape, 2, 0, 0, width, height);
  vector2d_rotate(dim, dim, 2, 0, 0, theta);
  vector2d_shift(dim, dim, 2, x, y);

  gdk_gc_set_foreground(drawing_gc, &color);
  gdk_draw_polygon(pixmap, drawing_gc, 1, arrow, 7);

  dim_x = (dim[0].x < dim[1].x ? dim[0].x : dim[1].x);
  dim_y = (dim[0].y < dim[1].y ? dim[0].y : dim[1].y);
  dim_width = (dim[0].x < dim[1].x ? dim[1].x - dim[0].x : dim[0].x - dim[1].x);
  dim_height = (dim[0].y < dim[1].y ? dim[1].y - dim[0].y : dim[0].y - dim[1].y);

  printf("dim = [(%d, %d), (%d, %d)]\n", dim[0].x, dim[0].y, dim[1].x, dim[1].y);
  printf("dim_x = %d, dim_y = %d, dim_width = %d, dim_height = %d\n",
	 dim_x, dim_y, dim_width, dim_height);

  draw_grid(dim_x, dim_y, dim_width, dim_height);
}

static void erase_arrow(int x, int y, double theta, int width, int height) {

  static const GdkPoint dim_shape[2] = {{-100, -50}, {0, 50}};
  GdkPoint dim[2];
  int dim_x, dim_y, dim_width, dim_height;
  
  vector2d_scale(dim, (GdkPoint *) dim_shape, 2, 0, 0, width, height);
  vector2d_rotate(dim, dim, 2, 0, 0, theta);
  vector2d_shift(dim, dim, 2, x, y);

  dim_x = (dim[0].x < dim[1].x ? dim[0].x : dim[1].x);
  dim_y = (dim[0].y < dim[1].y ? dim[0].y : dim[1].y);
  dim_width = (dim[0].x < dim[1].x ? dim[1].x - dim[0].x : dim[0].x - dim[1].x);
  dim_height = (dim[0].y < dim[1].y ? dim[1].y - dim[0].y : dim[0].y - dim[1].y);

  grid_to_image(dim_x, dim_y, dim_width, dim_height);
  draw_grid(dim_x, dim_y, dim_width, dim_height);
}

static void draw_path() {

  carmen_world_point_t world_point;
  carmen_map_point_t map_point;
  int i, r1, r2;
  double theta;

  world_point.map = map_point.map = &map;

  r1 = r2 = room;
  theta = 0.0;

  for (i = 0; i < pathlen; i++) {
    r1 = r2;
    if (doors[path[i]].room1 == r1) {
      r2 = doors[path[i]].room2;
      theta = doors[path[i]].pose.theta;
    }
    else {
      r2 = doors[path[i]].room1;
      theta = carmen_normalize_theta(doors[path[i]].pose.theta + M_PI);
    }
    world_point.pose.x = doors[path[i]].pose.x;
    world_point.pose.y = doors[path[i]].pose.y;
    carmen_world_to_map(&world_point, &map_point);
    draw_arrow(map_point.x, map_point.y, theta, 20, 10, carmen_red);
  }
}

static void erase_path() {

  carmen_world_point_t world_point;
  carmen_map_point_t map_point;
  int i, r1, r2;
  double theta;

  world_point.map = map_point.map = &map;

  r1 = r2 = room;
  theta = 0.0;

  for (i = 0; i < pathlen; i++) {
    r1 = r2;
    if (doors[path[i]].room1 == r1) {
      r2 = doors[path[i]].room2;
      theta = doors[path[i]].pose.theta;
    }
    else {
      r2 = doors[path[i]].room1;
      theta = carmen_normalize_theta(doors[path[i]].pose.theta + M_PI);
    }
    world_point.pose.x = doors[path[i]].pose.x;
    world_point.pose.y = doors[path[i]].pose.y;
    carmen_world_to_map(&world_point, &map_point);
    erase_arrow(map_point.x, map_point.y, theta, 20, 10);
  }
}

static gint button_press_event(GtkWidget *widget __attribute__ ((unused)),
			       GdkEventMotion *event) {

  int x, y;

  x = (int) ((event->x / (double) canvas_width) * grid_width);
  y = (int) ((1.0 - ((event->y+1) / (double) canvas_height)) * grid_height);
  
  goal = closest_room(x,y,10);
  printf("goal = %d\n", goal);
  
  return TRUE;
}

static gint canvas_configure(GtkWidget *widget,
			     gpointer p __attribute__ ((unused))) {

  int display = (drawing_gc != NULL);

  canvas_width = widget->allocation.width;
  canvas_height = widget->allocation.height;

  if (pixmap)
    gdk_pixmap_unref(pixmap);

  pixmap = gdk_pixmap_new(canvas->window, canvas_width,
			  canvas_height, -1);

  if (display) {
    grid_to_image(0, 0, grid_width, grid_height);
    draw_grid(0, 0, grid_width, grid_height);
  }

  return TRUE;
}

static gint canvas_expose(GtkWidget *widget __attribute__ ((unused)),
			  GdkEventExpose *event) {

  int display = (drawing_gc != NULL);
  
  if (display) {
    //grid_to_image(0, 0, grid_width, grid_height);
    //draw_grid(0, 0, grid_width, grid_height);
    gdk_draw_pixmap(canvas->window,
		    canvas->style->fg_gc[GTK_WIDGET_STATE(canvas)],
		    pixmap, event->area.x, event->area.y,
		    event->area.x, event->area.y,
		    event->area.width, event->area.height);
  }

  return TRUE;
}

static void window_destroy(GtkWidget *w __attribute__ ((unused))) {

  gtk_main_quit();
}

static void gui_init() {

  GtkWidget *vbox;

  window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_policy(GTK_WINDOW(window), FALSE, FALSE, FALSE);
  gtk_signal_connect(GTK_OBJECT(window), "destroy",
		     GTK_SIGNAL_FUNC(window_destroy), NULL);

  vbox = gtk_vbox_new(TRUE, 0);

  canvas = gtk_drawing_area_new();

  gtk_drawing_area_size(GTK_DRAWING_AREA(canvas), canvas_width, canvas_height);

  gtk_box_pack_start(GTK_BOX(vbox), canvas, TRUE, TRUE, 0);

  gtk_signal_connect(GTK_OBJECT(canvas), "expose_event",
		     GTK_SIGNAL_FUNC(canvas_expose), NULL);

  gtk_signal_connect(GTK_OBJECT(canvas), "configure_event",
		     GTK_SIGNAL_FUNC(canvas_configure), NULL);

  gtk_signal_connect(GTK_OBJECT(canvas), "button_press_event",
		     GTK_SIGNAL_FUNC(button_press_event), NULL);

  gtk_widget_add_events(canvas, GDK_BUTTON_PRESS_MASK);

  gtk_container_add(GTK_CONTAINER(window), vbox);

  gtk_widget_show_all(window);

  drawing_gc = gdk_gc_new(canvas->window);
  carmen_graphics_setup_colors();
}

static int pq_f(path_node_p p) {

  int d, g, h, h2;

  g = p->cost;

  h = dist(doors[rooms[goal].doors[0]].pose.x - doors[p->path[p->pathlen-1]].pose.x,
	   doors[rooms[goal].doors[0]].pose.y - doors[p->path[p->pathlen-1]].pose.y);

  for (d = 1; d < rooms[goal].num_doors; d++) {
    h2 = dist(doors[rooms[goal].doors[d]].pose.x - doors[p->path[p->pathlen-1]].pose.x,
	      doors[rooms[goal].doors[d]].pose.y - doors[p->path[p->pathlen-1]].pose.y);
    if (h2 < h)
      h = h2;
  }

  return g+h;
}

static path_node_p pq_insert(path_node_p pq, path_node_p p) {

  path_node_p tmp;
  
  if (pq == NULL || pq_f(p) <= pq_f(pq)) {
    p->next = pq;
    return p;
  }

  for (tmp = pq; tmp->next; tmp = tmp->next)
    if (pq_f(p) <= pq_f(tmp->next))
      break;

  p->next = tmp->next;
  tmp->next = p;

  return pq;
}

static path_node_p pq_expand(path_node_p pq) {

  path_node_p p, tmp;
  int r, pd, rd, d;

  p = pq;
  pq = pq->next;

  r = -1;

  if (p->pathlen == 1) {
    if (room == doors[p->path[0]].room1)
      r = doors[p->path[0]].room2;
    else
      r = doors[p->path[0]].room1;
  }
  else {
    if (doors[p->path[p->pathlen-2]].room1 == doors[p->path[p->pathlen-1]].room1 ||
	doors[p->path[p->pathlen-2]].room2 == doors[p->path[p->pathlen-1]].room1)
      r = doors[p->path[p->pathlen-1]].room2;
    else
      r = doors[p->path[p->pathlen-1]].room1;
  }    

  for (rd = 0; rd < rooms[r].num_doors; rd++) {
    d = rooms[r].doors[rd];
    for (pd = 0; pd < p->pathlen; pd++)
      if (p->path[pd] == d)
	break;
    if (pd == p->pathlen) {
      tmp = (path_node_p) calloc(1, sizeof(path_node_t));
      carmen_test_alloc(tmp);
      tmp->pathlen = p->pathlen + 1;
      tmp->path = (int *) calloc(tmp->pathlen, sizeof(int));
      carmen_test_alloc(tmp->path);
      memcpy(tmp->path, p->path, p->pathlen * sizeof(int));
      tmp->path[tmp->pathlen-1] = d;
      tmp->cost = p->cost + dist(doors[d].pose.x - doors[p->path[p->pathlen-1]].pose.x,
				 doors[d].pose.y - doors[p->path[p->pathlen-1]].pose.y);
      pq = pq_insert(pq, tmp);
    }
  }

  return pq;
}

static inline int is_goal(path_node_p p) {

  return (doors[p->path[p->pathlen-1]].room1 == goal ||
	  doors[p->path[p->pathlen-1]].room2 == goal);
}

static void pq_free(path_node_p pq) {

  if (pq == NULL)
    return;

  pq_free(pq->next);

  free(pq->path);
  free(pq);
}

static path_node_p pq_init() {

  path_node_p pq, tmp;
  int rd, d;

  pq = NULL;
  d = -1;

  for (rd = 0; rd < rooms[room].num_doors; rd++) {
    d = rooms[room].doors[rd];
    tmp = (path_node_p) calloc(1, sizeof(path_node_t));
    carmen_test_alloc(tmp);
    tmp->pathlen = 1;
    tmp->path = (int *) calloc(tmp->pathlen, sizeof(int));
    tmp->path[0] = d;
    tmp->cost = dist(doors[d].pose.x - global_pos.globalpos.x,
		     doors[d].pose.y - global_pos.globalpos.y);
    pq = pq_insert(pq, tmp);
  }

  return pq;
}

static void print_path(int *p, int plen) {

  int i;

  printf("path =");  
  for (i = 0; i < plen; i++)
    printf(" %d", p[i]);
  printf("\n");
}

static int path_eq(int *path1, int pathlen1, int *path2, int pathlen2) {

  int i;

  if (path1 == NULL)
    return (path2 == NULL);

  if (path2 == NULL)
    return (path1 == NULL);

  if (pathlen1 != pathlen2)
    return 0;

  for (i = 0; i < pathlen1; i++)
    if (path1[i] != path2[i])
      return 0;

  return 1;
}

/*
 * Perform A* search with current globalpos as the starting state
 * and doors as nodes.  Returns 1 if path changed; 0 otherwise.
 */
static int get_path() {

  path_node_p pq;  //path queue
  int changed = 0;

  if (goal == room) {
    changed = !path_eq(path, pathlen, NULL, 0);
#ifndef NO_GRAPHICS
    if (changed)
      erase_path();
#endif
    pathlen = 0;
    if (path)
      free(path);
    path = NULL;
    goal = -1;
    return changed;
  }

  for (pq = pq_init(); pq != NULL; pq = pq_expand(pq)) {
    if (is_goal(pq)) {
      changed = !path_eq(path, pathlen, pq->path, pq->pathlen);
#ifndef NO_GRAPHICS
      if (changed)
	erase_path();
#endif
      pathlen = pq->pathlen;
      path = realloc(path, pathlen * sizeof(int));
      memcpy(path, pq->path, pathlen * sizeof(int));
      pq_free(pq);
#ifndef NO_GRAPHICS
      if (changed)
	draw_path();
#endif
      return changed;
    }
  }

  changed = !path_eq(path, pathlen, NULL, -1);
#ifndef NO_GRAPHICS
  if (changed)
    erase_path();
#endif
  pathlen = -1;
  if (path)
    free(path);
  path = NULL;
#ifndef NO_GRAPHICS
  if (changed)
    draw_path();
#endif
  return changed;
}

/*
 * assumes path != NULL
 */
static void publish_path_msg() {

  static carmen_gnav_path_msg path_msg;
  static int first = 1;
  IPC_RETURN_TYPE err;

  //fprintf(stderr, "publish_path_msg()\n");

  if (first) {
    strcpy(path_msg.host, carmen_get_tenchar_host_name());
    path_msg.path = NULL;
    path_msg.pathlen = 0;
    first = 0;
  }

  path_msg.timestamp = carmen_get_time_ms();

  path_msg.pathlen = pathlen;

  //fprintf(stderr, "pathlen = %d\n", pathlen);

  print_path(path, pathlen);

  if (path != NULL) {
    path_msg.path = (int *) realloc(path_msg.path, pathlen * sizeof(int));
    carmen_test_alloc(path_msg.path);
    memcpy(path_msg.path, path, pathlen * sizeof(int));
  }
  else
    path_msg.path = NULL;

  err = IPC_publishData(CARMEN_GNAV_PATH_MSG_NAME, &path_msg);
  carmen_test_ipc_exit(err, "Could not publish", CARMEN_GNAV_PATH_MSG_NAME);  
}

static void update_path() {

  if (goal < 0)
    return;

  if (get_path())
    publish_path_msg();
}

static void publish_room_msg() {

  static carmen_gnav_room_msg room_msg;
  static int first = 1;
  IPC_RETURN_TYPE err;
  
  if (first) {
    strcpy(room_msg.host, carmen_get_tenchar_host_name());
    first = 0;
  }

  room_msg.timestamp = carmen_get_time_ms();
  room_msg.room = room;

  err = IPC_publishData(CARMEN_GNAV_ROOM_MSG_NAME, &room_msg);
  carmen_test_ipc_exit(err, "Could not publish", CARMEN_GNAV_ROOM_MSG_NAME);  
}

static void gnav_rooms_topology_query_handler(MSG_INSTANCE msgRef, BYTE_ARRAY callData,
					      void *clientData __attribute__ ((unused))) {

  FORMATTER_PTR formatter;
  IPC_RETURN_TYPE err;
  carmen_gnav_rooms_topology_msg response;

  formatter = IPC_msgInstanceFormatter(msgRef);
  IPC_freeByteArray(callData);
  
  response.timestamp = carmen_get_time_ms();
  strcpy(response.host, carmen_get_tenchar_host_name());

  response.topology.rooms = (carmen_room_p) calloc(num_rooms, sizeof(carmen_room_t));
  carmen_test_alloc(response.topology.rooms);
  memcpy(response.topology.rooms, rooms, num_rooms * sizeof(carmen_room_t));
  response.topology.num_rooms = num_rooms;

  response.topology.doors = (carmen_door_p) calloc(num_doors, sizeof(carmen_room_t));
  carmen_test_alloc(response.topology.doors);
  memcpy(response.topology.doors, doors, num_doors * sizeof(carmen_door_t));
  response.topology.num_doors = num_doors;

  err = IPC_respondData(msgRef, CARMEN_GNAV_ROOMS_TOPOLOGY_MSG_NAME, &response);
  carmen_test_ipc(err, "Could not respond", CARMEN_GNAV_ROOMS_TOPOLOGY_MSG_NAME);
}

static void gnav_set_goal_handler(MSG_INSTANCE msgRef, BYTE_ARRAY callData,
				  void *clientData __attribute__ ((unused))) {

  carmen_gnav_set_goal_msg goal_msg;
  FORMATTER_PTR formatter;
  IPC_RETURN_TYPE err = IPC_OK;

  formatter = IPC_msgInstanceFormatter(msgRef);
  err = IPC_unmarshallData(formatter, callData, &goal_msg,
			   sizeof(carmen_gnav_set_goal_msg));
  IPC_freeByteArray(callData);
  carmen_test_ipc_return(err, "Could not unmarshall", IPC_msgInstanceName(msgRef));

  carmen_verbose("goal = %d\n", goal_msg.goal);

  goal = goal_msg.goal;
}

void localize_handler() {

  carmen_world_point_t world_point;
  carmen_map_point_t map_point;
  int new_room;

  world_point.map = map_point.map = &map;
  world_point.pose.x = global_pos.globalpos.x;
  world_point.pose.y = global_pos.globalpos.y;
  carmen_world_to_map(&world_point, &map_point);

  new_room = closest_room(map_point.x, map_point.y, 10);
  
  if (new_room != room) {
    room = new_room;
    printf("room = %d\n", room);
    publish_room_msg();
  }

  update_path();
}

static void messages_init() {

  IPC_RETURN_TYPE err;

  err = IPC_defineMsg(CARMEN_GNAV_ROOM_MSG_NAME, IPC_VARIABLE_LENGTH, 
		      CARMEN_GNAV_ROOM_MSG_FMT);
  carmen_test_ipc_exit(err, "Could not define", CARMEN_GNAV_ROOM_MSG_NAME);

  err = IPC_defineMsg(CARMEN_GNAV_ROOMS_TOPOLOGY_QUERY_NAME, IPC_VARIABLE_LENGTH, 
		      CARMEN_GNAV_ROOMS_TOPOLOGY_QUERY_FMT);
  carmen_test_ipc_exit(err, "Could not define message", 
		       CARMEN_GNAV_ROOMS_TOPOLOGY_QUERY_NAME);

  err = IPC_defineMsg(CARMEN_GNAV_ROOMS_TOPOLOGY_MSG_NAME, IPC_VARIABLE_LENGTH, 
		      CARMEN_GNAV_ROOMS_TOPOLOGY_MSG_FMT);
  carmen_test_ipc_exit(err, "Could not define message",
		       CARMEN_GNAV_ROOMS_TOPOLOGY_MSG_NAME);

  err = IPC_defineMsg(CARMEN_GNAV_SET_GOAL_MSG_NAME, IPC_VARIABLE_LENGTH, 
		      CARMEN_GNAV_SET_GOAL_MSG_FMT);
  carmen_test_ipc_exit(err, "Could not define", CARMEN_GNAV_SET_GOAL_MSG_NAME);

  err = IPC_defineMsg(CARMEN_GNAV_PATH_MSG_NAME, IPC_VARIABLE_LENGTH, 
		      CARMEN_GNAV_PATH_MSG_FMT);
  carmen_test_ipc_exit(err, "Could not define", CARMEN_GNAV_PATH_MSG_NAME);

  carmen_localize_subscribe_globalpos_message(&global_pos, localize_handler,
					      CARMEN_SUBSCRIBE_LATEST);

  err = IPC_subscribe(CARMEN_GNAV_ROOMS_TOPOLOGY_QUERY_NAME, 
		      gnav_rooms_topology_query_handler, NULL);
  carmen_test_ipc_exit(err, "Could not subcribe message", 
		       CARMEN_GNAV_ROOMS_TOPOLOGY_QUERY_NAME);
  IPC_setMsgQueueLength(CARMEN_GNAV_ROOMS_TOPOLOGY_QUERY_NAME, 100);

  err = IPC_subscribe(CARMEN_GNAV_SET_GOAL_MSG_NAME, gnav_set_goal_handler, NULL);
  carmen_test_ipc_exit(err, "Could not subcribe message", CARMEN_GNAV_SET_GOAL_MSG_NAME);
  IPC_setMsgQueueLength(CARMEN_GNAV_SET_GOAL_MSG_NAME, 100);
}

static gint updateIPC(gpointer *data __attribute__ ((unused))) {

  sleep_ipc(0.01);
  carmen_graphics_update_ipc_callbacks((GdkInputFunction) updateIPC);

  return 1;
}

#endif

static void params_init(int argc __attribute__ ((unused)),
			char *argv[] __attribute__ ((unused))) {

  grid_resolution = DEFAULT_GRID_RESOLUTION;
}

int main(int argc, char *argv[]) {

  int i;

  for (i = 1; i < argc; i++) {
    if (!strcmp(argv[i], "-fast"))
      fast = 1;
    else
      carmen_die("usage:  gnav [-fast]\n");      
  }

  carmen_initialize_ipc(argv[0]);
  carmen_param_check_version(argv[0]);

#ifndef NO_GRAPHICS
  gtk_init(&argc, &argv);
#endif

  params_init(argc, argv);

#ifndef NO_GRAPHICS
  gui_init();
#endif

  get_map();
  messages_init();

#ifndef NO_GRAPHICS
  carmen_graphics_update_ipc_callbacks((GdkInputFunction) updateIPC);
  gtk_main();
#endif

  return 0;
}
