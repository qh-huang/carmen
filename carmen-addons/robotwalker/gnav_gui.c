#include <carmen/carmen_graphics.h>
#include "walker_interface.h"
#include "walkerserial_interface.h"
#include "gnav_interface.h"

/*********************************
Setting functions for controlling the GUI
*/
static void increaseDistanceTraveled(int distanceIncrease);
static void setDistanceTraveled(int distance);
static void setRoomName(char* roomName);
static void setArrowAngle(double theta);

/**********************************
 * Graphics Utility functions
 */
static void vector2d_rotate(GdkPoint *dst, GdkPoint *src, int n,
			    int x, int y, double theta);
static void vector2d_scale(GdkPoint *dst, GdkPoint *src, int n, int x, int y,
			   int width_percent, int height_percent);
static void vector2d_shift(GdkPoint *dst, GdkPoint *src, int n, int x, int y);

/**********************************
 * Drawing functions
 */
static void draw_rect(GdkPixmap *pixmap, int x, int y, int width, int height, GdkColor color);
static void draw_arrow(GdkPixmap *pixmap, int x, int y, double theta, int width, int height,
		       GdkColor color);
static void grid_to_image(GdkPixmap *pixmap, int width, int height, int radius);

/**********************************
 * Drawing canvases
 */
static void draw_dest_canvas();
static void draw_traveledLabel_canvas();
static void draw_left_canvas();
static void draw_right_canvas(int do_draw_arrow);

static gint canvas_configure(GtkWidget *widget,
			     gpointer p __attribute__ ((unused)));

/**********************************
 * Message handling functions
 */

static GdkGC *drawing_gc = NULL;
static GdkPixmap *leftPixmap = NULL, 
  *rightPixmap = NULL, 
  *destPixmap=NULL,
  *traveledLabelPixmap=NULL;
static GtkWidget *window;
static GtkWidget *leftCanvas,
  *rightCanvas,
  *destCanvas,
  *traveledLabelCanvas;
static int left_canvas_width = 500, left_canvas_height = 500,
  right_canvas_width = 500, right_canvas_height = 500,
  dest_label_canvas_width = 500, dest_label_canvas_height = 50,
  traveled_label_canvas_width = 500, traveled_label_canvas_height = 50;
static int label_height=50;
static double arrowAngle = 1.0;
static GtkWidget *distanceLabel;
static GtkWidget *directionsLabel;
static char dest_name[128], room_name[128];

static GdkFont* font;
static GdkColor backgroundColor;
static GdkColor fontColor;

static carmen_map_t map;
static int **grid;
static int grid_width, grid_height;
static double grid_resolution;

static int room = -1;
static int pathlen = -1;
carmen_localize_globalpos_message global_pos;
carmen_gnav_room_msg room_msg;
carmen_gnav_path_msg path_msg;
carmen_rooms_topology_p rooms_topology = NULL;

static int distanceTraveled=567;



/**********************************
Setting functions for controlling the GUI
*/


static void increaseDistanceTraveled(int distanceIncrease) {
  char setString[100];

  distanceTraveled+=distanceIncrease;
  sprintf(setString, "%dft", distanceTraveled);
  gtk_label_set_text(GTK_LABEL(distanceLabel), setString);
  canvas_configure(traveledLabelCanvas, NULL);  
}

static void setDistanceTraveled(int distance) {
  char setString[100];

  distanceTraveled=distance;
  sprintf(setString, "%dft", distanceTraveled);
  gtk_label_set_text(GTK_LABEL(distanceLabel), setString);
  canvas_configure(traveledLabelCanvas, NULL);
}

static void setRoomName(char *newName) {

  sprintf(room_name, "%s", newName);
}

static void setArrowAngle(double theta) {

  arrowAngle = theta;
}

/**********************************
 * Graphics Utility functions
 */

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

