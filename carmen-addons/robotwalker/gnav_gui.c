#include <carmen/carmen_graphics.h>
#include "gnav_interface.h"

static GdkGC *drawing_gc = NULL;
static GdkPixmap *pixmap = NULL;
static GtkWidget *window, *canvas;
static int canvas_width = 300, canvas_height = 300;


static void draw_grid(int x, int y, int width, int height) {

  gdk_draw_pixmap(canvas->window,
		  canvas->style->fg_gc[GTK_WIDGET_STATE(canvas)],
		  pixmap, x, y, x, y, width, height);

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

/*
static void draw_arrow(int x, int y, double theta, int width, int height,
		       GdkColor color) {

  // default arrow shape with x = y = theta = 0, width = 100, & height = 100
  static const GdkPoint arrow_shape[7] = {{0,0}, {-40, -50}, {-30, -10}, {-100, -10},
					  {-100, 10}, {-30, 10}, {-40, 50}};
  static const GdkPoint dim_shape[2] = {{-100, -50}, {0, 50}};
  GdkPoint arrow[7];
  GdkPoint dim[2];
  int dim_x, dim_y, dim_width, dim_height;

  vector2d_scale(arrow, (GdkPoint *) arrow_shape, 7, 0, 0, width, height);
  vector2d_rotate(arrow, arrow, 7, 0, 0, theta);
  vector2d_shift(arrow, arrow, 7, x, y);

  vector2d_scale(dim, (GdkPoint *) dim_shape, 2, 0, 0, width, height);
  vector2d_rotate(dim, dim, 2, 0, 0, theta);
  vector2d_shift(dim, dim, 2, x, y);

  gdk_gc_set_foreground(drawing_gc, &color);
  gdk_draw_polygon(pixmap, drawing_gc, 1, arrow, 7);

  dim_x = (dim[0].x < dim[1].x ? dim[0].x : dim[1].x);
  dim_y = (dim[0].y < dim[1].y ? dim[0].y : dim[1].y);
  dim_width = (dim[0].x < dim[1].x ? dim[1].x - dim[0].x : dim[0].x - dim[1].x);
  dim_height = (dim[0].y < dim[1].y ? dim[1].y - dim[0].y : dim[0].y - dim[1].y);

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

  draw_grid(dim_x, dim_y, dim_width, dim_height);
}
*/

static gint button_clicked(GtkWidget *widget __attribute__ ((unused)),
			   gpointer data) {

  int button = (int) data;

  printf("button %d clicked\n", button);

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

  if (display)
    draw_grid(0, 0, canvas_width, canvas_height);

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
  GtkWidget *button1;

  window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_policy(GTK_WINDOW(window), FALSE, TRUE, FALSE);
  gtk_signal_connect(GTK_OBJECT(window), "destroy",
		     GTK_SIGNAL_FUNC(window_destroy), NULL);

  vbox = gtk_vbox_new(TRUE, 0);

  canvas = gtk_drawing_area_new();

  gtk_drawing_area_size(GTK_DRAWING_AREA(canvas), canvas_width, canvas_height);

  button1 = gtk_button_new_with_label("button 1");

  gtk_signal_connect(GTK_OBJECT(button1), "clicked",
		     GTK_SIGNAL_FUNC(button_clicked), (gpointer) 1);

  gtk_box_pack_start(GTK_BOX(vbox), canvas, TRUE, TRUE, 0);
  gtk_box_pack_start(GTK_BOX(vbox), button1, TRUE, TRUE, 0);  

  gtk_signal_connect(GTK_OBJECT(canvas), "expose_event",
		     GTK_SIGNAL_FUNC(canvas_expose), NULL);

  gtk_signal_connect(GTK_OBJECT(canvas), "configure_event",
		     GTK_SIGNAL_FUNC(canvas_configure), NULL);

  gtk_container_add(GTK_CONTAINER(window), vbox);

  gtk_widget_show_all(window);

  drawing_gc = gdk_gc_new(canvas->window);
  carmen_graphics_setup_colors();
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

  gui_init();

  //ipc_init();

  carmen_graphics_update_ipc_callbacks((GdkInputFunction) updateIPC);
  gtk_main();

  return 0;
}
