#include <carmen/carmen_graphics.h>
#include <carmen/map_interface.h>
#include <carmen/map_io.h>


#define GRID_NONE     -1
#define GRID_UNKNOWN  -2
#define GRID_WALL     -3
#define GRID_DOOR     -4
#define GRID_PROBE    -5

//struct room;

/*
 * (pos.x, pos.y) is the center of the door.
 * pos.theta is between 0 and pi and indicates the 
 * angle of the positive (upward-pointing) vector
 * normal to the door at the center point.
*/
struct door {
  struct room *room1;
  struct room *room2;
  carmen_map_placelist_t points;
  carmen_point_t pos;  // in world coordinates
  double width;  // in meters
  int num;
};

struct room {
  struct door **doors;
  char *name;
  int num_doors;
  int num;
};

typedef struct door door_t, *door_p;
typedef struct room room_t, *room_p;

#define DEFAULT_GRID_RESOLUTION 0.1

typedef struct point_node {
  int x;
  int y;
  struct point_node *next;
} point_node_t, *point_node_p;

typedef struct {
  point_node_p probe_stack;
  point_node_p room_stack;
  int num;
} blob_t, *blob_p;

static carmen_map_t map;
static int **grid;
static int grid_width, grid_height;
static double grid_resolution;
static room_p rooms;
static int num_rooms;
static door_p doors;
static int num_doors;

static GdkGC *drawing_gc = NULL;
static GdkPixmap *pixmap = NULL;
static GtkWidget *window, *canvas;
static int canvas_width = 300, canvas_height = 300;

//static int cnt = 0;

static void draw_grid(int x0, int y0, int width, int height);
static void grid_to_image(int x, int y, int width, int height);


inline double dist(double x, double y) {

  return sqrt(x*x + y*y);
}

void print_doors() {

  int i;

  for (i = 0; i < num_doors; i++) {
    printf("door %d:  ", i);
    //printf("rooms(%d, %d), ", doors[i].room1->num, doors[i].room2->num);
    printf("pos(%.2f, %.2f, %.0f), ", doors[i].pos.x, doors[i].pos.y,
	   carmen_radians_to_degrees(doors[i].pos.theta));
    printf("width(%.2f)", doors[i].width);
    printf("\n");
  }
}

// fmt of place names: "<door>.<place>" 
void get_doors(carmen_map_placelist_p placelist) {

  int i, j, k, n, num_doornames, num_places, *num_doorplaces;
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
      if ((n == (int)strlen(doornames[j])) && !strncmp(doornames[j], placename, n))
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
  doors = (door_p) calloc(num_doors, sizeof(door_t));
  carmen_test_alloc(doors);

  printf("get_doors break 1\n");

  // get door places
  for (j = 0; j < num_doors; j++) {
    doors[j].num = j;
    doors[j].points.num_places = num_doorplaces[j];
    doors[j].points.places = (carmen_place_p) calloc(num_doorplaces[j], sizeof(carmen_place_t));
    carmen_test_alloc(doors[j].points.places);
    k = 0;
    for (i = 0; i < num_places; i++) {
      placename = placelist->places[i].name;
      n = strcspn(placename, ".");
      if ((n == (int)strlen(doornames[j])) && !strncmp(doornames[j], placename, n))
	memcpy(&doors[j].points.places[k++], &placelist->places[i], sizeof(carmen_place_t));
    }
    free(doornames[j]);
  }

  printf("get_doors break 2\n");

  free(doornames);
  free(num_doorplaces);
}