static void vector2d_shift(GdkPoint *dst, GdkPoint *src, int n, int x, int y) {

  int i;

  for (i = 0; i < n; i++) {
    dst[i].x = src[i].x + x;
    dst[i].y = src[i].y + y;
  }
}

static void draw_rect(GdkPixmap *pixmap, int x, int y, int width, int height, GdkColor color) {
  static const GdkPoint shape1[7] = {{-100,-100}, {100, -100}, {100, 100}, {-100, 100}};

  GdkPoint shape2[7];
  
  vector2d_scale(shape2, (GdkPoint *) shape1, 7, 0, 0, width, height);
  //  vector2d_rotate(shape2, shape2, 7, 0, 0, -theta);
  vector2d_shift(shape2, shape2, 7, x, y);

  gdk_gc_set_foreground(drawing_gc, &color);
  gdk_draw_polygon(pixmap, drawing_gc, 1, shape2, 7);
  
}

static void draw_arrow(GdkPixmap *pixmap, int x, int y, double theta, int width, int height,
		       GdkColor color) {

  // default arrow shape with x = y = theta = 0, width = 100, & height = 100
  //static const GdkPoint arrow_shape[7] = {{0,0}, {-40, -50}, {-30, -10}, {-100, -10},
  //					  {-100, 10}, {-30, 10}, {-40, 50}};
  static const GdkPoint arrow_shape[7] = {{50,0}, {15, -50}, {20, -15}, {-50, -15},
  					  {-50, 15}, {20, 15}, {15, 50}};


  GdkPoint arrow[7];
  //  int dim_x1, dim_y1, dim_x2, dim_y2, i;

  printf("draw_arrow(%d, %d, %.2f, %d, %d, ...)\n", x, y, theta, width, height);
  
  vector2d_scale(arrow, (GdkPoint *) arrow_shape, 7, 0, 0, width, height);
  vector2d_rotate(arrow, arrow, 7, 0, 0, -theta);
  vector2d_shift(arrow, arrow, 7, x, y);

  gdk_gc_set_foreground(drawing_gc, &color);
  gdk_draw_polygon(pixmap, drawing_gc, 1, arrow, 7);
}

static GdkColor grid_color(int x, int y) {

  if (x >= 0 && y >= 0 && x < grid_width && y < grid_height) {
    switch (grid[x][y]) {
    case GRID_NONE:    return carmen_white;
    case GRID_WALL:    return carmen_black;
    }
  }

  return carmen_blue;
}

void grid_to_image(GdkPixmap *pixmap, int width, int height, int radius) {

  static const double r = 10.0;  // radius in meters
  double gr;  // radius in grid units
  int x, y, i, j, px, py;
  double theta;
  carmen_world_point_t world_point;
  carmen_map_point_t map_point;
  GdkColor color;
  GdkPoint p;

  world_point.map = map_point.map = &map;
  world_point.pose.x = global_pos.globalpos.x;
  world_point.pose.y = global_pos.globalpos.y;
  carmen_world_to_map(&world_point, &map_point);
  
  x = map_point.x;
  y = map_point.y;
  theta = global_pos.globalpos.theta;

  gr = 2.0 * r / grid_resolution;  //dbug?

  px = (int)(width/2.0);
  py = (int)(height/2.0);

  for (i = (int)((width - radius)/2.0); i < (int)((width + radius)/2.0); i++) {
    for (j = (int)((height - radius)/2.0); j < (int)((height + radius)/2.0); j++) {
      p.x = x + (int)((i-px)*gr/(double)radius);
      p.y = y + (int)((py-j)*gr/(double)radius);
      //printf("p = (%d, %d)", p.x, p.y);
      vector2d_rotate(&p, &p, 1, x, y, theta - M_PI/2.0);
      //printf(", (%d, %d)\n", p.x, p.y);
      color = grid_color(p.x, p.y);
      gdk_gc_set_foreground(drawing_gc, &color);
      gdk_draw_point(pixmap, drawing_gc, i, j);
    }
  }

  gdk_gc_set_foreground(drawing_gc, &carmen_red);
  gdk_draw_arc(pixmap, drawing_gc, 1, px-6, py-6, 12, 12, 0, 360*64);
  gdk_gc_set_foreground(drawing_gc, &carmen_black);
  gdk_draw_arc(pixmap, drawing_gc, 0, px-6, py-6, 12, 12, 0, 360*64);
}


