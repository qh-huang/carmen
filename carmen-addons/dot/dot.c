#ifdef NO_GRAPHICS
#include <carmen/carmen.h>
#else
#include <carmen/carmen_graphics.h>
#endif
#include "dot_messages.h"
#include "dot.h"
#include <gsl/gsl_linalg.h>


#define MI(M,i,j) ((i)*(M)->tda + (j))

typedef struct dot_person_filter {
  carmen_list_t *sensor_update_list;
  carmen_list_t *unsensor_update_list;  // long readings
  int hidden_cnt;
  double x0;
  double y0;
  double x;
  double y;
  double logr;  // natural log of circle radius
  gsl_matrix *P;  // estimated state covariance matrix: rows and columns in order of: x, y, logr
  double a;
  gsl_matrix *Q;  // process noise covariance matrix
  gsl_matrix *R;  // sensor noise covariance matrix
} carmen_dot_person_filter_t, *carmen_dot_person_filter_p;

typedef struct dot_trash_filter {
  carmen_list_t *sensor_update_list;
  carmen_list_t *unsensor_update_list;
  double x;
  double y;
  double theta;
  double logw1;  // natural log of major eigenvalue
  double logw2;  // natural log of minor eigenvalue
  gsl_matrix *P;  // estimated state covariance matrix: rows and columns in order of: x, y, theta, logw1, logw2
  double a;
  gsl_matrix *Q;  // process noise covariance matrix
  gsl_matrix *R;  // sensor noise covariance matrix
} carmen_dot_trash_filter_t, *carmen_dot_trash_filter_p;

typedef struct dot_door_filter {
  carmen_list_t *sensor_update_list;
  carmen_list_t *unsensor_update_list;
  double x;
  double y;
  double t;  //theta
  double px;
  double py;
  double pt;
  double pxy;
  double a;
  double qx;
  double qy;
  double qxy;
  double qt;
} carmen_dot_door_filter_t, *carmen_dot_door_filter_p;

typedef struct dot_filter {
  carmen_dot_person_filter_t person_filter;
  carmen_dot_trash_filter_t trash_filter;
  carmen_dot_door_filter_t door_filter;
  int id;
  int type;
  int allow_change;
  int sensor_update_cnt;
  int do_motion_update;
  int updated;
  int invisible;
  int invisible_cnt;
} carmen_dot_filter_t, *carmen_dot_filter_p;


static double default_person_filter_a;
static double default_person_filter_px;
static double default_person_filter_py;
static double default_person_filter_pxy;
static double default_person_filter_qx;
static double default_person_filter_qy;
static double default_person_filter_qxy;
static double default_person_filter_qlogr;
static double default_person_filter_rx;
static double default_person_filter_ry;
static double default_person_filter_rxy;
static double default_trash_filter_px;
static double default_trash_filter_py;
static double default_trash_filter_a;
static double default_trash_filter_qx;
static double default_trash_filter_qy;
static double default_trash_filter_qxy;
static double default_trash_filter_qtheta;
static double default_trash_filter_qlogw1;
static double default_trash_filter_qlogw2;
static double default_trash_filter_rx;
static double default_trash_filter_ry;
static double default_trash_filter_rxy;
static double default_door_filter_px;
static double default_door_filter_py;
static double default_door_filter_pt;
static double default_door_filter_a;
static double default_door_filter_qx;
static double default_door_filter_qy;
static double default_door_filter_qxy;
static double default_door_filter_qt;
static double default_door_filter_rx;
static double default_door_filter_ry;
static double default_door_filter_rt;

static int new_filter_threshold;
static double new_cluster_threshold;
static double map_diff_threshold;
static double map_diff_threshold_scalar;
static double map_occupied_threshold;
static double sensor_update_dist;
static int sensor_update_cnt;

static int kill_hidden_person_cnt;
static double laser_max_range;
static double see_through_stdev;
static double trace_resolution;
static int invisible_cnt;
static double person_filter_displacement_threshold;

static carmen_localize_param_t localize_params;
static carmen_localize_map_t localize_map;
static carmen_robot_laser_message laser_msg;
static carmen_localize_globalpos_message odom;
static carmen_point_t last_sensor_update_odom;
static int do_sensor_update = 1;
static carmen_map_t static_map;
static carmen_dot_filter_p filters = NULL;
static int num_filters = 0;
static int filter_highlight = -1;


static inline int is_in_map(int x, int y) {

  return (x >= 0 && y >= 0 && x < static_map.config.x_size &&
	  y < static_map.config.y_size);
}


/*********** GRAPHICS ************/

#ifdef HAVE_GRAPHICS

static GtkWidget *canvas;
static GdkPixmap *pixmap = NULL, *map_pixmap = NULL;
static int pixmap_xpos, pixmap_ypos;
static int canvas_width = 804, canvas_height = 804;
static GdkGC *drawing_gc = NULL;

static int laser_mask[500];

/*
static void draw_points() {

  int i;

  for (i = 0; i < num_points; i++) {
    if (cluster_mask[i])
      gdk_gc_set_foreground(drawing_gc, &carmen_red);
    else
      gdk_gc_set_foreground(drawing_gc, &carmen_black);
    gdk_draw_arc(pixmap, drawing_gc, TRUE, (int)xpoints[i], (int)ypoints[i],
		 10, 10, 0, 360 * 64);
  }
}
*/

static void get_map_window() {

  carmen_map_point_t mp;
  carmen_world_point_t wp;
  
  wp.map = &static_map;
  wp.pose.x = odom.globalpos.x;
  wp.pose.y = odom.globalpos.y;

  carmen_world_to_map(&wp, &mp);

  pixmap_xpos = 4*(mp.x-100);
  pixmap_ypos = 4*(static_map.config.y_size - mp.y - 101);

  //pixmap_xpos = carmen_clamp(0, pixmap_xpos, 4*(static_map.config.x_size - 200));
  //pixmap_ypos = carmen_clamp(0, pixmap_ypos, 4*(static_map.config.y_size - 200));  
}

static void draw_map() {

  int x, y;
  double p = 0.0;
  GdkColor color;
  static int first = 1;

  if (!first)
    return;
  
  first = 0;

  for (x = 0; x < static_map.config.x_size; x++) {
    for (y = 0; y < static_map.config.y_size; y++) {
      if (is_in_map(x,y))
	p = exp(localize_map.prob[x][y]);
      else
	p = -1.0;
      if (p >= 0.0)
	color = carmen_graphics_add_color_rgb((int)(256*(1.0-p)), (int)(256*(1.0-p)), (int)(256*(1.0-p)));
      else
	color = carmen_blue;
      gdk_gc_set_foreground(drawing_gc, &color);
      gdk_draw_rectangle(map_pixmap, drawing_gc, TRUE, 4*x, 4*(static_map.config.y_size-y-1), 4, 4);
    }
  }
}

static void draw_robot() {

  gdk_gc_set_foreground(drawing_gc, &carmen_red);
  gdk_draw_arc(pixmap, drawing_gc, TRUE, canvas_width/2 - 5, canvas_height/2 - 5,
	       10, 10, 0, 360 * 64);
  gdk_gc_set_foreground(drawing_gc, &carmen_black);
  gdk_draw_arc(pixmap, drawing_gc, FALSE, canvas_width/2 - 5, canvas_height/2 - 5,
	       10, 10, 0, 360 * 64);
}

static int laser_point_belongs_to_dot(carmen_dot_filter_p f, int laser_index) {

  int i, s;

  for (i = 0; i < f->person_filter.sensor_update_list->length; i++) {
    s = *(int*)carmen_list_get(f->person_filter.sensor_update_list, i);
    if (s == laser_index)
      return 1;
  }

  return 0;
}

static void draw_laser() {

  int i, xpos, ypos;
  double x, y, theta;
  carmen_map_point_t mp;
  carmen_world_point_t wp;
  
  wp.map = &static_map;

  for (i = 0; i < laser_msg.num_readings; i++) {
    if (laser_msg.range[i] < laser_max_range) {
      theta = carmen_normalize_theta(laser_msg.theta + (i-90)*M_PI/180.0);
      x = laser_msg.x + cos(theta) * laser_msg.range[i];
      y = laser_msg.y + sin(theta) * laser_msg.range[i];
      wp.pose.x = x;
      wp.pose.y = y;
      carmen_world_to_map(&wp, &mp);
      xpos = 4*mp.x - pixmap_xpos;
      ypos = 4*(static_map.config.y_size - mp.y - 1) - pixmap_ypos;
      if (num_filters > 0 && filter_highlight >= 0 && laser_point_belongs_to_dot(&filters[filter_highlight], i))
	gdk_gc_set_foreground(drawing_gc, &carmen_blue);
      else if (laser_mask[i])
	gdk_gc_set_foreground(drawing_gc, &carmen_red);
      else
	gdk_gc_set_foreground(drawing_gc, &carmen_yellow);
      gdk_draw_arc(pixmap, drawing_gc, TRUE, xpos-2, ypos-2,
		   4, 4, 0, 360 * 64);      
    }
  }
}

static void draw_ellipse(double ux, double uy, double vx, double vxy, double vy, double k) {

#define ELLIPSE_PLOTPOINTS 30

  static GdkPoint poly[ELLIPSE_PLOTPOINTS];
  double len;
  gint i;
  double discriminant, eigval1, eigval2,
    eigvec1x, eigvec1y, eigvec2x, eigvec2y;
  //carmen_world_point_t point, e1, e2;
  carmen_graphics_screen_point_t p1;
  carmen_map_point_t mp;

  /* check for special case of axis-aligned */
  if (fabs(vxy) < (fabs(vx) + fabs(vy) + 1e-4) * 1e-4) {
    eigval1 = vx;
    eigval2 = vy;
    eigvec1x = 1.;
    eigvec1y = 0.;
    eigvec2x = 0.;
    eigvec2y = 1.;
  } else {

    /* compute axes and scales of ellipse */
    discriminant = sqrt(4*carmen_square(vxy) + 
			carmen_square(vx - vy));
    eigval1 = .5 * (vx + vy - discriminant);
    eigval2 = .5 * (vx + vy + discriminant);
    eigvec1x = (vx - vy - discriminant) / (2.*vxy);
    eigvec1y = 1.;
    eigvec2x = (vx - vy + discriminant) / (2.*vxy);
    eigvec2y = 1.;

    /* normalize eigenvectors */
    len = sqrt(carmen_square(eigvec1x) + 1.);
    eigvec1x /= len;
    eigvec1y /= len;
    len = sqrt(carmen_square(eigvec2x) + 1.);
    eigvec2x /= len;
    eigvec2y /= len;
  }

  /* take square root of eigenvalues and scale -- once this is
     done, eigvecs are unit vectors along axes and eigvals are
     corresponding radii */
  if (eigval1 < 0 || eigval2 < 0) {
    return;
  }

  eigval1 = sqrt(eigval1) * k;
  eigval2 = sqrt(eigval2) * k;
  if (eigval1 < .01) eigval1 = .01;
  if (eigval2 < .01) eigval2 = .01;

  /* compute points around edge of ellipse */
  for (i = 0; i < ELLIPSE_PLOTPOINTS; i++) {
    double theta = M_PI * (-1 + 2.*i/ELLIPSE_PLOTPOINTS);
    double xi = cos(theta) * eigval1;
    double yi = sin(theta) * eigval2;

    mp.x = (int)((xi * eigvec1x + yi * eigvec2x + ux)/(static_map.config.resolution/4.0));
    mp.y = (int)((xi * eigvec1y + yi * eigvec2y + uy)/(static_map.config.resolution/4.0));

    p1.x = mp.x - pixmap_xpos;
    p1.y = (4*static_map.config.y_size - mp.y - 1) - pixmap_ypos;

    poly[i].x = p1.x;
    poly[i].y = p1.y;
  }

  /* finally we can draw it */
  gdk_draw_polygon(pixmap, drawing_gc, FALSE,
                   poly, ELLIPSE_PLOTPOINTS);

  /*  
  e1 = *mean;
  e1.pose.x = mean->pose.x + eigval1 * eigvec1x;
  e1.pose.y = mean->pose.y + eigval1 * eigvec1y;

  e2 = *mean;
  e2.pose.x = mean->pose.x - eigval1 * eigvec1x;
  e2.pose.y = mean->pose.y - eigval1 * eigvec1y;

  carmen_map_graphics_draw_line(map_view, colour, &e1, &e2);

  e1.pose.x = mean->pose.x + eigval2 * eigvec2x;
  e1.pose.y = mean->pose.y + eigval2 * eigvec2y;
  e2.pose.x = mean->pose.x - eigval2 * eigvec2x;
  e2.pose.y = mean->pose.y - eigval2 * eigvec2y;

  carmen_map_graphics_draw_line(map_view, colour, &e1, &e2);
  */
}

static void draw_dots() {

  int i;
  double ux, uy, vx, vy, vxy, r;

  for (i = 0; i < num_filters; i++) {
    if (filters[i].type == CARMEN_DOT_PERSON) {
      gdk_gc_set_foreground(drawing_gc, &carmen_orange);
      ux = filters[i].person_filter.x;
      uy = filters[i].person_filter.y;
      r = exp(filters[i].person_filter.logr);
      vx = r*r;
      vy = r*r;
      vxy = 0.0;
      draw_ellipse(ux, uy, vx, vxy, vy, 1);
    }
    else if (filters[i].type == CARMEN_DOT_TRASH) {
      gdk_gc_set_foreground(drawing_gc, &carmen_green);
      ux = filters[i].trash_filter.x;
      uy = filters[i].trash_filter.y;
      vx = fabs(cos(filters[i].trash_filter.theta))*exp(filters[i].trash_filter.logw1);
      vy = fabs(sin(filters[i].trash_filter.theta))*exp(filters[i].trash_filter.logw1);
      vxy = (exp(filters[i].trash_filter.logw1) - vx) / tan(filters[i].trash_filter.theta);
      draw_ellipse(ux, uy, vx, vxy, vy, 1);
    }
  }
}

static void redraw() {

  if (pixmap == NULL)
    return;

  draw_map();

  gdk_gc_set_foreground(drawing_gc, &carmen_blue);
  gdk_draw_rectangle(pixmap, drawing_gc, TRUE, 0, 0, canvas_width, canvas_height);
  gdk_draw_pixmap(pixmap, drawing_gc,
		  map_pixmap, pixmap_xpos, pixmap_ypos, 0, 0, canvas_width, canvas_height);
  draw_robot();
  draw_laser();
  draw_dots();

  gdk_draw_pixmap(canvas->window,
		  canvas->style->fg_gc[GTK_WIDGET_STATE(canvas)],
		  pixmap, 0, 0, 0, 0, canvas_width, canvas_height);
}

static void canvas_button_press(GtkWidget *w __attribute__ ((unused)),
				GdkEventButton *event) {

  if (event->button == 1)
    filter_highlight = (num_filters > 0 ? (filter_highlight+1) % num_filters : -1);
  else
    filter_highlight = -1;

  redraw();
}

static void canvas_expose(GtkWidget *w __attribute__ ((unused)),
			  GdkEventExpose *event) {

  gdk_draw_pixmap(canvas->window,
		  canvas->style->fg_gc[GTK_WIDGET_STATE(canvas)],
		  pixmap, event->area.x, event->area.y,
		  event->area.x, event->area.y,
		  event->area.width, event->area.height);
}

static void canvas_configure(GtkWidget *w __attribute__ ((unused))) {

  int display = (drawing_gc != NULL);

  /*
  canvas_width = canvas->allocation.width;
  canvas_height = canvas->allocation.height;
  if (pixmap)
    gdk_pixmap_unref(pixmap);
  */

  pixmap = gdk_pixmap_new(canvas->window, canvas_width, canvas_height, -1);
  map_pixmap = gdk_pixmap_new(canvas->window, 4*static_map.config.x_size,
			      4*static_map.config.y_size, -1);

  /*
  gdk_gc_set_foreground(drawing_gc, &carmen_white);
  gdk_draw_rectangle(pixmap, canvas->style->white_gc, TRUE, 0, 0,
		     canvas_width, canvas_height);
  gdk_draw_pixmap(canvas->window,
		  canvas->style->fg_gc[GTK_WIDGET_STATE(canvas)],
		  pixmap, 0, 0, 0, 0, canvas_width, canvas_height);
  */

  if (display)
    redraw();
}

static void window_destroy(GtkWidget *w __attribute__ ((unused))) {

  gtk_main_quit();
}