/* returns room number */
int probe(int px, int py) {

  blob_p blob;
  point_node_p tmp;
  int x, y, i, j, step, cell, num;
  //int bbx0, bby0, bbx1, bby1;

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

  //bbx0 = px;
  //bby0 = py;
  //bbx1 = px + step;
  //bby1 = py + step;
  
  while (blob->probe_stack != NULL) {

    x = blob->probe_stack->x;
    y = blob->probe_stack->y;
  
    //if (x < bbx0)
    //  bbx0 = x;
    //else if (x + step > bbx1)
    //  bbx1 = x + step;
    //if (y < bby0)
    //  bby0 = y;
    //else if (y + step > bby1)
    //  bby1 = y + step;

    tmp = blob->probe_stack;
    blob->probe_stack = blob->probe_stack->next;
    free(tmp);
  
    for (i = x; i < x + step; i++) {
      for (j = y; j < y + step; j++) {
	//printf("adding (%d, %d) to room_stack\n", i, j);
	grid[i][j] = GRID_PROBE;
	tmp = (point_node_p) calloc(1, sizeof(point_node_t));
	carmen_test_alloc(tmp);
	tmp->x = i;
	tmp->y = i;
	tmp->next = blob->room_stack;
	blob->room_stack = tmp;
      }
    }
    
    // up
    cell = GRID_NONE;
    for (i = x; i < x + step && cell == GRID_NONE; i++)
      for (j = y + step; j < y + 2*step && cell == GRID_NONE; j++)
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
    for (i = x; i < x + step && cell == GRID_NONE; i++)
      for (j = y - 1; j >= y - step && cell == GRID_NONE; j--)
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
    for (i = x + step; i < x + 2*step && cell == GRID_NONE; i++)
      for (j = y; j < y + step && cell == GRID_NONE; j++)
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
    for (i = x-1; i >= x - step && cell == GRID_NONE; i--)
      for (j = y; j < y + step && cell == GRID_NONE; j++)
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

    grid_to_image(x-2, y-2, step+4, step+4);
    draw_grid(x-2, y-2, step+4, step+4);
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

  return num;
}

int get_farthest(int door, int place) {

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

void get_rooms() {

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

  rooms = (room_p) calloc(num_doors + 1, sizeof(room_t));
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

  grid_to_image(0, 0, grid_width, grid_height);
  draw_grid(0, 0, grid_width, grid_height);

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

    printf("door %d:  e1 = (%.2f, %.2f), e2 = (%.2f, %.2f), m = (%.2f, %.2f)\n",
	   i+1, e1x, e1y, e2x, e2y, mx, my);
    printf("probe 1 = (%.2f, %.2f), probe 2 = (%.2f, %.2f)\n", p1x, p1y, p2x, p2y);

    world_point.pose.x = p1x;
    world_point.pose.y = p1y;
    carmen_world_to_map(&world_point, &map_point);
    n = probe(map_point.x, map_point.y);

    printf("get_rooms break 1\n");

    if (n == num_rooms) {
      rooms[n].num = n;
      rooms[n].doors = (door_p *) calloc(num_doors, sizeof(door_p));
      carmen_test_alloc(rooms[n].doors);
      rooms[n].num_doors = 0;
      rooms[n].name = (char *) calloc(10, sizeof(char));
      carmen_test_alloc(rooms[n].name);
      sprintf(rooms[n].name, "room %d", n);
      num_rooms++;
    }

    printf("get_rooms break 2\n");

    for (j = 0; j < rooms[n].num_doors; j++)
      if (rooms[n].doors[j]->num == i)
	break;

    printf("get_rooms break 3\n");

    if (j == rooms[n].num_doors)
      rooms[n].doors[num_doors++] = &doors[i];

    printf("get_rooms break 4\n");

    world_point.pose.x = p2x;
    world_point.pose.y = p2y;
    carmen_world_to_map(&world_point, &map_point);
    n = probe(map_point.x, map_point.y);

    if (n == num_rooms) {
      rooms[n].num = n;
      rooms[n].doors = (door_p *) calloc(num_doors, sizeof(door_p));
      carmen_test_alloc(rooms[n].doors);
      rooms[n].num_doors = 0;
      rooms[n].name = (char *) calloc(10, sizeof(char));
      carmen_test_alloc(rooms[n].name);
      sprintf(rooms[n].name, "room %d", n);
      num_rooms++;
    }

    for (j = 0; j < rooms[n].num_doors; j++)
      if (rooms[n].doors[j]->num == i)
	break;

    if (j == rooms[n].num_doors)
      rooms[n].doors[num_doors++] = &doors[i];
  }

  //dbug: fill in gaps in grid
  //dbug: find which doors connect which rooms
  //dbug: find door width and pose
}

void grid_init() {

  int i, j;

  grid_resolution = map.config.resolution; //dbug

  grid_width = (int) map.config.x_size;
  grid_height = (int) map.config.y_size;

  grid = (int **) malloc(grid_width * sizeof(int *));
  carmen_test_alloc(grid);

  for (i = 0; i < grid_width; i++) {
    grid[i] = (int *) malloc(grid_height * sizeof(int));
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

  canvas_width = grid_width;
  canvas_height = grid_height;

  gtk_drawing_area_size(GTK_DRAWING_AREA(canvas), canvas_width, canvas_height);

  while(gtk_events_pending())
    gtk_main_iteration_do(TRUE);
}

void get_map() {

  carmen_map_placelist_t placelist;

  carmen_map_get_gridmap(&map);
  carmen_map_get_placelist(&placelist);
  
  if (placelist.num_places == 0) {
    fprintf(stderr, "no places found\n");
    exit(1);
  }

  grid_init();
  printf("get_map break 1\n");
  get_doors(&placelist);
  printf("get_map break 2\n");
  print_doors();
  printf("get_map break 3\n");
  printf("\n");
  get_rooms();
  printf("get_map break 4\n");
}

static GdkColor grid_color(int cell) {

  switch (cell) {
  case GRID_NONE:    return carmen_white;
  case GRID_UNKNOWN: return carmen_blue;
  case GRID_WALL:    return carmen_black;
  case GRID_DOOR:    return carmen_red;
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
      //printf("break %d%d1\n", x, y);
      x2 = (int) ((x / (double) canvas_width) * grid_width);
      y2 = (int) ((1.0 - ((y+1) / (double) canvas_height)) * grid_height);
      //printf("x2 = %d, y2 = %d\n", x2, y2);
      color = grid_color(grid[x2][y2]);
      //printf("break %d%d2\n", x, y);
      gdk_gc_set_foreground(drawing_gc, &color);
      //printf("break %d%d3\n", x, y);
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
  
  if (display)
    gdk_draw_pixmap(canvas->window,
		    canvas->style->fg_gc[GTK_WIDGET_STATE(canvas)],
		    pixmap, event->area.x, event->area.y,
		    event->area.x, event->area.y,
		    event->area.width, event->area.height);

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

  //gtk_signal_connect(GTK_OBJECT(canvas), "button_press_event",
  //                   GTK_SIGNAL_FUNC(canvas_button_press), NULL);

  //gtk_widget_add_events(canvas, GDK_BUTTON_PRESS_MASK);

  gtk_container_add(GTK_CONTAINER(window), vbox);

  gtk_widget_show_all(window);

  drawing_gc = gdk_gc_new(canvas->window);
  carmen_graphics_setup_colors();
}

static void params_init(int argc __attribute__ ((unused)),
			char *argv[] __attribute__ ((unused))) {

  grid_resolution = DEFAULT_GRID_RESOLUTION;
}

static gint updateIPC(gpointer *data __attribute__ ((unused))) {

  sleep_ipc(0.01);
  carmen_graphics_update_ipc_callbacks((GdkInputFunction) updateIPC);

  return 1;
}

int main(int argc, char *argv[]) {

  printf("break 1\n");

  carmen_initialize_ipc(argv[0]);
  carmen_param_check_version(argv[0]);

  printf("break 2\n");

  gtk_init(&argc, &argv);

  printf("break 3\n");

  params_init(argc, argv);
  printf("break 4\n");
  gui_init();
  printf("break 5\n");
  get_map();

  printf("break 6\n");

  carmen_graphics_update_ipc_callbacks((GdkInputFunction) updateIPC);

  printf("break 7\n");

  gtk_main();

  printf("break 8\n");

  return 0;
}