/**********************************
 * Drawing canvases
 */
static void draw_dest_canvas() {

  int fontHeight;

  draw_rect(destPixmap, 0, 0, dest_label_canvas_width,
	    dest_label_canvas_height, backgroundColor);

  gdk_gc_set_foreground(drawing_gc, &fontColor);
  fontHeight=gdk_text_height(font, dest_name, 21);
  //  if (canvas_height<fontHeight || canvas_height>fontHeight+30)
  //  gtk_drawing_area_size(GTK_DRAWING_AREA(destCanvas), canvas_width, fontHeight);
  label_height=fontHeight;
  gdk_draw_string(destPixmap,
		font,
		drawing_gc,
		10, fontHeight, dest_name);

  gdk_draw_pixmap(destCanvas->window,
		  destCanvas->style->fg_gc[GTK_WIDGET_STATE(destCanvas)],
		  destPixmap, 0, 0, 0, 0, dest_label_canvas_width,
		  dest_label_canvas_height);
}

static void draw_traveledLabel_canvas() {

  int fontHeight;
  char setString[100];

  sprintf(setString, "%dft", distanceTraveled);

  draw_rect(traveledLabelPixmap, 0, 0, traveled_label_canvas_width,
	    traveled_label_canvas_height, backgroundColor);

  gdk_gc_set_foreground(drawing_gc, &fontColor);
  fontHeight=gdk_text_height(font, setString, 21);
  label_height=fontHeight;
  gdk_draw_string(traveledLabelPixmap,
		font,
		drawing_gc,
		10, fontHeight, setString);

  gdk_draw_pixmap(traveledLabelCanvas->window,
		  traveledLabelCanvas->style->fg_gc[GTK_WIDGET_STATE(traveledLabelCanvas)],
		  traveledLabelPixmap, 0, 0, 0, 0, traveled_label_canvas_width,
		  traveled_label_canvas_height);
}

static void draw_right_canvas(int do_draw_arrow) {

  draw_rect(rightPixmap, 0, 0, right_canvas_width, right_canvas_height, backgroundColor);
  if (do_draw_arrow)
    draw_arrow(rightPixmap, right_canvas_width/2, right_canvas_height/2,
	       arrowAngle, 400, 200, carmen_red);

  gdk_draw_pixmap(rightCanvas->window,
		  rightCanvas->style->fg_gc[GTK_WIDGET_STATE(rightCanvas)],
		  rightPixmap, 0, 0, 0, 0, right_canvas_width, right_canvas_height);
}

static void draw_left_canvas() {

  draw_rect(leftPixmap, 0, 0, left_canvas_width, left_canvas_height, backgroundColor);
  grid_to_image(leftPixmap, left_canvas_width, left_canvas_height,
		(left_canvas_width < left_canvas_height ? left_canvas_width/2 - 1 :
		 left_canvas_height/2 - 1));

  gdk_draw_pixmap(leftCanvas->window,
		  leftCanvas->style->fg_gc[GTK_WIDGET_STATE(leftCanvas)],
		  leftPixmap, 0, 0, 0, 0, left_canvas_width, left_canvas_height);
}