static void gui_init() {

  GtkWidget *window, *vbox;

  window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_policy(GTK_WINDOW(window), FALSE, FALSE, FALSE);
  gtk_signal_connect(GTK_OBJECT(window), "destroy",
		     GTK_SIGNAL_FUNC(window_destroy), NULL);

  vbox = gtk_vbox_new(TRUE, 0);

  canvas = gtk_drawing_area_new();

  gtk_drawing_area_size(GTK_DRAWING_AREA(canvas), canvas_width,
			canvas_height);

  gtk_box_pack_start(GTK_BOX(vbox), canvas, TRUE, TRUE, 0);

  gtk_signal_connect(GTK_OBJECT(canvas), "expose_event",
		     GTK_SIGNAL_FUNC(canvas_expose), NULL);

  gtk_signal_connect(GTK_OBJECT(canvas), "configure_event",
		     GTK_SIGNAL_FUNC(canvas_configure), NULL);

  gtk_signal_connect(GTK_OBJECT(canvas), "button_press_event",
		     GTK_SIGNAL_FUNC(canvas_button_press), NULL);

  gtk_widget_add_events(canvas, GDK_BUTTON_PRESS_MASK);

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

#endif

/**************************************************/



//static void publish_dot_msg(carmen_dot_filter_p f, int delete);
static void publish_all_dot_msgs();

/**********************
static void print_filters() {

  int i;

  for (i = 0; i < num_filters; i++) {
    switch (filters[i].type) {
    case CARMEN_DOT_PERSON:
      printf("P");
      break;
    case CARMEN_DOT_TRASH:
	printf("T");
	break;
    case CARMEN_DOT_DOOR:
      printf("D");
      break;
    }
    printf("%d", filters[i].id);
    if (i < num_filters-1)
      printf(",");
  }
  printf("\n");
}
*************************/

static void reset() {

  free(filters);
  filters = NULL;
  num_filters = 0;
}

static inline double dist(double dx, double dy) {

  return sqrt(dx*dx+dy*dy);
}

static void rotate2d(double *x, double *y, double theta) {

  double x2, y2;

  x2 = *x;
  y2 = *y; 

  *x = x2*cos(theta) - y2*sin(theta);
  *y = x2*sin(theta) + y2*cos(theta);
}

/*
 * inverts 2d matrix [[a, b],[c, d]]
 */
static int invert2d(double *a, double *b, double *c, double *d) {

  double det;
  double a2, b2, c2, d2;

  det = (*a)*(*d) - (*b)*(*c);
  
  if (fabs(det) <= 0.00000001)
    return -1;

  a2 = (*d)/det;
  b2 = -(*b)/det;
  c2 = -(*c)/det;
  d2 = (*a)/det;

  *a = a2;
  *b = b2;
  *c = c2;
  *d = d2;

  return 0;
}

#if 0
static double matrix_det(gsl_matrix *M) {

  gsl_matrix *A;
  gsl_permutation *p;
  int signum;
  double d;

  A = gsl_matrix_alloc(M->size1, M->size2);
  carmen_test_alloc(A);
  gsl_matrix_memcpy(A, M);
  p = gsl_permutation_alloc(A->size1);
  carmen_test_alloc(p);
  gsl_linalg_LU_decomp(A, p, &signum);
  d = gsl_linalg_LU_det(A, signum);
  gsl_matrix_free(A);
  gsl_permutation_free(p);

  return d;
}
#endif


static void matrix_invert(gsl_matrix *M) {

  gsl_matrix *A;
  gsl_permutation *p;
  int signum;

  A = gsl_matrix_alloc(M->size1, M->size2);
  carmen_test_alloc(A);
  p = gsl_permutation_alloc(A->size1);
  carmen_test_alloc(p);
  gsl_linalg_LU_decomp(M, p, &signum);
  gsl_linalg_LU_invert(M, p, A);
  gsl_matrix_memcpy(M, A);
  gsl_matrix_free(A);
  gsl_permutation_free(p);
}

static gsl_matrix *matrix_mult(gsl_matrix *A, gsl_matrix *B) {

  gsl_matrix *C;
  unsigned int i, j, k;

  if (A->size2 != B->size1)
    carmen_die("Can't multiply %dx%d matrix A with %dx%d matrix B",
	       A->size1, A->size2, B->size1, B->size2);

  C = gsl_matrix_calloc(A->size1, B->size2);
  carmen_test_alloc(C);

  for (i = 0; i < A->size1; i++)
    for (j = 0; j < B->size2; j++)
      for (k = 0; k < A->size2; k++)
	C->data[MI(C,i,j)] += A->data[MI(A,i,k)]*B->data[MI(B,k,j)];
  
  return C;
}

/*
 * computes the orientation of the bivariate normal with variances
 * vx, vy, and covariance vxy.  returns an angle in [-pi/2, pi/2].
 */
static double bnorm_theta(double vx, double vy, double vxy) {

  double theta;
  double e;  // major eigenvalue
  double ex, ey;  // major eigenvector

  e = (vx + vy)/2.0 + sqrt(vxy*vxy + (vx-vy)*(vx-vy)/4.0);
  ex = vxy;
  ey = e - vx;

  theta = atan2(ey, ex);

  return theta;
}

/*
 * computes the major eigenvalue of the bivariate normal with variances
 * vx, vy, and covariance vxy.
 */
static inline double bnorm_w1(double vx, double vy, double vxy) {

  return (vx + vy)/2.0 + sqrt(vxy*vxy + (vx-vy)*(vx-vy)/4.0);
}

/*
 * computes the minor eigenvalue of the bivariate normal with variances
 * vx, vy, and covariance vxy.
 */
static inline double bnorm_w2(double vx, double vy, double vxy) {

  return (vx + vy)/2.0 - sqrt(vxy*vxy + (vx-vy)*(vx-vy)/4.0);
}

#if 0
static double bnorm_f(double x, double y, double ux, double uy,
		      double vx, double vy, double vxy) {
  
  double z, p;
  
  p = vxy/sqrt(vx*vy);
  z = (x - ux)*(x - ux)/vx - 2.0*p*(x - ux)*(y - uy)/vxy + (y - uy)*(y - uy)/vy;
  
  return exp(-z/(2.0*(1 - p*p)))/(2.0*M_PI*sqrt(vx*vy*(1 - p*p)));
}
#endif

//dbug: use errors P?
static int person_contains(carmen_dot_person_filter_p f, double x, double y, double stdevs) {
  
  double ux, uy, theta, r, vx, vy, vxy, e1, e2, er;


  ux = f->x;
  uy = f->y;
  r = exp(f->logr);
  vx = f->P->data[MI(f->P,0,0)];
  vy = f->P->data[MI(f->P,1,1)];
  vxy = f->P->data[MI(f->P,0,1)];
  e1 = bnorm_w1(vx, vy, vxy);
  e2 = bnorm_w2(vx, vy, vxy);
  theta = atan2(y-uy, x-ux);
  theta += bnorm_theta(vx, vy, vxy);
  er = e2*e2*e1*e1/(e2*e2*cos(theta)*cos(theta) + e1*e1*sin(theta)*sin(theta));

  return ((x-ux)*(x-ux) + (y-uy)*(y-uy) <= stdevs*stdevs*(r*r+er));
}

//dbug: use errors P?
static int trash_contains(carmen_dot_trash_filter_p f, double x, double y, double stdevs) {

  double ux, uy, theta, e1, e2, dx, dy;

  ux = f->x;
  uy = f->y;
  theta = f->theta;
  e1 = exp(f->logw1);
  e2 = exp(f->logw2);
  
  dx = x - ux;
  dy = y - uy;

  rotate2d(&dx, &dy, -theta);

  //return (e2*e2*dx*dx + e1*e1*dy*dy <= stdevs*stdevs*e1*e1*e2*e2);
  return (dx*dx + dy*dy <= stdevs*stdevs*e1*e1);
}

static int door_contains(carmen_dot_door_filter_p f, double x, double y, double stdevs) {

  double ux, uy, vx, vy, vxy, theta, e1, e2, dx, dy;

  ux = f->x;
  uy = f->y;
  vx = f->px;
  vy = f->py;
  vxy = f->pxy;
  theta = bnorm_theta(vx, vy, vxy);
  e1 = bnorm_w1(vx, vy, vxy);
  e2 = bnorm_w2(vx, vy, vxy);

  dx = x - ux;
  dy = y - uy;

  rotate2d(&dx, &dy, -theta);

  return (e2*e2*dx*dx + e1*e1*dy*dy <= stdevs*stdevs*e1*e1*e2*e2);
}

static int dot_contains(carmen_dot_filter_p f, double x, double y, double stdevs) {

  if (f->type == CARMEN_DOT_PERSON)
    return person_contains(&f->person_filter, x, y, stdevs);
  else if (f->type == CARMEN_DOT_TRASH)
    return trash_contains(&f->trash_filter, x, y, stdevs);
  else
    return door_contains(&f->door_filter, x, y, stdevs);

  return 0;
}

static void person_filter_motion_update(carmen_dot_person_filter_p f) {

  double ax, ay;
  
  // kalman filter time update with brownian motion model
  ax = carmen_uniform_random(-1.0, 1.0) * f->a;
  ay = carmen_uniform_random(-1.0, 1.0) * f->a;

  //printf("ax = %.4f, ay = %.4f\n", ax, ay);

  f->x += ax;
  f->y += ay;
  gsl_matrix_add(f->P, f->Q);

  //gsl_matrix_fprintf(stdout, f->P, "%f");
  //printf("\n\n");
  //gsl_matrix_fprintf(stdout, f->Q, "%f");
  //printf("\n");
}

static int ray_intersect_arg(double rx, double ry, double rtheta,
			     double x1, double y1, double x2, double y2) {

  static double epsilon = 0.01;  //dbug: param?

  if (fabs(rtheta) < M_PI/2.0 - epsilon) {
    if ((x1 < x2 || x2 < rx) && x1 >= rx)
      return 1;
    else if (x2 >= rx)
      return 2;
  }
  else if (fabs(rtheta) > M_PI/2.0 + epsilon) {
    if ((x1 > x2 || x2 > rx) && x1 <= rx)
      return 1;
    else if (x2 <= rx)
      return 2;    
  }
  else if (rtheta > 0.0) {
    if ((y1 < y2 || y2 < ry) && y1 >= ry)
      return 1;
    else if (y2 >= ry)
      return 2;
  }
  else {
    if ((y1 > y2 || y2 > ry) && y1 <= ry)
      return 1;
    else if (y2 <= ry)
      return 2;
  }

  return 0;
}

static int ray_intersect_circle(double rx, double ry, double rtheta,
				double cx, double cy, double cr,
				double *px, double *py) {

  double epsilon = 0.000001;
  double a, b, c, tan_rtheta;
  double x1, y1, x2, y2;

  if (fabs(cos(rtheta)) < epsilon) {
    if (cr*cr - (rx-cx)*(rx-cx) < 0.0)
      return 0;
    x1 = x2 = 0.0;
    y1 = cy + sqrt(cr*cr-(rx-cx)*(rx-cx));
    y2 = cy - sqrt(cr*cr-(rx-cx)*(rx-cx));
    switch (ray_intersect_arg(rx, ry, rtheta, x1, y1, x2, y2)) {
    case 0:
      return 0;
    case 2:
      y1 = y2;
    }
  }
  else {
    tan_rtheta = tan(rtheta);
    a = 1.0 + tan_rtheta*tan_rtheta;
    b = 2.0*tan_rtheta*(ry - rx*tan_rtheta - cy) - 2.0*cx;
    c = cx*cx + (ry - rx*tan_rtheta - cy)*(ry - rx*tan_rtheta - cy) - cr*cr;
    if (b*b - 4.0*a*c < 0.0)
      return 0;
    x1 = (-b + sqrt(b*b - 4.0*a*c))/(2.0*a);
    y1 = x1*tan_rtheta + ry - rx*tan_rtheta;
    x2 = (-b - sqrt(b*b - 4.0*a*c))/(2.0*a);
    y2 = x2*tan_rtheta + ry - rx*tan_rtheta;
    switch (ray_intersect_arg(rx, ry, rtheta, x1, y1, x2, y2)) {
    case 0:
      return 0;
    case 2:
      x1 = x2;
      y1 = y2;
    }
  }

  *px = x1;
  *py = y1;

  return 1;
}

/*
 * assumes expected sensor reading is on the ray
 */
static gsl_matrix *person_filter_measurement_jacobian(carmen_dot_person_filter_p f,
						      double lx, double ly, double ltheta) {

  gsl_matrix *H;
  double x0, y0, y1, y2, r;
  double cos_ltheta, sin_ltheta;
  double epsilon;
  double sign;

  sign = 1.0;
  epsilon = 0.00001;  //dbug: param?

  H = gsl_matrix_calloc(2,3);
  carmen_test_alloc(H);

  r = exp(f->logr);
  cos_ltheta = cos(ltheta);
  sin_ltheta = sin(ltheta);
  x0 = sin_ltheta*(f->x - lx) - cos_ltheta*(f->y - ly);
  y0 = cos_ltheta*(f->x - lx) + sin_ltheta*(f->y - ly);

  if (r*r - x0*x0 < 0) {  // [L^C| = 0
    H->data[MI(H,0,0)] = 1.0;
    H->data[MI(H,0,1)] = 0.0;
    H->data[MI(H,1,0)] = 0.0;
    H->data[MI(H,1,1)] = 1.0;
    if (x0 > 0.0) {
      H->data[MI(H,0,2)] = -sin_ltheta*r;
      H->data[MI(H,1,2)] = cos_ltheta*r;
    }
    else {
      H->data[MI(H,0,2)] = sin_ltheta*r;
      H->data[MI(H,1,2)] = -cos_ltheta*r;
    }
  }
  else if (r*r - x0*x0 < epsilon) {  // |L^C| = 1
    H->data[MI(H,0,0)] = cos_ltheta*cos_ltheta;
    H->data[MI(H,0,1)] = sin_ltheta*cos_ltheta;
    H->data[MI(H,0,2)] = 0.0;
    H->data[MI(H,1,0)] = sin_ltheta*cos_ltheta;
    H->data[MI(H,1,1)] = sin_ltheta*sin_ltheta;
    H->data[MI(H,1,2)] = 0.0;
  }
  else {  // |L^C| = 2
    y1 = y0 + sqrt(r*r - x0*x0);
    y2 = y0 - sqrt(r*r - x0*x0);
    switch (ray_intersect_arg(0.0, 0.0, M_PI/2.0, 0.0, y1, 0.0, y2)) {
    case 0:
      //carmen_warn("expected sensor reading isn't on ray! (0, 0, pi/2, 0, %f, 0, %f)", y1, y2);
      return NULL;
    case 1:
      sign = 1.0;
      break;
    case 2:
      sign = -1.0;
    }
    H->data[MI(H,0,0)] = cos_ltheta*cos_ltheta - sign*sin_ltheta*cos_ltheta*x0/sqrt(r*r-x0*x0);
    H->data[MI(H,0,1)] = sin_ltheta*cos_ltheta + sign*cos_ltheta*cos_ltheta*x0/sqrt(r*r-x0*x0);
    H->data[MI(H,0,2)] = sign*cos_ltheta*r*r/sqrt(r*r-x0*x0);
    H->data[MI(H,1,0)] = sin_ltheta*cos_ltheta - sign*sin_ltheta*sin_ltheta*x0/sqrt(r*r-x0*x0);
    H->data[MI(H,1,1)] = sin_ltheta*sin_ltheta + sign*sin_ltheta*cos_ltheta*x0/sqrt(r*r-x0*x0);
    H->data[MI(H,1,2)] = sign*sin_ltheta*r*r/sqrt(r*r-x0*x0);
  }

  return H;
}

static int person_filter_max_likelihood_measurement(carmen_dot_person_filter_p f, double lx, double ly,
						     double ltheta, double *hx, double *hy) {

  double x0, y0, x1, y1, y2, r;
  double cos_ltheta, sin_ltheta;
  double epsilon;

  epsilon = 0.00001;  //dbug: param?

  r = exp(f->logr);
  cos_ltheta = cos(ltheta);
  sin_ltheta = sin(ltheta);
  x0 = sin_ltheta*(f->x - lx) - cos_ltheta*(f->y - ly);
  y0 = cos_ltheta*(f->x - lx) + sin_ltheta*(f->y - ly);

  x1 = y1 = 0.0;

  if (r*r - x0*x0 < 0) {  // [L^C| = 0
    if (x0 > 0.0)
      x1 = x0 - r;
    else
      x1 = x0 + r;
    y1 = y0;
  }
  else if (r*r - x0*x0 < epsilon) {  // |L^C| = 1
    x1 = 0.0;
    y1 = y0;
  }
  else {  // |L^C| = 2
    x1 = 0.0;
    y1 = y0 + sqrt(r*r - x0*x0);
    y2 = y0 - sqrt(r*r - x0*x0);
    switch (ray_intersect_arg(0.0, 0.0, M_PI/2.0, 0.0, y1, 0.0, y2)) {
    case 0:
      carmen_warn("expected sensor reading isn't on ray! (0, 0, pi/2, 0, %f, 0, %f)", y1, y2);
      return -1;
    case 2:
      y1 = y2;
    }
  }   

  *hx = cos_ltheta*y1 + sin_ltheta*x1 + lx;
  *hy = sin_ltheta*y1 - cos_ltheta*x1 + ly;

  return 0;
}

static void person_filter_sensor_update(carmen_dot_person_filter_p f,
					double lx, double ly, double ltheta,
					float *range) {

  gsl_matrix *K, *M1, *M2, *H, *HT, *R2, *P2;
  double x, y, theta, hx, hy; //, d, dmin;
  int i, s, n; //, j, imin;

  //printf("update_person_filter()\n");

  P2 = gsl_matrix_calloc(3,3);
  carmen_test_alloc(P2);
  
  // (x,y) update
  if (f->sensor_update_list->length > 0 /*|| f->unsensor_update_list->length > 0*/) {
    H = gsl_matrix_alloc(2*f->sensor_update_list->length /*+ 2*f->unsensor_update_list->length*/, 3);
    carmen_test_alloc(H);
    n = 0;
    printf("( ");
    for (i = 0; i < f->sensor_update_list->length; i++) {
      s = *(int*)carmen_list_get(f->sensor_update_list, i);
      printf("%d ", s);
      theta = carmen_normalize_theta(ltheta + (s-90)*M_PI/180.0);
      M1 = person_filter_measurement_jacobian(f, lx, ly, theta);
      if (M1 == NULL)
	continue;
      H->data[MI(H,2*n,0)] = M1->data[MI(M1,0,0)];
      H->data[MI(H,2*n,1)] = M1->data[MI(M1,0,1)];
      H->data[MI(H,2*n,2)] = M1->data[MI(M1,0,2)];
      H->data[MI(H,2*n+1,0)] = M1->data[MI(M1,1,0)];
      H->data[MI(H,2*n+1,1)] = M1->data[MI(M1,1,1)];
      H->data[MI(H,2*n+1,2)] = M1->data[MI(M1,1,2)];
      n++;
      gsl_matrix_free(M1);
    }
    printf(")");
    /*
    printf("( ");
    for (i = 0; i < f->unsensor_update_list->length; i++) {
      s = *(int*)carmen_list_get(f->unsensor_update_list, i);
      printf("%d ", s);
      theta = carmen_normalize_theta(ltheta + (s-90)*M_PI/180.0);
      M1 = person_filter_measurement_jacobian(f, lx, ly, theta);
      if (M1 == NULL)
	continue;
      H->data[MI(H,2*n,0)] = M1->data[MI(M1,0,0)];
      H->data[MI(H,2*n,1)] = M1->data[MI(M1,0,1)];
      H->data[MI(H,2*n,2)] = M1->data[MI(M1,0,2)];
      H->data[MI(H,2*n+1,0)] = M1->data[MI(M1,1,0)];
      H->data[MI(H,2*n+1,1)] = M1->data[MI(M1,1,1)];
      H->data[MI(H,2*n+1,2)] = M1->data[MI(M1,1,2)];
      n++;
      gsl_matrix_free(M1);
    }
    printf(")");
    */
    if (n < f->sensor_update_list->length /*+ f->unsensor_update_list->length*/)
      carmen_die("*** n < f->sensor_update_list->length + f->unsensor_update_list->length (Jacobian) ***");
    HT = gsl_matrix_alloc(H->size2, H->size1);
    carmen_test_alloc(HT);
    gsl_matrix_transpose_memcpy(HT, H);
    
    P2->data[MI(P2,0,0)] = f->P->data[MI(f->P,0,0)];
    P2->data[MI(P2,0,1)] = f->P->data[MI(f->P,0,1)];
    P2->data[MI(P2,1,0)] = f->P->data[MI(f->P,1,0)];
    P2->data[MI(P2,1,1)] = f->P->data[MI(f->P,1,1)];
    //dbug
    P2->data[MI(P2,0,2)] = f->P->data[MI(f->P,0,2)];
    P2->data[MI(P2,1,2)] = f->P->data[MI(f->P,1,2)];
    P2->data[MI(P2,2,0)] = f->P->data[MI(f->P,2,0)];
    P2->data[MI(P2,2,1)] = f->P->data[MI(f->P,2,1)];
    P2->data[MI(P2,2,2)] = f->P->data[MI(f->P,2,2)];

    M1 = matrix_mult(H, P2);
    M2 = matrix_mult(M1, HT);
    gsl_matrix_free(M1);
    R2 = gsl_matrix_calloc(2*f->sensor_update_list->length /*+ 2*f->unsensor_update_list->length*/,
			   2*f->sensor_update_list->length /*+ 2*f->unsensor_update_list->length*/);
    carmen_test_alloc(R2);
    for (i = 0; i < f->sensor_update_list->length /*+ f->unsensor_update_list->length*/; i++) {
      R2->data[MI(R2,2*i,2*i)] = f->R->data[MI(f->R,0,0)];
      R2->data[MI(R2,2*i,2*i+1)] = f->R->data[MI(f->R,0,1)];
      R2->data[MI(R2,2*i+1,2*i)] = f->R->data[MI(f->R,1,0)];
      R2->data[MI(R2,2*i+1,2*i+1)] = f->R->data[MI(f->R,1,1)];
    }
    gsl_matrix_add(M2, R2);
    matrix_invert(M2);
    M1 = matrix_mult(HT, M2);
    gsl_matrix_free(M2);
    K = matrix_mult(P2, M1);
    gsl_matrix_free(M1);
    
    M1 = gsl_matrix_calloc(2*f->sensor_update_list->length /*+ 2*f->unsensor_update_list->length*/, 1);
    carmen_test_alloc(M1);
    n = 0;
    for (i = 0; i < f->sensor_update_list->length; i++) {
      s = *(int*)carmen_list_get(f->sensor_update_list, i);
      theta = carmen_normalize_theta(ltheta + (s-90)*M_PI/180.0);
      x = lx + cos(theta)*range[s];
      y = ly + sin(theta)*range[s];
      if (person_filter_max_likelihood_measurement(f, lx, ly, theta, &hx, &hy) < 0)
	continue;
      M1->data[MI(M1,2*n,0)] = x - hx;
      M1->data[MI(M1,2*n+1,0)] = y - hy;
      n++;
    }
    /*
    for (i = 0; i < f->unsensor_update_list->length; i++) {
      s = *(int*)carmen_list_get(f->unsensor_update_list, i);
      theta = carmen_normalize_theta(ltheta + (s-90)*M_PI/180.0);
      if (person_filter_max_likelihood_measurement(f, lx, ly, theta, &hx, &hy) < 0)
	continue;
      imin = -1;
      dmin = -1.0;
      for (j = 0; j < f->sensor_update_list->length; j++) {
	s = *(int*)carmen_list_get(f->unsensor_update_list, j);
	theta = carmen_normalize_theta(ltheta + (s-90)*M_PI/180.0);
	x = lx + cos(theta)*range[s];
	y = ly + sin(theta)*range[s];
	d = dist(x-hx, y-hy);
	if (d > dmin) {
	  dmin = d;
	  imin = s;
	}
      }
      theta = carmen_normalize_theta(ltheta + (imin-90)*M_PI/180.0);
      x = lx + cos(theta)*range[imin];
      y = ly + sin(theta)*range[imin];
      M1->data[MI(M1,2*n,0)] = x - hx;
      M1->data[MI(M1,2*n+1,0)] = y - hy;
      n++;
    }
    */
    if (n < f->sensor_update_list->length /*+ f->unsensor_update_list->length*/)
      carmen_die("*** n < f->sensor_update_list->length + f->unsensor_update_list->length (h) ***");
    
    M2 = matrix_mult(K, M1);
    gsl_matrix_free(M1);
    
    f->x += M2->data[MI(M2,0,0)];
    f->y += M2->data[MI(M2,1,0)];
    f->logr += M2->data[MI(M2,2,0)];
    if (exp(f->logr) < 0.2)
      f->logr = log(0.2);
    else if (exp(f->logr) > 0.3)
      f->logr = log(0.3);
    
    gsl_matrix_free(M2);
    M1 = matrix_mult(K, H);
    M2 = gsl_matrix_alloc(3,3);  //dbug
    carmen_test_alloc(M2);
    gsl_matrix_set_identity(M2);
    gsl_matrix_sub(M2, M1);
    gsl_matrix_free(M1);
    M1 = matrix_mult(M2, f->P);
    gsl_matrix_free(M2);
    gsl_matrix_free(f->P);
    f->P = M1;

    gsl_matrix_free(K);
    gsl_matrix_free(R2);
    gsl_matrix_free(H);
    gsl_matrix_free(HT);
  }


  // logr update
  if (0) { //f->unsensor_update_list->length > 0) {
    H = gsl_matrix_alloc(2*f->unsensor_update_list->length, 3);
    carmen_test_alloc(H);
    n = 0;
    printf("( ");
    for (i = 0; i < f->unsensor_update_list->length; i++) {
      s = *(int*)carmen_list_get(f->unsensor_update_list, i);
      printf("%d ", s);
      theta = carmen_normalize_theta(ltheta + (s-90)*M_PI/180.0);
      M1 = person_filter_measurement_jacobian(f, lx, ly, theta);
      if (M1 == NULL)
	continue;
      H->data[MI(H,2*n,0)] = M1->data[MI(M1,0,0)];
      H->data[MI(H,2*n,1)] = M1->data[MI(M1,0,1)];
      H->data[MI(H,2*n,2)] = M1->data[MI(M1,0,2)];
      H->data[MI(H,2*n+1,0)] = M1->data[MI(M1,1,0)];
      H->data[MI(H,2*n+1,1)] = M1->data[MI(M1,1,1)];
      H->data[MI(H,2*n+1,2)] = M1->data[MI(M1,1,2)];
      n++;
      gsl_matrix_free(M1);
    }
    printf(")");
    //H->size1 = 2*n;
    if (n < f->unsensor_update_list->length)
      carmen_die("*** n < f->unsensor_update_list->length (Jacobian) ***");
    HT = gsl_matrix_alloc(H->size2, H->size1);
    carmen_test_alloc(HT);
    gsl_matrix_transpose_memcpy(HT, H);
    
    P2->data[MI(P2,0,0)] = f->P->data[MI(f->P,0,0)];
    P2->data[MI(P2,0,1)] = f->P->data[MI(f->P,0,1)];
    P2->data[MI(P2,1,0)] = f->P->data[MI(f->P,1,0)];
    P2->data[MI(P2,1,1)] = f->P->data[MI(f->P,1,1)];
    P2->data[MI(P2,0,2)] = f->P->data[MI(f->P,0,2)];
    P2->data[MI(P2,1,2)] = f->P->data[MI(f->P,1,2)];
    P2->data[MI(P2,2,0)] = f->P->data[MI(f->P,2,0)];
    P2->data[MI(P2,2,1)] = f->P->data[MI(f->P,2,1)];
    P2->data[MI(P2,2,2)] = f->P->data[MI(f->P,2,2)];

    M1 = matrix_mult(H, P2);
    M2 = matrix_mult(M1, HT);
    gsl_matrix_free(M1);
    R2 = gsl_matrix_calloc(2*f->unsensor_update_list->length, 2*f->unsensor_update_list->length);
    carmen_test_alloc(R2);
    for (i = 0; i < f->unsensor_update_list->length; i++) {
      R2->data[MI(R2,2*i,2*i)] = f->R->data[MI(f->R,0,0)];
      R2->data[MI(R2,2*i,2*i+1)] = f->R->data[MI(f->R,0,1)];
      R2->data[MI(R2,2*i+1,2*i)] = f->R->data[MI(f->R,1,0)];
      R2->data[MI(R2,2*i+1,2*i+1)] = f->R->data[MI(f->R,1,1)];
    }
    gsl_matrix_add(M2, R2);
    matrix_invert(M2);
    M1 = matrix_mult(HT, M2);
    gsl_matrix_free(M2);
    K = matrix_mult(P2, M1);
    gsl_matrix_free(M1);
    
    M1 = gsl_matrix_calloc(2*f->unsensor_update_list->length, 1);
    carmen_test_alloc(M1);
    n = 0;
    for (i = 0; i < f->unsensor_update_list->length; i++) {
      s = *(int*)carmen_list_get(f->unsensor_update_list, i);
      theta = carmen_normalize_theta(ltheta + (s-90)*M_PI/180.0);
      x = lx + cos(theta)*range[s];
      y = ly + sin(theta)*range[s];
      if (person_filter_max_likelihood_measurement(f, lx, ly, theta, &hx, &hy) < 0)
	continue;
      M1->data[MI(M1,2*n,0)] = x - hx;
      M1->data[MI(M1,2*n+1,0)] = y - hy;
      
      n++;
    }
    //M1->size1 = 2*n;
    if (n < f->unsensor_update_list->length)
      carmen_die("*** n < f->unsensor_update_list->length (h) ***");
    
    M2 = matrix_mult(K, M1);
    gsl_matrix_free(M1);
    
    //f->x += M2->data[MI(M2,0,0)];
    //f->y += M2->data[MI(M2,1,0)];
    f->logr += M2->data[MI(M2,2,0)];
    if (exp(f->logr) < 0.2)
      f->logr = log(0.2);
    else if (exp(f->logr) > 0.4)
      f->logr = log(0.4);
    
    gsl_matrix_free(M2);
    M1 = matrix_mult(K, H);
    M2 = gsl_matrix_alloc(3,3);  //dbug
    carmen_test_alloc(M2);
    gsl_matrix_set_identity(M2);
    gsl_matrix_sub(M2, M1);
    gsl_matrix_free(M1);
    M1 = matrix_mult(M2, f->P);
    gsl_matrix_free(M2);
    gsl_matrix_free(f->P);
    f->P = M1;
  }

  gsl_matrix_free(P2);
}
 
#if 0
static void person_filter_sensor_update_r(carmen_dot_person_filter_p f,
					  double lx, double ly, double ltheta,
					  double x, double y) {

  gsl_matrix *K, *M1, *M2, *H, *HT;
  double hx, hy;
 
  H = person_filter_measurement_jacobian(f, lx, ly, ltheta);
  if (H == NULL)
    return;
  //H->data[MI(H,0,0)] = 0.0;
  //H->data[MI(H,0,1)] = 0.0;
  //H->data[MI(H,1,0)] = 0.0;
  //H->data[MI(H,1,1)] = 0.0;
  HT = gsl_matrix_alloc(H->size2, H->size1);
  carmen_test_alloc(HT);
  gsl_matrix_transpose_memcpy(HT, H);

  M1 = matrix_mult(H, f->P);
  M2 = matrix_mult(M1, HT);
  gsl_matrix_free(M1);
  gsl_matrix_add(M2, f->R);
  matrix_invert(M2);
  M1 = matrix_mult(HT, M2);
  gsl_matrix_free(M2);
  K = matrix_mult(f->P, M1);
  gsl_matrix_free(M1);

  if (person_filter_max_likelihood_measurement(f, lx, ly, ltheta, &hx, &hy) < 0)
    return;

  M1 = gsl_matrix_calloc(2,1);
  carmen_test_alloc(M1);
  M1->data[MI(M1,0,0)] = x - hx;
  M1->data[MI(M1,1,0)] = y - hy;
  M2 = matrix_mult(K, M1);
  gsl_matrix_free(M1);

  //f->x += M2->data[MI(M2,0,0)];
  //f->y += M2->data[MI(M2,1,0)];
  f->logr += M2->data[MI(M2,2,0)];
  if (exp(f->logr) < 0.2)
    f->logr = log(0.2);
  else if (exp(f->logr) > 0.4)
    f->logr = log(0.4);

  gsl_matrix_free(M2);
  M1 = matrix_mult(K, H);
  M2 = gsl_matrix_alloc(3,3);  //dbug
  carmen_test_alloc(M2);
  gsl_matrix_set_identity(M2);
  gsl_matrix_sub(M2, M1);
  gsl_matrix_free(M1);
  M1 = matrix_mult(M2, f->P);
  gsl_matrix_free(M2);
  gsl_matrix_free(f->P);
  f->P = M1;

  //printf("\nMeasure");
  //gsl_matrix_fprintf(stdout, f->P, "%f");
  //printf("\n\n");
}
#endif

static void trash_filter_motion_update(carmen_dot_trash_filter_p f) {

  double ax, ay;
  
  // kalman filter time update with brownian motion model
  ax = carmen_uniform_random(-1.0, 1.0) * f->a;
  ay = carmen_uniform_random(-1.0, 1.0) * f->a;

  //printf("ax = %.4f, ay = %.4f\n", ax, ay);

  f->x += ax;
  f->y += ay;
  gsl_matrix_add(f->P, f->Q);
}

static gsl_matrix *trash_filter_measurement_jacobian(carmen_dot_trash_filter_p f,
						     double lx, double ly, double ltheta) {

  gsl_matrix *H;
  double w1, w2;
  double x1, y1, x2, y2;
  double a, b, c;
  double xlp, ylp, tlp;  // x_l', y_l', theta_l'
  double xp, yp, k, xpp, ypp;
  double sint, cost, sin2t, cos2t;
  double sintlp, costlp, tantlp, sectlp;
  double dxlpdx, dxlpdy, dxlpdt;
  double dylpdx, dylpdy, dylpdt;
  double dadx, dady, dadt, dadlnw1, dadlnw2;
  double dbdx, dbdy, dbdt, dbdlnw1, dbdlnw2;
  double dcdx, dcdy, dcdt, dcdlnw1, dcdlnw2;
  double dxpdx, dxpdy, dxpdt, dxpdlnw1, dxpdlnw2;
  double dypdx, dypdy, dypdt, dypdlnw1, dypdlnw2;
  double dkdx, dkdy, dkdt, dkdlnw1, dkdlnw2;
  double dxppdx, dxppdy, dxppdt, dxppdlnw1, dxppdlnw2;
  double dyppdx, dyppdy, dyppdt, dyppdlnw1, dyppdlnw2;
  double sign = 0.0;
  double epsilon = 0.000001;

  //printf("trash_filter_measurement_jacobian(%f,%f,%f)\n", lx, ly, ltheta);

  xp = yp = 0.0;

  H = gsl_matrix_calloc(2,5);
  carmen_test_alloc(H);

  w1 = exp(f->logw1);
  w2 = exp(f->logw2);

  sint = sin(f->theta);
  cost = cos(f->theta);
  sin2t = sin(2.0*f->theta);
  cos2t = cos(2.0*f->theta);

  xlp = cost*(lx - f->x) + sint*(ly - f->y);
  ylp = -sint*(lx - f->x) + cost*(ly - f->y);
  tlp = ltheta + f->theta;

  //printf("\n***(f->x = %f, f->y = %f, f->theta = %f)***", f->x, f->y, f->theta);
  //printf("\n***(xlp = %f, ylp = %f, tlp = %f)***\n", xlp, ylp, tlp);

  sintlp = sin(tlp);
  costlp = cos(tlp);
  tantlp = tan(tlp);
  sectlp = 1.0/costlp;
  
  dxlpdx = -cost;
  dxlpdy = -sint;
  dxlpdt = -sint*(lx-f->x) + cost*(ly-f->y);
  dylpdx = sint;
  dylpdy = -cost;
  dylpdt = -cost*(lx-f->x) - sint*(ly-f->y);

  if (fabs(fabs(tlp) - M_PI/2.0) <= epsilon) {  // theta_l' is approx. (+/-)pi/2
    if ((1.0-w2*w2*xlp*xlp)/(w1*w1) < epsilon) {  // |L^E| = 0 or 1
      sign = (xlp >= 0 ? 1.0 : -1.0);
      H->data[MI(H,0,0)] = 1.0;
      H->data[MI(H,0,1)] = 0.0;
      H->data[MI(H,0,2)] = -sign*w1*sint;
      H->data[MI(H,0,3)] = sign*w1*cost;
      H->data[MI(H,0,4)] = 0.0;
      H->data[MI(H,1,0)] = 0.0;
      H->data[MI(H,1,1)] = 1.0;
      H->data[MI(H,1,2)] = sign*w1*cost;
      H->data[MI(H,1,3)] = sign*w1*sint;
      H->data[MI(H,1,4)] = 0.0;
    }
    else {  // |L^E| = 2
      x1 = cost*xlp - sint*sqrt((1.0-w2*w2*xlp*xlp)/(w1*w1)) + f->x;
      x2 = cost*xlp + sint*sqrt((1.0-w2*w2*xlp*xlp)/(w1*w1)) + f->x;
      y1 = sint*xlp + cost*sqrt((1.0-w2*w2*xlp*xlp)/(w1*w1)) + f->y;
      y2 = sint*xlp - cost*sqrt((1.0-w2*w2*xlp*xlp)/(w1*w1)) + f->y;
      switch (ray_intersect_arg(lx, ly, ltheta, x1, y1, x2, y2)) {
      case 0:
	carmen_warn("expected sensor reading isn't on ray! (break 1) (%f, %f, %f, %f, %f, %f, %f)",
		    lx, ly, ltheta, x1, y1, x2, y2);
	return NULL;
      case 1:
	sign = 1.0;
	break;
      case 2:
	sign = -1.0;
      }
      H->data[MI(H,0,0)] = sint*sint - sign*sint*cost*(w2*w2/(w1*w1))*xlp*sqrt(w1*w1/(1.0-w2*w2*xlp*xlp));
      H->data[MI(H,0,1)] = -sint*cost - sign*sint*sint*(w2*w2/(w1*w1))*xlp*sqrt(w1*w1/(1.0-w2*w2*xlp*xlp));
      H->data[MI(H,0,2)] = (cos2t*(ly-f->y) - sin2t*(lx - f->x) - sign*cost*sqrt((1.0-w2*w2*xlp*xlp)/(w1*w1))
			    + sign*sint*(w2*w2/(w1*w1))*xlp*sqrt(w1*w1/(1.0-w2*w2*xlp*xlp))*dxlpdt);
      H->data[MI(H,0,3)] = sign*sint*sqrt((1.0-w2*w2*xlp*xlp)/(w1*w1));
      H->data[MI(H,0,4)] = sign*sint*w2*w2*xlp*xlp/(w1*sqrt(1.0-w2*w2*xlp*xlp));
      H->data[MI(H,1,0)] = -sint*cost + sign*cost*cost*(w2*w2/(w1*w1))*xlp*sqrt(w1*w1/(1.0-w2*w2*xlp*xlp));
      H->data[MI(H,1,1)] = cost*cost + sign*sint*cost*(w2*w2/(w1*w1))*xlp*sqrt(w1*w1/(1.0-w2*w2*xlp*xlp));
      H->data[MI(H,1,2)] = (cos2t*(lx-f->x) + sin2t*(ly - f->y) - sign*sint*sqrt((1.0-w2*w2*xlp*xlp)/(w1*w1))
			    + sign*cost*(w2*w2/(w1*w1))*xlp*sqrt(w1*w1/(1.0-w2*w2*xlp*xlp))*dxlpdt);
      H->data[MI(H,1,3)] = -sign*cost*sqrt((1.0-w2*w2*xlp*xlp)/(w1*w1));
      H->data[MI(H,1,4)] = -sign*cost*w2*w2*xlp*xlp/(w1*sqrt(1.0-w2*w2*xlp*xlp));
    }
  }
  else {  // theta_l' != (+/-)pi/2
    a = w2*w2 + w1*w1*tantlp*tantlp;
    b = 2.0*w1*w1*tantlp*(ylp-xlp*tantlp);
    c = w1*w1*(ylp-xlp*tantlp)*(ylp-xlp*tantlp) - w1*w1*w2*w2;
    
    dadx = 0.0;
    dady = 0.0;
    dadt = 2.0*w1*w1*tantlp*sectlp*sectlp;
    dadlnw1 = 2.0*w1*w1*tantlp*tantlp;
    dadlnw2 = 2.0*w2*w2;
    dbdx = 2.0*w1*w1*tantlp*(dylpdx - dxlpdx*tantlp);
    dbdy = 2.0*w1*w1*tantlp*(dylpdy - dxlpdy*tantlp);
    dbdt = 2.0*w1*w1*(sectlp*sectlp*ylp + tantlp*dylpdt - 2.0*tantlp*sectlp*sectlp*xlp - tantlp*tantlp*dxlpdt);
    dbdlnw1 = 2.0*b;
    dbdlnw2 = 0.0;
    dcdx = 2.0*w1*w1*(ylp - xlp*tantlp)*(dylpdx - dxlpdx*tantlp);
    dcdy = 2.0*w1*w1*(ylp - xlp*tantlp)*(dylpdy - dxlpdy*tantlp);
    dcdt = 2.0*w1*w1*(ylp - xlp*tantlp)*(dylpdt - dxlpdt*tantlp - xlp*sectlp*sectlp);
    dcdlnw1 = 2.0*c;
    dcdlnw2 = -2.0*w1*w1*w2*w2;
    
    if (b*b - 4.0*a*c < 0.0) {  // |L^E| = 0
      xp = -b/(2.0*a);
      yp = xp*tantlp + ylp - xlp*tantlp;
      k = (ylp - xlp*tantlp)/sqrt(w2*w2 + w1*w1*tantlp*tantlp);

      xpp = xp/k;
      ypp = yp/k;

      dxpdx = (b*dadx - a*dbdx)/(2.0*a*a);
      dxpdy = (b*dady - a*dbdy)/(2.0*a*a);
      dxpdt = (b*dadt - a*dbdt)/(2.0*a*a);
      dxpdlnw1 = (b*dadlnw1 - a*dbdlnw1)/(2.0*a*a);
      dxpdlnw2 = (b*dadlnw2 - a*dbdlnw2)/(2.0*a*a);
      dypdx = dxpdx*tantlp + dylpdx - dxlpdx*tantlp;
      dypdy = dxpdy*tantlp + dylpdy - dxlpdy*tantlp;
      dypdt = dxpdt*tantlp + xp*sectlp*sectlp + dylpdt - dxlpdt*tantlp - xlp*sectlp*sectlp;
      dypdlnw1 = dxpdlnw1*tantlp;
      dypdlnw2 = dxpdlnw2*tantlp;

      dkdx = (dylpdx - dxlpdx*tantlp)/sqrt(w2*w2+w1*w1*tantlp*tantlp);
      dkdy = (dylpdy - dxlpdy*tantlp)/sqrt(w2*w2+w1*w1*tantlp*tantlp);
      dkdt = (sqrt(w2*w2+w1*w1*tantlp*tantlp)*(dylpdt - xlp*sectlp*sectlp - dxlpdt*tantlp) -
	      (w1*w1*tantlp*sectlp*sectlp/sqrt(w2*w2+w1*w1*tantlp*tantlp))*(ylp - xlp*tantlp)) / (w2*w2+w1*w1*tantlp*tantlp);
      dkdlnw1 = -(ylp - xlp*tantlp)*w1*w1*tantlp*tantlp/sqrt((w2*w2+w1*w1*tantlp*tantlp)*(w2*w2+w1*w1*tantlp*tantlp)*(w2*w2+w1*w1*tantlp*tantlp));
      dkdlnw2 = -(ylp - xlp*tantlp)*w2*w2/sqrt((w2*w2+w1*w1*tantlp*tantlp)*(w2*w2+w1*w1*tantlp*tantlp)*(w2*w2+w1*w1*tantlp*tantlp));
      
      dxppdx = (k*dxpdx - xp*dkdx)/(k*k);
      dxppdy = (k*dxpdy - xp*dkdy)/(k*k);
      dxppdt = (k*dxpdt - xp*dkdt)/(k*k);
      dxppdlnw1 = (k*dxpdlnw1 - xp*dkdlnw1)/(k*k);
      dxppdlnw2 = (k*dxpdlnw2 - xp*dkdlnw2)/(k*k);
      dyppdx = (k*dypdx - yp*dkdx)/(k*k);
      dyppdy = (k*dypdy - yp*dkdy)/(k*k);
      dyppdt = (k*dypdt - yp*dkdt)/(k*k);
      dyppdlnw1 = (k*dypdlnw1 - yp*dkdlnw1)/(k*k);
      dyppdlnw2 = (k*dypdlnw2 - yp*dkdlnw2)/(k*k);

      xp = xpp;
      yp = ypp;

      dxpdx = dxppdx;
      dxpdy = dxppdy;
      dxpdt = dxppdt;
      dxpdlnw1 = dxppdlnw1;
      dxpdlnw2 = dxppdlnw2;
      dypdx = dyppdx;
      dypdy = dyppdy;
      dypdt = dyppdt;
      dypdlnw1 = dyppdlnw1;
      dypdlnw2 = dyppdlnw2;
    }
    else if (b*b - 4.0*a*c < epsilon) {  // |L^E| = 1
      xp = -b/(2.0*a);
      yp = xp*tantlp + ylp - xlp*tantlp;

      dxpdx = (b*dadx - a*dbdx)/(2.0*a*a);
      dxpdy = (b*dady - a*dbdy)/(2.0*a*a);
      dxpdt = (b*dadt - a*dbdt)/(2.0*a*a);
      dxpdlnw1 = (b*dadlnw1 - a*dbdlnw1)/(2.0*a*a);
      dxpdlnw2 = (b*dadlnw2 - a*dbdlnw2)/(2.0*a*a);
      dypdx = dxpdx*tantlp + dylpdx - dxlpdx*tantlp;
      dypdy = dxpdy*tantlp + dylpdy - dxlpdy*tantlp;
      dypdt = dxpdt*tantlp + xp*sectlp*sectlp + dylpdt - dxlpdt*tantlp - xlp*sectlp*sectlp;
      dypdlnw1 = dxpdlnw1*tantlp;
      dypdlnw2 = dxpdlnw2*tantlp;
    }
    else {  // |L^E| = 2
      x1 = (-b + sqrt(b*b-4.0*a*c))/(2.0*a);
      x2 = (-b - sqrt(b*b-4.0*a*c))/(2.0*a);
      y1 = x1*tantlp + ylp - xlp*tantlp;
      y2 = x2*tantlp + ylp - xlp*tantlp;
      switch (ray_intersect_arg(xlp, ylp, tlp, x1, y1, x2, y2)) {
      case 0:
	carmen_warn("expected sensor reading isn't on ray! (break 2) (%f, %f, %f, %f, %f, %f, %f)",
		    xlp, ylp, tlp, x1, y1, x2, y2);
	return NULL;
      case 1:
	xp = x1;
	yp = y1;
	sign = 1.0;
	break;
      case 2:
	xp = x2;
	yp = x2;
	sign = -1.0;
      }

      dxpdx = (-a*dbdx + sign*a*(b*dbdx - 2.0*a*dcdx - 2.0*c*dadx)/sqrt(b*b-4.0*a*c) + (b - sign*sqrt(b*b-4.0*a*c))*dadx) / (2.0*a*a);
      dxpdy = (-a*dbdy + sign*a*(b*dbdy - 2.0*a*dcdy - 2.0*c*dady)/sqrt(b*b-4.0*a*c) + (b - sign*sqrt(b*b-4.0*a*c))*dady) / (2.0*a*a);
      dxpdt = (-a*dbdt + sign*a*(b*dbdt - 2.0*a*dcdt - 2.0*c*dadt)/sqrt(b*b-4.0*a*c) + (b - sign*sqrt(b*b-4.0*a*c))*dadt) / (2.0*a*a);
      dxpdlnw1 = (-a*dbdlnw1 + sign*a*(b*dbdlnw1 - 2.0*a*dcdlnw1 - 2.0*c*dadlnw1)/sqrt(b*b-4.0*a*c) + (b - sign*sqrt(b*b-4.0*a*c))*dadlnw1) / (2.0*a*a);
      dxpdlnw2 = (-a*dbdlnw2 + sign*a*(b*dbdlnw2 - 2.0*a*dcdlnw2 - 2.0*c*dadlnw2)/sqrt(b*b-4.0*a*c) + (b - sign*sqrt(b*b-4.0*a*c))*dadlnw2) / (2.0*a*a);
      dypdx = dxpdx*tantlp + dylpdx - dxlpdx*tantlp;
      dypdy = dxpdy*tantlp + dylpdy - dxlpdy*tantlp;
      dypdt = dxpdt*tantlp + xp*sectlp*sectlp + dylpdt - dxlpdt*tantlp - xlp*sectlp*sectlp;
      dypdlnw1 = dxpdlnw1*tantlp;
      dypdlnw2 = dxpdlnw2*tantlp;
    }

    H->data[MI(H,0,0)] = cost*dxpdx - sint*dypdx + 1.0;
    H->data[MI(H,0,1)] = cost*dxpdy - sint*dypdy;
    H->data[MI(H,0,2)] = -sint*xp + cost*dxpdt - cost*yp - sint*dypdt;
    H->data[MI(H,0,3)] = cost*dxpdlnw1 - sint*dypdlnw1;
    H->data[MI(H,0,4)] = cost*dxpdlnw2 - sint*dypdlnw2;
    H->data[MI(H,1,0)] = sint*dxpdx + cost*dypdx;
    H->data[MI(H,1,1)] = sint*dxpdy + cost*dypdy + 1.0;
    H->data[MI(H,1,2)] = cost*xp + sint*dxpdt - sint*yp + cost*dypdt;
    H->data[MI(H,1,3)] = sint*dxpdlnw1 + cost*dypdlnw1;
    H->data[MI(H,1,4)] = sint*dxpdlnw2 + cost*dypdlnw2;
  }

  return H;
}

static int trash_filter_max_likelihood_measurement(carmen_dot_trash_filter_p f, double lx, double ly,
						   double ltheta, double *hx, double *hy) {
  
  double w1, w2;
  double x1, y1, x2, y2;
  double a, b, c;
  double xlp, ylp, tlp;  // x_l', y_l', theta_l'
  double sint, cost;
  double sintlp, costlp, tantlp;
  double xp, yp, k;
  double epsilon = 0.000001;

  w1 = exp(f->logw1);
  w2 = exp(f->logw2);

  sint = sin(f->theta);
  cost = cos(f->theta);

  xlp = cost*(lx - f->x) + sint*(ly - f->y);
  ylp = -sint*(lx - f->x) + cost*(ly - f->y);
  tlp = ltheta + f->theta;

  sintlp = sin(tlp);
  costlp = cos(tlp);
  tantlp = tan(tlp);

  if (fabs(fabs(tlp) - M_PI/2.0) <= epsilon) {  // theta_l' is approx. (+/-)pi/2
    if ((1.0-w2*w2*xlp*xlp)/(w1*w1) < epsilon) {  // |L^E| = 0 or 1
      if (xlp >= 0) {
	*hx = w1*cost + f->x;
	*hy = w1*sint + f->y;
      }
      else {
	*hx = -w1*cost + f->x;
	*hy = -w1*sint + f->y;
      }
    }
    else {  // |L^E| = 2
      x1 = cost*xlp - sint*sqrt((1.0-w2*w2*xlp*xlp)/(w1*w1)) + f->x;
      x2 = cost*xlp + sint*sqrt((1.0-w2*w2*xlp*xlp)/(w1*w1)) + f->x;
      y1 = sint*xlp + cost*sqrt((1.0-w2*w2*xlp*xlp)/(w1*w1)) + f->y;
      y2 = sint*xlp - cost*sqrt((1.0-w2*w2*xlp*xlp)/(w1*w1)) + f->y;
      switch (ray_intersect_arg(lx, ly, ltheta, x1, y1, x2, y2)) {
      case 0:
	carmen_warn("expected sensor reading isn't on ray! (break 3) (%f, %f, %f, %f, %f, %f, %f)",
		    lx, ly, ltheta, x1, y1, x2, y2);
	return -1;
      case 1:
	*hx = x1;
	*hy = y1;
	break;
      case 2:
	*hx = x2;
	*hy = y2;
      }
    }
  }
  else {  // theta_l' != (+/-)pi/2
    a = w2*w2 + w1*w1*tantlp*tantlp;
    b = 2.0*w1*w1*tantlp*(ylp-xlp*tantlp);
    c = w1*w1*(ylp-xlp*tantlp)*(ylp-xlp*tantlp) - w1*w1*w2*w2;
    if (b*b - 4.0*a*c < 0.0) {  // |L^E| = 0
      xp = -b/(2.0*a);
      yp = xp*tantlp + ylp - xlp*tantlp;
      k = (ylp - xlp*tantlp)/sqrt(w2*w2 + w1*w1*tantlp*tantlp);
      *hx = cost*(xp/k) - sint*(yp/k) + f->x;
      *hy = sint*(xp/k) + cost*(yp/k) + f->y;
    }
    else if (b*b - 4.0*a*c < epsilon) {  // |L^E| = 1
      xp = -b/(2.0*a);
      yp = xp*tantlp + ylp - xlp*tantlp;
      *hx = cost*xp - sint*yp + f->x;
      *hy = sint*xp + cost*yp + f->y;
    }
    else {  // |L^E| = 2
      x1 = (-b + sqrt(b*b-4.0*a*c))/(2.0*a);
      x2 = (-b - sqrt(b*b-4.0*a*c))/(2.0*a);
      y1 = x1*tantlp + ylp - xlp*tantlp;
      y2 = x2*tantlp + ylp - xlp*tantlp;
      switch (ray_intersect_arg(xlp, ylp, tlp, x1, y1, x2, y2)) {
      case 0:
	carmen_warn("expected sensor reading isn't on ray! (break 4) (%f, %f, %f, %f, %f, %f, %f)",
		    xlp, ylp, tlp, x1, y1, x2, y2);
	return -1;
      case 1:
	*hx = cost*x1 - sint*y1 + f->x;
	*hy = sint*x1 + cost*y1 + f->y;
	break;
      case 2:
	*hx = cost*x2 - sint*y2 + f->x;
	*hy = sint*x2 + cost*y2 + f->y;
      }
    }
  }

  //printf("\n ~~~ hx = %f, hy = %f ~~~\n", *hx, *hy);

  return 0;
}

static void trash_filter_sensor_update(carmen_dot_trash_filter_p f,
				       double lx, double ly, double ltheta,
				       float *range) {

  gsl_matrix *K, *M1, *M2, *H, *HT, *R2, *P2;
  double x, y, theta, hx, hy; //, d, dmin;
  int i, s, n; //, j, imin;

  //printf("update_trash_filter(%f,%f,%f)\n", lx, ly, ltheta);

  P2 = gsl_matrix_calloc(5,5);
  carmen_test_alloc(P2);
  
  if (f->sensor_update_list->length > 0 /*|| f->unsensor_update_list->length > 0*/) {
    H = gsl_matrix_alloc(2*f->sensor_update_list->length /*+ 2*f->unsensor_update_list->length*/, 5);
    carmen_test_alloc(H);
    n = 0;
    //printf(" list length = %d ", f->sensor_update_list->length);
    printf("( ");
    for (i = 0; i < f->sensor_update_list->length; i++) {
      s = *(int*)carmen_list_get(f->sensor_update_list, i);
      printf("%d ", s);
      theta = carmen_normalize_theta(ltheta + (s-90)*M_PI/180.0);
      M1 = trash_filter_measurement_jacobian(f, lx, ly, theta);
      if (M1 == NULL)
	continue;
      H->data[MI(H,2*n,0)] = M1->data[MI(M1,0,0)];
      H->data[MI(H,2*n,1)] = M1->data[MI(M1,0,1)];
      H->data[MI(H,2*n,2)] = M1->data[MI(M1,0,2)];
      H->data[MI(H,2*n,3)] = M1->data[MI(M1,0,3)];
      H->data[MI(H,2*n,4)] = M1->data[MI(M1,0,4)];
      H->data[MI(H,2*n+1,0)] = M1->data[MI(M1,1,0)];
      H->data[MI(H,2*n+1,1)] = M1->data[MI(M1,1,1)];
      H->data[MI(H,2*n+1,2)] = M1->data[MI(M1,1,2)];
      H->data[MI(H,2*n+1,3)] = M1->data[MI(M1,1,3)];
      H->data[MI(H,2*n+1,3)] = M1->data[MI(M1,1,4)];
      n++;
      gsl_matrix_free(M1);
    }
    printf(")");
    /*
    printf("( ");
    for (i = 0; i < f->unsensor_update_list->length; i++) {
      s = *(int*)carmen_list_get(f->unsensor_update_list, i);
      printf("%d ", s);
      theta = carmen_normalize_theta(ltheta + (s-90)*M_PI/180.0);
      M1 = person_filter_measurement_jacobian(f, lx, ly, theta);
      if (M1 == NULL)
	continue;
      H->data[MI(H,2*n,0)] = M1->data[MI(M1,0,0)];
      H->data[MI(H,2*n,1)] = M1->data[MI(M1,0,1)];
      H->data[MI(H,2*n,2)] = M1->data[MI(M1,0,2)];
      H->data[MI(H,2*n+1,0)] = M1->data[MI(M1,1,0)];
      H->data[MI(H,2*n+1,1)] = M1->data[MI(M1,1,1)];
      H->data[MI(H,2*n+1,2)] = M1->data[MI(M1,1,2)];
      n++;
      gsl_matrix_free(M1);
    }
    printf(")");
    */
    if (n < f->sensor_update_list->length /*+ f->unsensor_update_list->length*/)
      carmen_die("*** n < f->sensor_update_list->length + f->unsensor_update_list->length (Jacobian) ***");
    HT = gsl_matrix_alloc(H->size2, H->size1);
    carmen_test_alloc(HT);
    gsl_matrix_transpose_memcpy(HT, H);
    
    P2->data[MI(P2,0,0)] = f->P->data[MI(f->P,0,0)];
    P2->data[MI(P2,0,1)] = f->P->data[MI(f->P,0,1)];
    P2->data[MI(P2,1,0)] = f->P->data[MI(f->P,1,0)];
    P2->data[MI(P2,1,1)] = f->P->data[MI(f->P,1,1)];
    //dbug
    P2->data[MI(P2,0,2)] = f->P->data[MI(f->P,0,2)];
    P2->data[MI(P2,0,3)] = f->P->data[MI(f->P,0,3)];
    P2->data[MI(P2,0,4)] = f->P->data[MI(f->P,0,4)];
    P2->data[MI(P2,1,2)] = f->P->data[MI(f->P,1,2)];
    P2->data[MI(P2,1,3)] = f->P->data[MI(f->P,1,3)];
    P2->data[MI(P2,1,4)] = f->P->data[MI(f->P,1,4)];
    P2->data[MI(P2,2,0)] = f->P->data[MI(f->P,2,0)];
    P2->data[MI(P2,2,1)] = f->P->data[MI(f->P,2,1)];
    P2->data[MI(P2,2,2)] = f->P->data[MI(f->P,2,2)];
    P2->data[MI(P2,2,3)] = f->P->data[MI(f->P,2,3)];
    P2->data[MI(P2,2,4)] = f->P->data[MI(f->P,2,4)];
    P2->data[MI(P2,3,0)] = f->P->data[MI(f->P,3,0)];
    P2->data[MI(P2,3,1)] = f->P->data[MI(f->P,3,1)];
    P2->data[MI(P2,3,2)] = f->P->data[MI(f->P,3,2)];
    P2->data[MI(P2,3,3)] = f->P->data[MI(f->P,3,3)];
    P2->data[MI(P2,3,4)] = f->P->data[MI(f->P,3,4)];
    P2->data[MI(P2,4,0)] = f->P->data[MI(f->P,4,0)];
    P2->data[MI(P2,4,1)] = f->P->data[MI(f->P,4,1)];
    P2->data[MI(P2,4,2)] = f->P->data[MI(f->P,4,2)];
    P2->data[MI(P2,4,3)] = f->P->data[MI(f->P,4,3)];
    P2->data[MI(P2,4,4)] = f->P->data[MI(f->P,4,4)];

    M1 = matrix_mult(H, P2);
    M2 = matrix_mult(M1, HT);
    gsl_matrix_free(M1);
    R2 = gsl_matrix_calloc(2*f->sensor_update_list->length /*+ 2*f->unsensor_update_list->length*/,
			   2*f->sensor_update_list->length /*+ 2*f->unsensor_update_list->length*/);
    carmen_test_alloc(R2);
    for (i = 0; i < f->sensor_update_list->length /*+ f->unsensor_update_list->length*/; i++) {
      R2->data[MI(R2,2*i,2*i)] = f->R->data[MI(f->R,0,0)];
      R2->data[MI(R2,2*i,2*i+1)] = f->R->data[MI(f->R,0,1)];
      R2->data[MI(R2,2*i+1,2*i)] = f->R->data[MI(f->R,1,0)];
      R2->data[MI(R2,2*i+1,2*i+1)] = f->R->data[MI(f->R,1,1)];
    }
    gsl_matrix_add(M2, R2);
    matrix_invert(M2);

    if (isnan(M2->data[MI(M2,0,0)])) {
      gsl_matrix_free(M2);
      gsl_matrix_free(R2);
      gsl_matrix_free(H);
      gsl_matrix_free(HT);
      gsl_matrix_free(P2);
      return;
    }

    //printf("\nM2 = \n");
    //gsl_matrix_fprintf(stdout, M2, "%f");
    //printf("\n");

    M1 = matrix_mult(HT, M2);
    gsl_matrix_free(M2);
    K = matrix_mult(P2, M1);
    gsl_matrix_free(M1);
    
    M1 = gsl_matrix_calloc(2*f->sensor_update_list->length /*+ 2*f->unsensor_update_list->length*/, 1);
    carmen_test_alloc(M1);
    n = 0;
    for (i = 0; i < f->sensor_update_list->length; i++) {
      s = *(int*)carmen_list_get(f->sensor_update_list, i);
      theta = carmen_normalize_theta(ltheta + (s-90)*M_PI/180.0);
      x = lx + cos(theta)*range[s];
      y = ly + sin(theta)*range[s];
      if (trash_filter_max_likelihood_measurement(f, lx, ly, theta, &hx, &hy) < 0)
	continue;
      M1->data[MI(M1,2*n,0)] = x - hx;
      M1->data[MI(M1,2*n+1,0)] = y - hy;
      n++;
    }
    /*
    for (i = 0; i < f->unsensor_update_list->length; i++) {
      s = *(int*)carmen_list_get(f->unsensor_update_list, i);
      theta = carmen_normalize_theta(ltheta + (s-90)*M_PI/180.0);
      if (person_filter_max_likelihood_measurement(f, lx, ly, theta, &hx, &hy) < 0)
	continue;
      imin = -1;
      dmin = -1.0;
      for (j = 0; j < f->sensor_update_list->length; j++) {
	s = *(int*)carmen_list_get(f->unsensor_update_list, j);
	theta = carmen_normalize_theta(ltheta + (s-90)*M_PI/180.0);
	x = lx + cos(theta)*range[s];
	y = ly + sin(theta)*range[s];
	d = dist(x-hx, y-hy);
	if (d > dmin) {
	  dmin = d;
	  imin = s;
	}
      }
      theta = carmen_normalize_theta(ltheta + (imin-90)*M_PI/180.0);
      x = lx + cos(theta)*range[imin];
      y = ly + sin(theta)*range[imin];
      M1->data[MI(M1,2*n,0)] = x - hx;
      M1->data[MI(M1,2*n+1,0)] = y - hy;
      n++;
    }
    */
    if (n < f->sensor_update_list->length /*+ f->unsensor_update_list->length*/)
      carmen_die("*** n < f->sensor_update_list->length + f->unsensor_update_list->length (h) ***");
    
    M2 = matrix_mult(K, M1);
    gsl_matrix_free(M1);
    
    f->x += M2->data[MI(M2,0,0)];
    f->y += M2->data[MI(M2,1,0)];
    f->theta += carmen_normalize_theta(M2->data[MI(M2,2,0)]);
    f->logw1 += M2->data[MI(M2,3,0)];
    f->logw2 += M2->data[MI(M2,4,0)];

    if (f->logw1 < f->logw2) {
      x = f->logw1;
      f->logw1 = f->logw2;
      f->logw2 = x;
    }
    if (exp(f->logw1) < 0.15)
      f->logw1 = log(0.15);

    gsl_matrix_free(M2);
    M1 = matrix_mult(K, H);
    M2 = gsl_matrix_alloc(5,5);  //dbug
    carmen_test_alloc(M2);
    gsl_matrix_set_identity(M2);
    gsl_matrix_sub(M2, M1);
    gsl_matrix_free(M1);
    M1 = matrix_mult(M2, f->P);
    gsl_matrix_free(M2);
    gsl_matrix_free(f->P);
    f->P = M1;

    gsl_matrix_free(R2);
    gsl_matrix_free(H);
    gsl_matrix_free(HT);
    gsl_matrix_free(K);
  }

  gsl_matrix_free(P2);
}

#if 0
static void door_filter_motion_update(carmen_dot_door_filter_p f) {

  double ax, ay;
  
  // kalman filter time update with brownian motion model
  ax = carmen_uniform_random(-1.0, 1.0) * f->a;
  ay = carmen_uniform_random(-1.0, 1.0) * f->a;

  //printf("ax = %.4f, ay = %.4f\n", ax, ay);

  f->x += ax;
  f->y += ay;
  f->px = ax*ax*f->px + f->qx;
  f->pxy = ax*ax*f->pxy + f->qxy;
  f->py = ay*ay*f->py + f->qy;
}
#endif

static void door_filter_sensor_update(carmen_dot_door_filter_p f, double x, double y) {

  double rx, ry, rxy, rt;
  double pxx, pxy, pyx, pyy, pt;
  double mxx, mxy, myx, myy, mt;
  double kxx, kxy, kyx, kyy, kt;

  pxx = f->px;
  pxy = f->pxy;
  pyx = f->pxy;
  pyy = f->py;
  pt = f->pt; //dbug: + f->qt ?

  rx = default_door_filter_rx*cos(f->t)*cos(f->t) +
    default_door_filter_ry*sin(f->t)*sin(f->t);
  ry = default_door_filter_rx*sin(f->t)*sin(f->t) +
    default_door_filter_ry*cos(f->t)*cos(f->t);
  rxy = default_door_filter_rx*cos(f->t)*sin(f->t) -
    default_door_filter_ry*cos(f->t)*sin(f->t);
  rt = default_door_filter_rt;

  // kalman filter sensor update
  mxx = pxx + rx;
  mxy = pxy + rxy;
  myx = pyx + rxy;
  myy = pyy + ry;
  mt = pt + rt;
  if (invert2d(&mxx, &mxy, &myx, &myy) < 0) {
    carmen_die("Error: can't invert matrix M");
    return;
  }
  mt = 1.0/mt;
  kxx = pxx*mxx+pxy*myx;
  kxy = pxx*mxy+pxy*myy;
  kyx = pyx*mxx+pyy*myx;
  kyy = pyx*mxy+pyy*myy;
  kt = pt*mt;
  f->x = f->x + kxx*(x - f->x) + kxy*(y - f->y);
  f->y = f->y + kyx*(x - f->x) + kyy*(y - f->y);
  f->px = (1.0 - kxx)*pxx + (-kxy)*pyx;
  f->pxy = (1.0 - kxx)*pxy + (-kxy)*pyy;
  f->py = (-kyx)*pxy + (1.0 - kyy)*pyy;
  f->pt = (1.0 - kt)*pt;
}

static void add_new_dot_filter(int *cluster_map, int c, int n,
			       double *x, double *y) {
  int i, id, cnt;
  double ux, uy, ulogr, vx, vy, vxy, vlogr, vxlogr, vylogr, lr;
  carmen_dot_person_filter_p pf;
  carmen_dot_trash_filter_p tf;
  carmen_dot_door_filter_p df;

  for (id = 0; id < num_filters; id++) {
    for (i = 0; i < num_filters; i++)
      if (filters[i].id == id)
	break;
    if (i == num_filters)
      break;
  }

  printf("A%d ", id);
  fflush(0);

  for (i = 0; i < n && cluster_map[i] != c; i++);
  if (i == n)
    carmen_die("Error: invalid cluster! (cluster %d not found)\n", c);

  num_filters++;
  filters = (carmen_dot_filter_p) realloc(filters, num_filters*sizeof(carmen_dot_filter_t));
  carmen_test_alloc(filters);

  filters[num_filters-1].id = id;

  ux = uy = 0.0;
  cnt = 0;
  for (i = 0; i < n; i++)
    if (cluster_map[i] == c) {
      ux += x[i];
      uy += y[i];
      cnt++;
    }
  ux /= (double)cnt;
  uy /= (double)cnt;

  ulogr = 0.0;
  for (i = 0; i < n; i++)
    if (cluster_map[i] == c)
      ulogr += log(sqrt((x[i]-ux)*(x[i]-ux) + (y[i]-uy)*(y[i]-uy)));
  ulogr /= (double)cnt;

  vx = vy = vxy = vlogr = vxlogr = vylogr = 0.0;
  for (i = 0; i < n; i++)
    if (cluster_map[i] == c) {
      vx += (x[i]-ux)*(x[i]-ux);
      vy += (y[i]-uy)*(y[i]-uy);
      vxy += (x[i]-ux)*(y[i]-uy);
      lr = log(sqrt((x[i]-ux)*(x[i]-ux) + (y[i]-uy)*(y[i]-uy)));
      vlogr += (lr-ulogr)*(lr-ulogr);
      vxlogr += (x[i]-ux)*(lr-ulogr);
      vylogr += (y[i]-uy)*(lr-ulogr);
    }
  vx /= (double)cnt;
  vy /= (double)cnt;
  vxy /= (double)cnt;
  vlogr /= (double)cnt;
  vxlogr /= (double)cnt;
  vylogr /= (double)cnt;

  printf("(ux = %.2f, uy = %.2f)", ux, uy);

  pf = &filters[num_filters-1].person_filter;
  pf->sensor_update_list = carmen_list_create(sizeof(int), 5);
  pf->unsensor_update_list = carmen_list_create(sizeof(int), 5);
  pf->x = ux;
  pf->y = uy;
  pf->logr = ulogr;
  pf->x0 = ux;
  pf->y0 = uy;
  pf->hidden_cnt = 0;
  pf->P = gsl_matrix_calloc(3,3);
  carmen_test_alloc(pf->P);
  gsl_matrix_set(pf->P, 0, 0, 0.01); //vx);
  gsl_matrix_set(pf->P, 0, 1, 0.001); //vxy);
  gsl_matrix_set(pf->P, 0, 2, 0.001); //vxlogr);
  gsl_matrix_set(pf->P, 1, 0, 0.001); //vxy);
  gsl_matrix_set(pf->P, 1, 1, 0.01); //vy);
  gsl_matrix_set(pf->P, 1, 2, 0.001); //vylogr);
  gsl_matrix_set(pf->P, 2, 0, 0.001); //vxlogr);
  gsl_matrix_set(pf->P, 2, 1, 0.001); //vylogr);
  gsl_matrix_set(pf->P, 2, 2, 0.00001); //vlogr);
  pf->a = default_person_filter_a;
  pf->Q = gsl_matrix_calloc(3,3);
  carmen_test_alloc(pf->Q);
  gsl_matrix_set(pf->Q, 0, 0, default_person_filter_qx);
  gsl_matrix_set(pf->Q, 0, 1, default_person_filter_qxy);
  gsl_matrix_set(pf->Q, 1, 0, default_person_filter_qxy);
  gsl_matrix_set(pf->Q, 1, 1, default_person_filter_qy);
  gsl_matrix_set(pf->Q, 2, 2, default_person_filter_qlogr);
  pf->R = gsl_matrix_calloc(2,2);
  carmen_test_alloc(pf->R);
  gsl_matrix_set(pf->R, 0, 0, default_person_filter_rx);
  gsl_matrix_set(pf->R, 0, 1, default_person_filter_rxy);
  gsl_matrix_set(pf->R, 1, 0, default_person_filter_rxy);
  gsl_matrix_set(pf->R, 1, 1, default_person_filter_ry);

  /*
  for (i = 0; i < n; i++)
    if (cluster_map[i] == c) {
      //printf(" (%.2f, %.2f)", x[i], y[i]);
      person_filter_sensor_update(pf, x[i], y[i]);
    }
  //printf("\n");
  */

  tf = &filters[num_filters-1].trash_filter;
  tf->sensor_update_list = carmen_list_create(sizeof(int), 5);
  tf->unsensor_update_list = carmen_list_create(sizeof(int), 5);
  tf->x = ux;
  tf->y = uy;
  tf->theta = bnorm_theta(vx, vy, vxy);
  tf->logw1 = log(bnorm_w1(vx, vy, vxy));
  tf->logw2 = log(bnorm_w2(vx, vy, vxy));
  tf->P = gsl_matrix_calloc(5,5);
  carmen_test_alloc(tf->P);
  gsl_matrix_set(tf->P, 0, 0, vx);
  gsl_matrix_set(tf->P, 0, 1, vxy);
  gsl_matrix_set(tf->P, 1, 0, vxy);
  gsl_matrix_set(tf->P, 1, 1, vy);
  gsl_matrix_set(tf->P, 2, 2, 0.1);  //dbug: param?
  gsl_matrix_set(tf->P, 3, 3, 0.1);  //dbug: param?
  gsl_matrix_set(tf->P, 4, 4, 0.1);  //dbug: param?
  tf->a = default_trash_filter_a;
  tf->Q = gsl_matrix_calloc(5,5);
  carmen_test_alloc(tf->Q);
  gsl_matrix_set(tf->Q, 0, 0, default_trash_filter_qx);
  gsl_matrix_set(tf->Q, 0, 1, default_trash_filter_qxy);
  gsl_matrix_set(tf->Q, 1, 0, default_trash_filter_qxy);
  gsl_matrix_set(tf->Q, 1, 1, default_trash_filter_qy);
  gsl_matrix_set(tf->Q, 2, 2, default_trash_filter_qtheta);
  gsl_matrix_set(tf->Q, 3, 3, default_trash_filter_qlogw1);
  gsl_matrix_set(tf->Q, 4, 4, default_trash_filter_qlogw2);
  tf->R = gsl_matrix_calloc(2,2);
  carmen_test_alloc(tf->R);
  gsl_matrix_set(tf->R, 0, 0, default_trash_filter_rx);
  gsl_matrix_set(tf->R, 1, 0, default_trash_filter_rxy);
  gsl_matrix_set(tf->R, 0, 1, default_trash_filter_rxy);
  gsl_matrix_set(tf->R, 1, 1, default_trash_filter_ry);

  /*
  for (i = 0; i < n; i++)
    if (cluster_map[i] == c)
      trash_filter_sensor_update(tf, x[i], y[i]);
  */

  df = &filters[num_filters-1].door_filter;
  df->sensor_update_list = carmen_list_create(sizeof(int), 5);
  df->unsensor_update_list = carmen_list_create(sizeof(int), 5);
  df->x = ux;
  df->y = uy;
  df->t = bnorm_theta(vx, vy, vxy);
  //    bnorm_theta(filters[num_filters-1].person_filter.px,
  //	filters[num_filters-1].person_filter.py,
  //	filters[num_filters-1].person_filter.pxy);
  df->px = vx; //default_door_filter_px;
  df->py = vy; //default_door_filter_py;
  df->pt = default_door_filter_pt;
  df->pxy = vxy; //0;  //dbug?
  df->a = default_door_filter_a;
  df->qx = default_door_filter_qx;
  df->qy = default_door_filter_qy;
  df->qxy = default_door_filter_qxy;
  df->qt = default_door_filter_qt;  

  for (i = 0; i < n; i++)
    if (cluster_map[i] == c)
      door_filter_sensor_update(df, x[i], y[i]);

  filters[num_filters-1].type = CARMEN_DOT_TRASH;  //dbug?
  filters[num_filters-1].allow_change = 1;
  filters[num_filters-1].sensor_update_cnt = 0;
  filters[num_filters-1].do_motion_update = 0;
  filters[num_filters-1].updated = 1;
  filters[num_filters-1].invisible = 0;
  filters[num_filters-1].invisible_cnt = 0;

  //print_filters();
}

static double map_prob(double x, double y) {

  carmen_world_point_t wp;
  carmen_map_point_t mp;

  wp.map = &static_map;
  wp.pose.x = x;
  wp.pose.y = y;

  carmen_world_to_map(&wp, &mp);

  /*
  printf("map_prob(%.2f, %.2f): localize_map prob = %.4f\n", x, y,
	 localize_map.prob[mp.x][mp.y]);
  printf("map_prob(%.2f, %.2f): localize_map gprob = %.4f\n", x, y,
	 localize_map.gprob[mp.x][mp.y]);
  printf("map_prob(%.2f, %.2f): localize_map complete prob = %.4f\n", x, y,
	 localize_map.complete_prob[mp.x*static_map.config.y_size+mp.y]);
  */

  return exp(localize_map.gprob[mp.x][mp.y]);
}

#if 0
static int list_contains(carmen_list_t *list, int entry) {

  int i;

  for (i = 0; i < list->length; i++)
    if (*(int*)carmen_list_get(list, i) == entry)
      return 1;

  return 0;
}
#endif

static int dot_filter(double lx, double ly, double ltheta, double x, double y, int ri) {

  int i, imax;
  double p, pmax;
  //double ux, uy;
  //double vx, vy, vxy;
  double hx, hy;
  double px, py;

  imax = -1;
  pmax = 0.0;

  for (i = 0; i < num_filters; i++) {
    if (ray_intersect_circle(lx, ly, ltheta, filters[i].person_filter.x,
			     filters[i].person_filter.y, exp(filters[i].person_filter.logr),
			     &px, &py)) {
      if (dist(x-lx, y-ly) - dist(px-lx, py-ly) > 1.0) {  // long reading that goes through filter
	carmen_list_add(filters[i].person_filter.unsensor_update_list, &ri);
      }
	//person_filter_sensor_update_r(&filters[i].person_filter, lx, ly, ltheta, x, y);
    }
    if (person_contains(&filters[i].person_filter, x, y, 2.0)) {  //dbug: param?
      person_filter_max_likelihood_measurement(&filters[i].person_filter, lx, ly, ltheta, &hx, &hy);
      p = 0.2/dist(x-hx, y-hy);
      if (p > pmax) {
	pmax = p;
	imax = i;
      }
    }

    if (trash_contains(&filters[i].trash_filter, x, y, 2.0)) {  //dbug: param?
      trash_filter_max_likelihood_measurement(&filters[i].trash_filter, lx, ly, ltheta, &hx, &hy);
      p = 0.2/dist(x-hx, y-hy);
      if (p > pmax) {
	pmax = p;
	imax = i;
      }
    }

    /*    
    if (door_contains(&filters[i].door_filter, x, y, 3)) {  //dbug: param?
      ux = filters[i].door_filter.x;
      uy = filters[i].door_filter.y;
      vx = filters[i].door_filter.px;
      vy = filters[i].door_filter.py;
      vxy = filters[i].door_filter.pxy;
      p = bnorm_f(x, y, ux, uy, vx, vy, vxy);  //dbug: change to door sensor model
      if (p > pmax) {
	pmax = p;
	imax = i;
      }
    }
    */
  }

  /*
  if (imax >= 0)
    printf("pmax = %.4f, map_prob = %.4f at filter %d (x=%.2f, y = %.2f)\n",
	   pmax, map_prob(x, y), imax, x, y);
  */

  if (map_prob(x, y) >= map_occupied_threshold) {
#ifdef HAVE_GRAPHICS
    laser_mask[ri] = 0;
#endif
    return 1;
  }

  if (imax >= 0 && pmax > map_prob(x, y)) {
    carmen_list_add(filters[imax].person_filter.sensor_update_list, &ri);
    carmen_list_add(filters[imax].trash_filter.sensor_update_list, &ri);
    return 1;
  }

  return 0;
}

static void update_dots(double lx, double ly, double ltheta, float *range) {

  int i;

  //printf("\nupdate_dots(%f,%f,%f)\n", lx, ly, ltheta);

  for (i = 0; i < num_filters; i++) {
    if (filters[i].person_filter.sensor_update_list->length > 0 ||
	filters[i].person_filter.unsensor_update_list->length > 0) {
      if (filters[i].person_filter.sensor_update_list->length > 0) {
	filters[i].person_filter.hidden_cnt = 0;
	filters[i].updated = 1;
      }
      person_filter_sensor_update(&filters[i].person_filter, lx, ly, ltheta, range);
    }

    if (1) { //filters[i].do_motion_update) {
      filters[i].do_motion_update = 0;
      trash_filter_motion_update(&filters[i].trash_filter);
      //door_filter_motion_update(&filters[i].door_filter);
      //filters[i].updated = 1;
    }
    if (filters[i].trash_filter.sensor_update_list->length > 0) {
      if (do_sensor_update || filters[i].sensor_update_cnt < sensor_update_cnt) {
	filters[i].sensor_update_cnt++;
	trash_filter_sensor_update(&filters[i].trash_filter, lx, ly, ltheta, range);
	//dbug: change!!
	//door_filter_sensor_update(&filters[i].door_filter, x, y);
	filters[i].updated = 1;
      }
    }

    //if (filters[i].type == CARMEN_DOT_PERSON)
    //filters[i].updated = 1;
  }
}

// returns 1 if point is in map
static int map_filter(double x, double y, double r) {

  carmen_world_point_t wp;
  carmen_map_point_t mp;
  double d;
  int md, i, j;

  wp.map = &static_map;
  wp.pose.x = x;
  wp.pose.y = y;

  carmen_world_to_map(&wp, &mp);

  d = (map_diff_threshold + r*map_diff_threshold_scalar)/static_map.config.resolution;
  md = carmen_round(d);
  
  for (i = mp.x-md; i <= mp.x+md; i++)
    for (j = mp.y-md; j <= mp.y+md; j++)
      if (is_in_map(i, j) && dist(i-mp.x, j-mp.y) <= d &&
	  exp(localize_map.gprob[i][j]) >= map_occupied_threshold)
	return 1;

  return 0;
}

static int cluster(int *cluster_map, int cluster_cnt, int current_reading,
		   double *x, double *y) {
  
  int i, r;
  int nmin;
  double dmin;
  double d;

  if (cluster_cnt == 0) {
    cluster_map[current_reading] = 1;
    //printf("putting reading %d at (%.2f %.2f) in cluster 1\n", current_reading, *x, *y);
    return 1;
  }

  r = current_reading;
  dmin = new_cluster_threshold;
  nmin = -1;

  // nearest neighbor
  for (i = 0; i < r; i++) {
    if (cluster_map[i]) {
      d = dist(x[i]-x[r], y[i]-y[r]);
      if (d < dmin) {
	dmin = d;
	nmin = i;
      }
    }
  }

  if (nmin < 0) {
    cluster_cnt++;
    cluster_map[r] = cluster_cnt;
  }
  else
    cluster_map[r] = cluster_map[nmin];

  //  printf("putting reading %d at (%.2f %.2f) in cluster %d\n", current_reading, *x, *y,
  //	 cluster_map[r]);

  return cluster_cnt;
}

static void delete_filter(int i) {

  //printf("D%d\n", filters[i].id);

  gsl_matrix_free(filters[i].person_filter.P);
  gsl_matrix_free(filters[i].person_filter.Q);
  gsl_matrix_free(filters[i].person_filter.R);
  gsl_matrix_free(filters[i].trash_filter.P);
  gsl_matrix_free(filters[i].trash_filter.Q);
  gsl_matrix_free(filters[i].trash_filter.R);

  carmen_list_destroy(&filters[i].person_filter.sensor_update_list);
  carmen_list_destroy(&filters[i].person_filter.unsensor_update_list);
  carmen_list_destroy(&filters[i].trash_filter.sensor_update_list);
  carmen_list_destroy(&filters[i].trash_filter.unsensor_update_list);
  carmen_list_destroy(&filters[i].door_filter.sensor_update_list);
  carmen_list_destroy(&filters[i].door_filter.unsensor_update_list);

  if (i < num_filters-1)
    memmove(&filters[i], &filters[i+1], (num_filters-i-1)*sizeof(carmen_dot_filter_t));
  num_filters--;

  //print_filters();
}

/*
 * delete people filters when we haven't seen them for
 * kill_hidden_person_cnt updates.
 */
static void kill_people() {

  int i;

  for (i = 0; i < num_filters; i++)
    if (!filters[i].updated)
      if (++filters[i].person_filter.hidden_cnt >= kill_hidden_person_cnt &&
	  filters[i].type == CARMEN_DOT_PERSON)
	delete_filter(i);
}

/*
 * (1) classify dot as a person (if displacement > threshold)
 * (2) do motion update (if !hidden or classified as a person)
 */
static void filter_motion() {

  int i;
  double dx, dy;

  for (i = 0; i < num_filters; i++) {
    
    dx = filters[i].person_filter.x - filters[i].person_filter.x0;
    dy = filters[i].person_filter.y - filters[i].person_filter.y0;

    if (dist(dx, dy) >= person_filter_displacement_threshold) {
      filters[i].type = CARMEN_DOT_PERSON;
      filters[i].allow_change = 0;
    }
    
    if (filters[i].person_filter.hidden_cnt == 0 || filters[i].type == CARMEN_DOT_PERSON) {
      person_filter_motion_update(&filters[i].person_filter);
      filters[i].updated = 1;
    }
  }
}

//dbug: change to use ray_intersect functions
void trace_laser(double x1, double y1, double x2, double y2) {

  int i;
  double x, y, dx, dy, stdev;

  dx = trace_resolution * cos(atan2(y2-y1, x2-x1));
  dy = trace_resolution * sin(atan2(y2-y1, x2-x1));

  stdev = see_through_stdev;

  x = x1;
  y = y1;
  while ((dx >= 0 ? x <= x2 : x >= x2) && (dy >= 0 ? y <= y2 : y >= y2)) {
    //check for intersection with filters
    for (i = 0; i < num_filters; i++)
      if (!filters[i].invisible && dot_contains(&filters[i], x, y, stdev))
	filters[i].invisible = 1;
    x += dx;
    y += dy;
  }
}

 static void laser_handler(carmen_robot_laser_message *laser) {

   int i, c, n, dotf, mapf;
  static int cluster_map[500];
  static int cluster_cnt;
  static double x[500], y[500];
  //static int odd = 0;
  double ltheta;

  if (static_map.map == NULL || odom.timestamp == 0.0)
    return;

  carmen_localize_correct_laser(laser, &odom);

  kill_people();  // when we haven't seen them for a while
  filter_motion();

  /*
  if (odd) {
    printf("*M*");
    fflush(0);
    publish_all_dot_msgs();  //dbug
  }
  */

  for (i = 0; i < num_filters; i++) {
    filters[i].updated = 0;
    filters[i].invisible = 0;
    filters[i].person_filter.sensor_update_list->length = 0;
    filters[i].person_filter.unsensor_update_list->length = 0;
    filters[i].trash_filter.sensor_update_list->length = 0;
    filters[i].trash_filter.unsensor_update_list->length = 0;
  }

  // don't update trash & door filters unless we've moved
  if (dist(last_sensor_update_odom.x - odom.globalpos.x,
	   last_sensor_update_odom.y - odom.globalpos.y) >= sensor_update_dist) {
    do_sensor_update = 1;
    for (i = 0; i < num_filters; i++)
      filters[i].do_motion_update = 1;
  }
  else
    do_sensor_update = 0;  
  if (do_sensor_update) {
    last_sensor_update_odom.x = odom.globalpos.x;
    last_sensor_update_odom.y = odom.globalpos.y;
  }

  cluster_cnt = 0;
  for (i = 0; i < laser->num_readings; i++) {
    cluster_map[i] = 0;
    if (laser->range[i] < laser_max_range) {
      ltheta = carmen_normalize_theta(laser->theta + (i-90)*M_PI/180.0);
      x[i] = laser->x + cos(ltheta) * laser->range[i];
      y[i] = laser->y + sin(ltheta) * laser->range[i];
      trace_laser(laser->x, laser->y, x[i], y[i]);
#ifdef HAVE_GRAPHICS
      laser_mask[i] = 1;
#endif
      dotf = dot_filter(laser->x, laser->y, ltheta, x[i], y[i], i);
      mapf = (dotf ? 0 : map_filter(x[i], y[i], laser->range[i]));
#ifdef HAVE_GRAPHICS
      if (mapf)
	laser_mask[i] = 0;
#endif
      if (!dotf && !mapf)
	cluster_cnt = cluster(cluster_map, cluster_cnt, i, x, y);
    }
  }

  update_dots(laser->x, laser->y, laser->theta, laser->range);

  // delete invisible filters (filters we can see through)
  for (i = 0; i < num_filters; i++) {
    if (filters[i].invisible && !filters[i].updated) {
      if (++filters[i].invisible_cnt >= invisible_cnt)
	delete_filter(i);
    }
    else
      filters[i].invisible_cnt = 0;
  }      

  //printf("\nclusters: ");
  for (c = 1; c <= cluster_cnt; c++) {
    printf("( ");
    n = 0;
    for (i = 0; i < laser->num_readings; i++)
      if (cluster_map[i] == c) {
	printf("%d ", i);
	n++;
      }
    printf(")");
    if (n >= new_filter_threshold)
      add_new_dot_filter(cluster_map, c, laser->num_readings, x, y);
  }

  /*
  if (!odd) {  //dbug
    printf("*S*");
    fflush(0);
  */
  publish_all_dot_msgs();
  /*
    odd = 1;
  }
  else
    odd = 0;
  */


#ifdef HAVE_GRAPHICS
  get_map_window();
  redraw();
#endif


}

/*******************************************
static void publish_person_msg(carmen_dot_filter_p f, int delete) {

  static carmen_dot_person_msg msg;
  static int first = 1;
  IPC_RETURN_TYPE err;
  
  if (first) {
    strcpy(msg.host, carmen_get_tenchar_host_name());
    first = 0;
  }

  msg.person.id = f->id;
  msg.person.x = f->person_filter.x;
  msg.person.y = f->person_filter.y;
  msg.person.vx = f->person_filter.px;
  msg.person.vy = f->person_filter.py;
  msg.person.vxy = f->person_filter.pxy;
  msg.delete = delete;
  msg.timestamp = carmen_get_time_ms();

  err = IPC_publishData(CARMEN_DOT_PERSON_MSG_NAME, &msg);
  carmen_test_ipc_exit(err, "Could not publish", CARMEN_DOT_PERSON_MSG_NAME);
}

static void publish_trash_msg(carmen_dot_filter_p f, int delete) {

  static carmen_dot_trash_msg msg;
  static int first = 1;
  IPC_RETURN_TYPE err;
  
  if (first) {
    strcpy(msg.host, carmen_get_tenchar_host_name());
    first = 0;
  }

  msg.trash.id = f->id;
  msg.trash.x = f->trash_filter.x;
  msg.trash.y = f->trash_filter.y;
  msg.trash.vx = f->trash_filter.px;
  msg.trash.vy = f->trash_filter.py;
  msg.trash.vxy = f->trash_filter.pxy;
  msg.delete = delete;
  msg.timestamp = carmen_get_time_ms();

  err = IPC_publishData(CARMEN_DOT_TRASH_MSG_NAME, &msg);
  carmen_test_ipc_exit(err, "Could not publish", CARMEN_DOT_TRASH_MSG_NAME);
}

static void publish_door_msg(carmen_dot_filter_p f, int delete) {

  static carmen_dot_door_msg msg;
  static int first = 1;
  IPC_RETURN_TYPE err;
  
  if (first) {
    strcpy(msg.host, carmen_get_tenchar_host_name());
    first = 0;
  }

  msg.door.id = f->id;
  msg.door.x = f->door_filter.x;
  msg.door.y = f->door_filter.y;
  msg.door.theta = f->door_filter.t;
  msg.door.vx = f->door_filter.px;
  msg.door.vy = f->door_filter.py;
  msg.door.vxy = f->door_filter.pxy;
  msg.door.vtheta = f->door_filter.pt;
  msg.delete = delete;
  msg.timestamp = carmen_get_time_ms();

  err = IPC_publishData(CARMEN_DOT_DOOR_MSG_NAME, &msg);
  carmen_test_ipc_exit(err, "Could not publish", CARMEN_DOT_DOOR_MSG_NAME);
}

static void publish_dot_msg(carmen_dot_filter_p f, int delete) {

  switch (f->type) {
  case CARMEN_DOT_PERSON:
    //if (delete)
    //printf("publishing delete person %d msg\n", f->id);
    publish_person_msg(f, delete);
    break;
  case CARMEN_DOT_TRASH:
    //if (delete)
    //printf("publishing delete trash %d msg\n", f->id);
    publish_trash_msg(f, delete);
    break;
  case CARMEN_DOT_DOOR:
    //if (delete)
    //printf("publishing delete door %d msg\n", f->id);
    publish_door_msg(f, delete);
    break;
  }
}
***************************************************/

static void publish_all_dot_msgs() {

  static carmen_dot_all_people_msg all_people_msg;
  static carmen_dot_all_trash_msg all_trash_msg;
  static carmen_dot_all_doors_msg all_doors_msg;
  static int first = 1;
  IPC_RETURN_TYPE err;
  int i, n;
  
  if (first) {
    all_people_msg.people = NULL;
    strcpy(all_people_msg.host, carmen_get_tenchar_host_name());
    all_trash_msg.trash = NULL;
    strcpy(all_trash_msg.host, carmen_get_tenchar_host_name());
    all_doors_msg.doors = NULL;
    strcpy(all_doors_msg.host, carmen_get_tenchar_host_name());
    first = 0;
  }

  for (n = i = 0; i < num_filters; i++)
    if (filters[i].type == CARMEN_DOT_PERSON)
      n++;

  all_people_msg.num_people = n;
  if (n == 0)
    all_people_msg.people = NULL;
  else {
    all_people_msg.people = (carmen_dot_person_p)
      realloc(all_people_msg.people, n*sizeof(carmen_dot_person_t));
  
    for (n = i = 0; i < num_filters; i++)
      if (filters[i].type == CARMEN_DOT_PERSON) {
	all_people_msg.people[n].id = filters[i].id;
	all_people_msg.people[n].x = filters[i].person_filter.x;
	all_people_msg.people[n].y = filters[i].person_filter.y;
	all_people_msg.people[n].r = exp(filters[i].person_filter.logr);
	all_people_msg.people[n].vx = gsl_matrix_get(filters[i].person_filter.P, 0, 0);
	all_people_msg.people[n].vy = gsl_matrix_get(filters[i].person_filter.P, 1, 1);
	all_people_msg.people[n].vxy = gsl_matrix_get(filters[i].person_filter.P, 0, 1);
	n++;
      }
  }

  all_people_msg.timestamp = carmen_get_time_ms();

  err = IPC_publishData(CARMEN_DOT_ALL_PEOPLE_MSG_NAME, &all_people_msg);
  carmen_test_ipc_exit(err, "Could not publish", CARMEN_DOT_ALL_PEOPLE_MSG_NAME);

  for (n = i = 0; i < num_filters; i++)
    if (filters[i].type == CARMEN_DOT_TRASH)
      n++;

  all_trash_msg.num_trash = n;
  if (n == 0)
    all_trash_msg.trash = NULL;
  else {
    all_trash_msg.trash = (carmen_dot_trash_p)
      realloc(all_trash_msg.trash, n*sizeof(carmen_dot_trash_t));
  
    for (n = i = 0; i < num_filters; i++)
      if (filters[i].type == CARMEN_DOT_TRASH) {
	all_trash_msg.trash[n].id = filters[i].id;
	all_trash_msg.trash[n].x = filters[i].trash_filter.x;
	all_trash_msg.trash[n].y = filters[i].trash_filter.y;
	all_trash_msg.trash[n].theta = filters[i].trash_filter.theta;
	all_trash_msg.trash[n].major = exp(filters[i].trash_filter.logw1);
	all_trash_msg.trash[n].minor = exp(filters[i].trash_filter.logw2);
	all_trash_msg.trash[n].vx = gsl_matrix_get(filters[i].trash_filter.P, 0, 0);
	all_trash_msg.trash[n].vy = gsl_matrix_get(filters[i].trash_filter.P, 1, 1);
	all_trash_msg.trash[n].vxy = gsl_matrix_get(filters[i].trash_filter.P, 0, 1);
	n++;
      }
  }

  all_trash_msg.timestamp = carmen_get_time_ms();

  err = IPC_publishData(CARMEN_DOT_ALL_TRASH_MSG_NAME, &all_trash_msg);
  carmen_test_ipc_exit(err, "Could not publish", CARMEN_DOT_ALL_TRASH_MSG_NAME);

  for (n = i = 0; i < num_filters; i++)
    if (filters[i].type == CARMEN_DOT_DOOR)
      n++;

  all_doors_msg.num_doors = n;
  if (n == 0)
    all_doors_msg.doors = NULL;
  else {
    all_doors_msg.doors = (carmen_dot_door_p)
      realloc(all_doors_msg.doors, n*sizeof(carmen_dot_door_t));
  
    for (n = i = 0; i < num_filters; i++)
      if (filters[i].type == CARMEN_DOT_DOOR) {
	all_doors_msg.doors[n].id = filters[i].id;
	all_doors_msg.doors[n].x = filters[i].door_filter.x;
	all_doors_msg.doors[n].y = filters[i].door_filter.y;
	all_doors_msg.doors[n].vx = filters[i].door_filter.px;
	all_doors_msg.doors[n].vy = filters[i].door_filter.py;
	all_doors_msg.doors[n].vxy = filters[i].door_filter.pxy;
	n++;
      }
  }

  all_doors_msg.timestamp = carmen_get_time_ms();

  err = IPC_publishData(CARMEN_DOT_ALL_DOORS_MSG_NAME, &all_doors_msg);
  carmen_test_ipc_exit(err, "Could not publish", CARMEN_DOT_ALL_DOORS_MSG_NAME);
}

static void respond_all_people_msg(MSG_INSTANCE msgRef) {

  static carmen_dot_all_people_msg msg;
  static int first = 1;
  IPC_RETURN_TYPE err;
  int i, n;

  if (first) {
    first = 0;
    msg.people = NULL;
    strcpy(msg.host, carmen_get_tenchar_host_name());
  }
  
  for (n = i = 0; i < num_filters; i++)
    if (filters[i].type == CARMEN_DOT_PERSON)
      n++;

  msg.num_people = n;
  if (n == 0)
    msg.people = NULL;
  else {
    msg.people = (carmen_dot_person_p)
		  realloc(msg.people, n*sizeof(carmen_dot_person_t));
    carmen_test_alloc(msg.people);
    for (n = i = 0; i < num_filters; i++) {
      if (filters[i].type == CARMEN_DOT_PERSON) {
	msg.people[n].id = filters[i].id;
	msg.people[n].x = filters[i].person_filter.x;
	msg.people[n].y = filters[i].person_filter.y;
	msg.people[n].vx = gsl_matrix_get(filters[i].person_filter.P, 0, 0);
	msg.people[n].vy = gsl_matrix_get(filters[i].person_filter.P, 1, 1);
	msg.people[n].vxy = gsl_matrix_get(filters[i].person_filter.P, 0, 1);
	n++;
      }
    }
  }

  msg.timestamp = carmen_get_time_ms();

  err = IPC_respondData(msgRef, CARMEN_DOT_ALL_PEOPLE_MSG_NAME, &msg);
  carmen_test_ipc(err, "Could not respond",
		  CARMEN_DOT_ALL_PEOPLE_MSG_NAME);
}

static void respond_all_trash_msg(MSG_INSTANCE msgRef) {

  static carmen_dot_all_trash_msg msg;
  static int first = 1;
  IPC_RETURN_TYPE err;
  int i, n;
  
  if (first) {
    first = 0;
    msg.trash = NULL;
    strcpy(msg.host, carmen_get_tenchar_host_name());
  }
  
  for (n = i = 0; i < num_filters; i++)
    if (filters[i].type == CARMEN_DOT_TRASH)
      n++;

  msg.num_trash = n;
  if (n == 0)
    msg.trash = NULL;

  else {
    msg.trash = (carmen_dot_trash_p)
      realloc(msg.trash, n*sizeof(carmen_dot_trash_t));
    carmen_test_alloc(msg.trash);
    for (n = i = 0; i < num_filters; i++) {
      if (filters[i].type == CARMEN_DOT_TRASH) {
	msg.trash[n].id = filters[i].id;
	msg.trash[n].x = filters[i].trash_filter.x;
	msg.trash[n].y = filters[i].trash_filter.y;
	msg.trash[n].vx = gsl_matrix_get(filters[i].trash_filter.P, 0, 0);
	msg.trash[n].vy = gsl_matrix_get(filters[i].trash_filter.P, 1, 1);
	msg.trash[n].vxy = gsl_matrix_get(filters[i].trash_filter.P, 0, 1);
	n++;
      }
    }
  }

  msg.timestamp = carmen_get_time_ms();

  err = IPC_respondData(msgRef, CARMEN_DOT_ALL_TRASH_MSG_NAME, &msg);
  carmen_test_ipc(err, "Could not respond",
		  CARMEN_DOT_ALL_TRASH_MSG_NAME);
}

static void respond_all_doors_msg(MSG_INSTANCE msgRef) {

  static carmen_dot_all_doors_msg msg;
  static int first = 1;
  IPC_RETURN_TYPE err;
  int i, n;
  
  if (first) {
    first = 0;
    msg.doors = NULL;
    strcpy(msg.host, carmen_get_tenchar_host_name());
  }
  
  for (n = i = 0; i < num_filters; i++)
    if (filters[i].type == CARMEN_DOT_DOOR)
      n++;

  msg.num_doors = n;
  if (n == 0)
    msg.doors = NULL;

  else {
    msg.doors = (carmen_dot_door_p)
      realloc(msg.doors, n*sizeof(carmen_dot_door_t));
    carmen_test_alloc(msg.doors);
    for (n = i = 0; i < num_filters; i++) {
      if (filters[i].type == CARMEN_DOT_DOOR) {
	msg.doors[n].id = filters[i].id;
	msg.doors[n].x = filters[i].door_filter.x;
	msg.doors[n].y = filters[i].door_filter.y;
	msg.doors[n].theta = filters[i].door_filter.t;
	msg.doors[n].vx = filters[i].door_filter.px;
	msg.doors[n].vy = filters[i].door_filter.py;
	msg.doors[n].vxy = filters[i].door_filter.pxy;
	msg.doors[n].vtheta = filters[i].door_filter.pt;
	n++;
      }
    }
  }

  msg.timestamp = carmen_get_time_ms();

  err = IPC_respondData(msgRef, CARMEN_DOT_ALL_DOORS_MSG_NAME, &msg);
  carmen_test_ipc(err, "Could not respond",
		  CARMEN_DOT_ALL_DOORS_MSG_NAME);
}

static void dot_query_handler
(MSG_INSTANCE msgRef, BYTE_ARRAY callData,
 void *clientData __attribute__ ((unused))) {

  FORMATTER_PTR formatter;
  IPC_RETURN_TYPE err;
  carmen_dot_query query;

  formatter = IPC_msgInstanceFormatter(msgRef);
  err = IPC_unmarshallData(formatter, callData, &query, sizeof(carmen_dot_query));
  IPC_freeByteArray(callData);
  carmen_test_ipc_return(err, "Could not unmarshall",
			 IPC_msgInstanceName(msgRef));

  printf("--> received query %d\n", query.type);

  switch(query.type) {
  case CARMEN_DOT_PERSON:
    respond_all_people_msg(msgRef);
    break;
  case CARMEN_DOT_TRASH:
    respond_all_trash_msg(msgRef);
    break;
  case CARMEN_DOT_DOOR:
    respond_all_doors_msg(msgRef);
    break;
  }
}

static void reset_handler
(MSG_INSTANCE msgRef __attribute__ ((unused)), BYTE_ARRAY callData,
 void *clientData __attribute__ ((unused))) {

  IPC_freeByteArray(callData);

  reset();
}

static void ipc_init() {

  IPC_RETURN_TYPE err;

  err = IPC_defineMsg(CARMEN_DOT_PERSON_MSG_NAME, IPC_VARIABLE_LENGTH, 
	 	      CARMEN_DOT_PERSON_MSG_FMT);
  carmen_test_ipc_exit(err, "Could not define", CARMEN_DOT_PERSON_MSG_NAME);

  err = IPC_defineMsg(CARMEN_DOT_TRASH_MSG_NAME, IPC_VARIABLE_LENGTH, 
	 	      CARMEN_DOT_TRASH_MSG_FMT);
  carmen_test_ipc_exit(err, "Could not define", CARMEN_DOT_TRASH_MSG_NAME);

  err = IPC_defineMsg(CARMEN_DOT_DOOR_MSG_NAME, IPC_VARIABLE_LENGTH, 
	 	      CARMEN_DOT_DOOR_MSG_FMT);
  carmen_test_ipc_exit(err, "Could not define", CARMEN_DOT_DOOR_MSG_NAME);

  err = IPC_defineMsg(CARMEN_DOT_ALL_PEOPLE_MSG_NAME, IPC_VARIABLE_LENGTH, 
	 	      CARMEN_DOT_ALL_PEOPLE_MSG_FMT);
  carmen_test_ipc_exit(err, "Could not define", CARMEN_DOT_ALL_PEOPLE_MSG_NAME);

  err = IPC_defineMsg(CARMEN_DOT_ALL_TRASH_MSG_NAME, IPC_VARIABLE_LENGTH, 
	 	      CARMEN_DOT_ALL_TRASH_MSG_FMT);
  carmen_test_ipc_exit(err, "Could not define", CARMEN_DOT_ALL_TRASH_MSG_NAME);

  err = IPC_defineMsg(CARMEN_DOT_ALL_DOORS_MSG_NAME, IPC_VARIABLE_LENGTH, 
	 	      CARMEN_DOT_ALL_DOORS_MSG_FMT);
  carmen_test_ipc_exit(err, "Could not define", CARMEN_DOT_ALL_DOORS_MSG_NAME);

  err = IPC_subscribe(CARMEN_DOT_QUERY_NAME, dot_query_handler, NULL);
  carmen_test_ipc_exit(err, "Could not subcribe", CARMEN_DOT_QUERY_NAME);
  IPC_setMsgQueueLength(CARMEN_DOT_QUERY_NAME, 100);

  err = IPC_subscribe(CARMEN_DOT_RESET_MSG_NAME, reset_handler, NULL);
  carmen_test_ipc_exit(err, "Could not subcribe", CARMEN_DOT_RESET_MSG_NAME);
  IPC_setMsgQueueLength(CARMEN_DOT_RESET_MSG_NAME, 100);

  carmen_robot_subscribe_frontlaser_message(&laser_msg,
					    (carmen_handler_t)laser_handler,
					    CARMEN_SUBSCRIBE_LATEST);
  carmen_localize_subscribe_globalpos_message(&odom, NULL,
					      CARMEN_SUBSCRIBE_LATEST);
}

static void params_init(int argc, char *argv[]) {

  carmen_param_t param_list[] = {
    {"robot", "frontlaser_offset", CARMEN_PARAM_DOUBLE, 
     &localize_params.front_laser_offset, 0, NULL},
    {"robot", "rearlaser_offset", CARMEN_PARAM_DOUBLE, 
     &localize_params.rear_laser_offset, 0, NULL},
    {"localize", "num_particles", CARMEN_PARAM_INT, 
     &localize_params.num_particles, 0, NULL},
    {"localize", "max_range", CARMEN_PARAM_DOUBLE, &localize_params.max_range, 1, NULL},
    {"localize", "min_wall_prob", CARMEN_PARAM_DOUBLE, 
     &localize_params.min_wall_prob, 0, NULL},
    {"localize", "outlier_fraction", CARMEN_PARAM_DOUBLE, 
     &localize_params.outlier_fraction, 0, NULL},
    {"localize", "update_distance", CARMEN_PARAM_DOUBLE, 
     &localize_params.update_distance, 0, NULL},
    {"localize", "laser_skip", CARMEN_PARAM_INT, &localize_params.laser_skip, 1, NULL},
    {"localize", "use_rear_laser", CARMEN_PARAM_ONOFF, 
     &localize_params.use_rear_laser, 0, NULL},
    {"localize", "do_scanmatching", CARMEN_PARAM_ONOFF,
     &localize_params.do_scanmatching, 1, NULL},
    {"localize", "constrain_to_map", CARMEN_PARAM_ONOFF, 
     &localize_params.constrain_to_map, 1, NULL},
    {"localize", "odom_a1", CARMEN_PARAM_DOUBLE, &localize_params.odom_a1, 1, NULL},
    {"localize", "odom_a2", CARMEN_PARAM_DOUBLE, &localize_params.odom_a2, 1, NULL},
    {"localize", "odom_a3", CARMEN_PARAM_DOUBLE, &localize_params.odom_a3, 1, NULL},
    {"localize", "odom_a4", CARMEN_PARAM_DOUBLE, &localize_params.odom_a4, 1, NULL},
    {"localize", "occupied_prob", CARMEN_PARAM_DOUBLE, 
     &localize_params.occupied_prob, 0, NULL},
    {"localize", "lmap_std", CARMEN_PARAM_DOUBLE, 
     &localize_params.lmap_std, 0, NULL},
    {"localize", "global_lmap_std", CARMEN_PARAM_DOUBLE, 
     &localize_params.global_lmap_std, 0, NULL},
    {"localize", "global_evidence_weight", CARMEN_PARAM_DOUBLE, 
     &localize_params.global_evidence_weight, 0, NULL},
    {"localize", "global_distance_threshold", CARMEN_PARAM_DOUBLE, 
     &localize_params.global_distance_threshold, 1, NULL},
    {"localize", "global_test_samples", CARMEN_PARAM_INT,
     &localize_params.global_test_samples, 1, NULL},
    {"localize", "use_sensor", CARMEN_PARAM_ONOFF,
     &localize_params.use_sensor, 0, NULL},

    {"dot", "person_filter_a", CARMEN_PARAM_DOUBLE, &default_person_filter_a, 1, NULL},
    {"dot", "person_filter_px", CARMEN_PARAM_DOUBLE, &default_person_filter_px, 1, NULL},
    {"dot", "person_filter_py", CARMEN_PARAM_DOUBLE, &default_person_filter_py, 1, NULL},
    {"dot", "person_filter_pxy", CARMEN_PARAM_DOUBLE, &default_person_filter_pxy, 1, NULL},
    {"dot", "person_filter_qx", CARMEN_PARAM_DOUBLE, &default_person_filter_qx, 1, NULL},
    {"dot", "person_filter_qy", CARMEN_PARAM_DOUBLE, &default_person_filter_qy, 1, NULL},
    {"dot", "person_filter_qxy", CARMEN_PARAM_DOUBLE, &default_person_filter_qxy, 1, NULL},
    {"dot", "person_filter_qlogr", CARMEN_PARAM_DOUBLE, &default_person_filter_qlogr, 1, NULL},
    {"dot", "person_filter_rx", CARMEN_PARAM_DOUBLE, &default_person_filter_rx, 1, NULL},
    {"dot", "person_filter_ry", CARMEN_PARAM_DOUBLE, &default_person_filter_ry, 1, NULL},
    {"dot", "person_filter_rxy", CARMEN_PARAM_DOUBLE, &default_person_filter_rxy, 1, NULL},
    {"dot", "trash_filter_px", CARMEN_PARAM_DOUBLE, &default_trash_filter_px, 1, NULL},
    {"dot", "trash_filter_py", CARMEN_PARAM_DOUBLE, &default_trash_filter_py, 1, NULL},
    {"dot", "trash_filter_a", CARMEN_PARAM_DOUBLE, &default_trash_filter_a, 1, NULL},
    {"dot", "trash_filter_qx", CARMEN_PARAM_DOUBLE, &default_trash_filter_qx, 1, NULL},
    {"dot", "trash_filter_qy", CARMEN_PARAM_DOUBLE, &default_trash_filter_qy, 1, NULL},
    {"dot", "trash_filter_qxy", CARMEN_PARAM_DOUBLE, &default_trash_filter_qxy, 1, NULL},
    {"dot", "trash_filter_qtheta", CARMEN_PARAM_DOUBLE, &default_trash_filter_qtheta, 1, NULL},
    {"dot", "trash_filter_qlogw1", CARMEN_PARAM_DOUBLE, &default_trash_filter_qlogw1, 1, NULL},
    {"dot", "trash_filter_qlogw2", CARMEN_PARAM_DOUBLE, &default_trash_filter_qlogw2, 1, NULL},
    {"dot", "trash_filter_rx", CARMEN_PARAM_DOUBLE, &default_trash_filter_rx, 1, NULL},
    {"dot", "trash_filter_ry", CARMEN_PARAM_DOUBLE, &default_trash_filter_ry, 1, NULL},
    {"dot", "trash_filter_rxy", CARMEN_PARAM_DOUBLE, &default_trash_filter_rxy, 1, NULL},
    {"dot", "door_filter_px", CARMEN_PARAM_DOUBLE, &default_door_filter_px, 1, NULL},
    {"dot", "door_filter_py", CARMEN_PARAM_DOUBLE, &default_door_filter_py, 1, NULL},
    {"dot", "door_filter_pt", CARMEN_PARAM_DOUBLE, &default_door_filter_pt, 1, NULL},
    {"dot", "door_filter_a", CARMEN_PARAM_DOUBLE, &default_door_filter_a, 1, NULL},
    {"dot", "door_filter_qx", CARMEN_PARAM_DOUBLE, &default_door_filter_qx, 1, NULL},
    {"dot", "door_filter_qy", CARMEN_PARAM_DOUBLE, &default_door_filter_qy, 1, NULL},
    {"dot", "door_filter_qxy", CARMEN_PARAM_DOUBLE, &default_door_filter_qxy, 1, NULL},
    {"dot", "door_filter_qt", CARMEN_PARAM_DOUBLE, &default_door_filter_qt, 1, NULL},
    {"dot", "door_filter_rx", CARMEN_PARAM_DOUBLE, &default_door_filter_rx, 1, NULL},
    {"dot", "door_filter_ry", CARMEN_PARAM_DOUBLE, &default_door_filter_ry, 1, NULL},
    {"dot", "door_filter_rt", CARMEN_PARAM_DOUBLE, &default_door_filter_rt, 1, NULL},
    {"dot", "new_filter_threshold", CARMEN_PARAM_INT, &new_filter_threshold, 1, NULL},
    {"dot", "new_cluster_threshold", CARMEN_PARAM_DOUBLE, &new_cluster_threshold, 1, NULL},
    {"dot", "map_diff_threshold", CARMEN_PARAM_DOUBLE, &map_diff_threshold, 1, NULL},
    {"dot", "map_diff_threshold_scalar", CARMEN_PARAM_DOUBLE,
     &map_diff_threshold_scalar, 1, NULL},
    {"dot", "map_occupied_threshold", CARMEN_PARAM_DOUBLE, &map_occupied_threshold, 1, NULL},
    {"dot", "person_filter_displacement_threshold", CARMEN_PARAM_DOUBLE,
     &person_filter_displacement_threshold, 1, NULL},
    {"dot", "kill_hidden_person_cnt", CARMEN_PARAM_INT, &kill_hidden_person_cnt, 1, NULL},
    {"dot", "sensor_update_dist", CARMEN_PARAM_DOUBLE, &sensor_update_dist, 1, NULL},
    {"dot", "sensor_update_cnt", CARMEN_PARAM_INT, &sensor_update_cnt, 1, NULL},
    {"dot", "laser_max_range", CARMEN_PARAM_DOUBLE, &laser_max_range, 1, NULL},
    {"dot", "see_through_stdev", CARMEN_PARAM_DOUBLE, &see_through_stdev, 1, NULL},
    {"dot", "trace_resolution", CARMEN_PARAM_DOUBLE, &trace_resolution, 1, NULL},
    {"dot", "invisible_cnt", CARMEN_PARAM_INT, &invisible_cnt, 1, NULL}
  };

  carmen_param_install_params(argc, argv, param_list,
	 		      sizeof(param_list) / sizeof(param_list[0]));
}

void shutdown_module(int sig) {

  sig = 0;

  exit(0);
}

int main(int argc, char *argv[]) {

  carmen_initialize_ipc(argv[0]);
  carmen_param_check_version(argv[0]);
  carmen_randomize(&argc, &argv);

  signal(SIGINT, shutdown_module);

  odom.timestamp = 0.0;
  //static_map.map = NULL;
  printf("getting gridmap...");
  fflush(0);
  carmen_map_get_gridmap(&static_map);
  printf("done\n");

  params_init(argc, argv);
  ipc_init();

  printf("occupied_prob = %f\n", localize_params.occupied_prob);
  printf("getting localize map...");
  fflush(0);
  carmen_to_localize_map(&static_map, &localize_map, &localize_params);
  printf("done\n");

#ifdef HAVE_GRAPHICS
  gtk_init(&argc, &argv);
  gui_init();
  carmen_graphics_update_ipc_callbacks((GdkInputFunction) updateIPC);
  gtk_main();
#else
  IPC_dispatch();
#endif
  
  return 0;
}