static gint canvas_configure(GtkWidget *widget,
			     gpointer p __attribute__ ((unused))) {

  int display = (drawing_gc != NULL);

  /*  if (widget==leftCanvas) {
    printf("leftCanvas configure\n");
  } else if (widget==rightCanvas) {
    printf("rightCanvas configure\n");
  } else if (widget==destCanvas) {
    printf("dest configure\n");
  } else {
    printf("other Canvas configure\n");
    }*/

  if (display) {
    // Decide which draw to do:
    if (widget==rightCanvas) {
    // right
      if (rightPixmap)
	gdk_pixmap_unref(rightPixmap);
      right_canvas_width = widget->allocation.width;
      right_canvas_height = widget->allocation.height;
      rightPixmap = gdk_pixmap_new(widget->window, right_canvas_width,
			      right_canvas_height, -1);
      draw_right_canvas(0);
    } else if (widget==leftCanvas) {
    // left
      if (leftPixmap)
	gdk_pixmap_unref(leftPixmap);
      left_canvas_width = widget->allocation.width;
      left_canvas_height = widget->allocation.height;
      leftPixmap = gdk_pixmap_new(widget->window, left_canvas_width,
				  left_canvas_height, -1);
      draw_left_canvas();
    } else if (widget==destCanvas) {
    // dest
      if (destPixmap)
	gdk_pixmap_unref(destPixmap);
      dest_label_canvas_width = widget->allocation.width;
      dest_label_canvas_height = widget->allocation.height;
      destPixmap = gdk_pixmap_new(widget->window, dest_label_canvas_width,
					   dest_label_canvas_height, -1);
      draw_dest_canvas();
    } else if (widget==traveledLabelCanvas) {
    // traveledLabel
      if (traveledLabelPixmap)
	gdk_pixmap_unref(traveledLabelPixmap);
      traveled_label_canvas_width = widget->allocation.width;
      traveled_label_canvas_height = widget->allocation.height;
      traveledLabelPixmap = gdk_pixmap_new(widget->window, traveled_label_canvas_width,
			      traveled_label_canvas_height, -1);
      draw_traveledLabel_canvas();
    }
    while(gtk_events_pending())
      gtk_main_iteration_do(TRUE);
  }

  //  printf("end Canvas configure\n");
  return TRUE;
}

/**********************************
 */
static gint canvas_expose(GtkWidget *widget __attribute__ ((unused)),
			  GdkEventExpose *event) {

  int display = (drawing_gc != NULL);
  
  if (display) {

    gdk_draw_pixmap(rightCanvas->window,
		    rightCanvas->style->fg_gc[GTK_WIDGET_STATE(rightCanvas)],
		    rightPixmap, event->area.x, event->area.y,
		    event->area.x, event->area.y,
		    event->area.width, event->area.height);

    gdk_draw_pixmap(leftCanvas->window,
		    leftCanvas->style->fg_gc[GTK_WIDGET_STATE(leftCanvas)],
		    leftPixmap, event->area.x, event->area.y,
		    event->area.x, event->area.y,
		    event->area.width, event->area.height);

    gdk_draw_pixmap(destCanvas->window,
		    destCanvas->style->fg_gc[GTK_WIDGET_STATE(destCanvas)],
		    destPixmap, event->area.x, event->area.y,
		    event->area.x, event->area.y,
		    event->area.width, event->area.height);

    gdk_draw_pixmap(traveledLabelCanvas->window,
		    traveledLabelCanvas->style->fg_gc[GTK_WIDGET_STATE(traveledLabelCanvas)],
		    traveledLabelPixmap, event->area.x, event->area.y,
		    event->area.x, event->area.y,
		    event->area.width, event->area.height);
  }
  return TRUE;
}


/**************************
 * Main-ish functions
 */
static void window_destroy(GtkWidget *w __attribute__ ((unused))) {
  gtk_main_quit();
}

static void gui_init() {

  GtkWidget *vbox;
  GtkWidget *hboxTop;
  GtkWidget *hbox;
  GtkWidget *canvasHbox;

  window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_policy(GTK_WINDOW(window), FALSE, TRUE, FALSE);
  gtk_signal_connect(GTK_OBJECT(window), "destroy",
		     GTK_SIGNAL_FUNC(window_destroy), NULL);

  vbox = gtk_vbox_new(FALSE, 0);
  hbox = gtk_hbox_new(TRUE, 0);
  canvasHbox = gtk_hbox_new(TRUE, 0);
  hboxTop = gtk_hbox_new(FALSE, 0);

  distanceLabel = gtk_label_new("567ft");
  dest_name[0] = '\0';
  directionsLabel = gtk_label_new("Turn Right");


  leftCanvas = gtk_drawing_area_new();
  rightCanvas = gtk_drawing_area_new();
  destCanvas = gtk_drawing_area_new();
  traveledLabelCanvas = gtk_drawing_area_new();

  gtk_drawing_area_size(GTK_DRAWING_AREA(leftCanvas),
			left_canvas_width, left_canvas_height);
  gtk_drawing_area_size(GTK_DRAWING_AREA(rightCanvas),
			right_canvas_width, right_canvas_height);
  gtk_drawing_area_size(GTK_DRAWING_AREA(traveledLabelCanvas),
			traveled_label_canvas_width, traveled_label_canvas_height);
  gtk_drawing_area_size(GTK_DRAWING_AREA(destCanvas),
			dest_label_canvas_width, dest_label_canvas_height);

  gtk_box_pack_start(GTK_BOX(vbox), hboxTop, TRUE, TRUE, 0);
  gtk_box_pack_start(GTK_BOX(hboxTop), traveledLabelCanvas, TRUE, TRUE, 0);
  gtk_box_pack_start(GTK_BOX(hboxTop), destCanvas, TRUE, TRUE, 0);

  gtk_box_pack_start(GTK_BOX(vbox), canvasHbox, TRUE, TRUE, 0);
  gtk_box_pack_start(GTK_BOX(canvasHbox), leftCanvas, TRUE, TRUE, 0);
  gtk_box_pack_start(GTK_BOX(canvasHbox), rightCanvas, TRUE, TRUE, 0);

  gtk_box_pack_start(GTK_BOX(vbox), directionsLabel, TRUE, TRUE, 0);
  gtk_box_pack_start(GTK_BOX(vbox), hbox, TRUE, TRUE, 0);

  gtk_signal_connect(GTK_OBJECT(leftCanvas), "expose_event",
		     GTK_SIGNAL_FUNC(canvas_expose), NULL);

  gtk_signal_connect(GTK_OBJECT(leftCanvas), "configure_event",
		     GTK_SIGNAL_FUNC(canvas_configure), NULL);

  gtk_signal_connect(GTK_OBJECT(rightCanvas), "expose_event",
		     GTK_SIGNAL_FUNC(canvas_expose), NULL);

  gtk_signal_connect(GTK_OBJECT(rightCanvas), "configure_event",
		     GTK_SIGNAL_FUNC(canvas_configure), NULL);

  gtk_signal_connect(GTK_OBJECT(destCanvas), "expose_event",
		     GTK_SIGNAL_FUNC(canvas_expose), NULL);

  gtk_signal_connect(GTK_OBJECT(destCanvas), "configure_event",
		     GTK_SIGNAL_FUNC(canvas_configure), NULL);

  gtk_signal_connect(GTK_OBJECT(traveledLabelCanvas), "expose_event",
		     GTK_SIGNAL_FUNC(canvas_expose), NULL);

  gtk_signal_connect(GTK_OBJECT(traveledLabelCanvas), "configure_event",
		     GTK_SIGNAL_FUNC(canvas_configure), NULL);

  gtk_container_add(GTK_CONTAINER(window), vbox);

  gtk_widget_show_all(window);

  drawing_gc = gdk_gc_new(rightCanvas->window);


  // We want to make this larger, but 
  font = gdk_font_load("-*-helvetica-bold-r-normal--24-*-*-*-*-*-iso8859-1");  

  carmen_graphics_setup_colors();
  backgroundColor=carmen_white;
  fontColor=carmen_black;
  
  canvas_configure(leftCanvas, NULL);
  canvas_configure(rightCanvas, NULL);
  canvas_configure(destCanvas, NULL);
  canvas_configure(traveledLabelCanvas, NULL);
}

void arrow_update() {

  carmen_door_p next_door;
  double x, y, theta;

  printf("arrow_update:  room = %d, pathlen = %d\n", room, pathlen);

  if (room == -1)
    return;

  if (rooms_topology == NULL)
    rooms_topology = carmen_gnav_get_rooms_topology();

  if (pathlen < 0)  // no goal
    return;
  
  if (pathlen == 0) {  // goal reached
    draw_right_canvas(0);
    return;
  }

  next_door = &rooms_topology->doors[path_msg.path[0]];
  x = global_pos.globalpos.x;
  y = global_pos.globalpos.y;
  theta = global_pos.globalpos.theta;

  if (sqrt((next_door->pose.x-x)*(next_door->pose.x-x) +
	   (next_door->pose.y-y)*(next_door->pose.y-y)) < 0.5) {
    if (pathlen == 1) {
      draw_right_canvas(0);
      return;
    }
    next_door = &rooms_topology->doors[path_msg.path[1]];
  }

  setArrowAngle(M_PI/2.0 - theta +
		atan2(next_door->pose.y - y, next_door->pose.x - x));

  draw_right_canvas(1);
}

void room_handler() {

  printf("in room_handler room: %d\n", room_msg.room);

  room = room_msg.room;
  arrow_update();
}

void path_handler() {

  pathlen = path_msg.pathlen;
  arrow_update();
}

void button_handler(carmen_walkerserial_button_msg *button_msg) {

  static int buttons[6] = {0, 1, 2, 3, 4, 5};
  int goal;

  goal = buttons[button_msg->button];
  carmen_walker_set_goal(goal);
}

void goal_changed_handler(carmen_walker_goal_changed_msg *goal_changed_msg) {

  sprintf(dest_name, "To %s",
	  rooms_topology->rooms[goal_changed_msg->goal].name);
  draw_dest_canvas();
}

void localize_handler() {

  static int first = 1;
  static long last_update;
  struct timeval time;
  long cur_time;

  gettimeofday(&time, NULL);
  cur_time = 1000*time.tv_sec + time.tv_usec/1000;

  if (first) {
    first = 0;
    last_update = cur_time;
  }
  else if (cur_time - last_update > 500) {
    last_update = cur_time;
    draw_left_canvas();
  }

  arrow_update();
}

void grid_init() {

  int i, j;

  carmen_map_get_gridmap(&map);

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

  /*
  while(gtk_events_pending())
    gtk_main_iteration_do(TRUE);
  */
}

void ipc_init() {

  carmen_localize_subscribe_globalpos_message(&global_pos, localize_handler,
					      CARMEN_SUBSCRIBE_LATEST);

  carmen_gnav_subscribe_room_message(&room_msg, room_handler,
				      CARMEN_SUBSCRIBE_LATEST);

  carmen_gnav_subscribe_path_message(&path_msg, path_handler,
				      CARMEN_SUBSCRIBE_LATEST);

  carmen_walkerserial_subscribe_button_message(NULL,
					       (carmen_handler_t)
					       button_handler,
					       CARMEN_SUBSCRIBE_LATEST);

  carmen_walker_subscribe_goal_changed_message(NULL,
					       (carmen_handler_t)
					       goal_changed_handler,
					       CARMEN_SUBSCRIBE_LATEST);
}

static gint updateIPC(gpointer *data __attribute__ ((unused))) {

  sleep_ipc(0.01);
  carmen_graphics_update_ipc_callbacks((GdkInputFunction) updateIPC);

  return 1;
}

int main(int argc, char **argv) {

  carmen_initialize_ipc(argv[0]);
  carmen_param_check_version(argv[0]);

  gtk_init(&argc, &argv);

  grid_init();
  gui_init();
  ipc_init();

  carmen_graphics_update_ipc_callbacks((GdkInputFunction) updateIPC);
  gtk_main();

  return 0;
}
