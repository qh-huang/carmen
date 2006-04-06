/*********************************************************
 *
 * This source code is part of the Carnegie Mellon Robot
 * Navigation Toolkit (CARMEN)
 *
 * CARMEN Copyright (c) 2002 Michael Montemerlo, Nicholas
 * Roy, Sebastian Thrun, Dirk Haehnel, Cyrill Stachniss,
 * and Jared Glover
 *
 * CARMEN is free software; you can redistribute it and/or 
 * modify it under the terms of the GNU General Public 
 * License as published by the Free Software Foundation; 
 * either version 2 of the License, or (at your option)
 * any later version.
 *
 * CARMEN is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied 
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 * PURPOSE.  See the GNU General Public License for more 
 * details.
 *
 * You should have received a copy of the GNU General 
 * Public License along with CARMEN; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place, 
 * Suite 330, Boston, MA  02111-1307 USA
 *
 ********************************************************/

/***********************************
 * map_editor_graphics deals with  *
 * the pixmaps and the main window *
 * which display the map.          *
 ***********************************/
//#include <gnome.h>

// This chunk of code is ripped from global_graphics.c and 
// global_graphics.h. Because those files have been upgraded
// to GTK 2.0, they can't be included here.

#include <gtk/gtk.h>
#include <gdk_imlib.h>
#include <carmen/carmen.h>

#define CARMEN_GRAPHICS_INVERT          1
#define CARMEN_GRAPHICS_RESCALE         2
#define CARMEN_GRAPHICS_ROTATE          4
#define CARMEN_GRAPHICS_BLACK_AND_WHITE 8

unsigned char *carmen_graphics_convert_to_image(carmen_map_p map, int flags) 
{
  register float *data_ptr;
  unsigned char *image_data = NULL;
  register unsigned char *image_ptr = NULL;
  double value;
  int x_size, y_size;
  int x_index, y_index;
  int index;
  double max_val = -MAXDOUBLE, min_val = MAXDOUBLE;

  int rescale = flags & CARMEN_GRAPHICS_RESCALE;
  int invert = flags & CARMEN_GRAPHICS_INVERT;
  int rotate = flags & CARMEN_GRAPHICS_ROTATE;
  int black_and_white = flags & CARMEN_GRAPHICS_BLACK_AND_WHITE;

  if (map == NULL) {
    carmen_warn("carmen_graphics_convert_to_image was passed NULL map.\n");
    return NULL;
  }

  x_size = map->config.x_size;
  y_size = map->config.y_size;
  image_data = (unsigned char *)calloc(x_size*y_size*3, sizeof(unsigned char));
  carmen_test_alloc(image_data);

  if (rescale) {
    max_val = -MAXDOUBLE;
    min_val = MAXDOUBLE;
    data_ptr = map->complete_map;
    for (index = 0; index < map->config.x_size*map->config.y_size; index++) {
      max_val = carmen_fmax(max_val, *data_ptr);
      if (*data_ptr >= 0)
	min_val = carmen_fmin(min_val, *data_ptr);
      data_ptr++;
    }
  }

  if (max_val < 0)
    rescale = 0;
  
  image_ptr = image_data;
  data_ptr = map->complete_map;
  for (x_index = 0; x_index < x_size; x_index++) {
    for (y_index = 0; y_index < y_size; y_index++) {
      value = *(data_ptr++);	    
      if (rotate)
	image_ptr = image_data+y_index*x_size*3+x_index;
      if (value < 0) {
	if (black_and_white) {
	  *(image_ptr++) = 255;
	  *(image_ptr++) = 255;
	  *(image_ptr++) = 255;
	} else {
	  *(image_ptr++) = 0;
	  *(image_ptr++) = 0;
	  *(image_ptr++) = 255;
	}
      } else if(!rescale && value > 1.0) {
	if (black_and_white) {
	  *(image_ptr++) = 128;
	  *(image_ptr++) = 128;
	  *(image_ptr++) = 128;
	} else {
	  *(image_ptr++) = 255;
	  *(image_ptr++) = 0;
	  *(image_ptr++) = 0;
	}
      } else {
	if (rescale)
	  value = (value - min_val) / (max_val - min_val);
	if (!invert)
	  value = 1 - value;
	for (index = 0; index < 3; index++)
	  *(image_ptr++) = value * 255;
      }
    }
  }
  return image_data;
}

static GdkColormap *cmap = NULL;

GdkColor carmen_red, carmen_blue, carmen_white, carmen_yellow, 
  carmen_green, carmen_light_blue, carmen_black, carmen_orange, 
  carmen_grey, carmen_light_grey, carmen_purple;


static void _add_color(GdkColor *color, char *name)
{
  if(cmap == NULL)
    cmap = gdk_colormap_get_system();

  if (!gdk_color_parse (name, color)) {
    g_error("couldn't parse color");
    return;
  }

  if(!gdk_colormap_alloc_color(cmap, color, FALSE, TRUE))
    g_error("couldn't allocate color");
}

void
carmen_graphics_setup_colors(void)
{
  _add_color(&carmen_red, "red");
  _add_color(&carmen_blue, "blue");
  _add_color(&carmen_white, "white");
  _add_color(&carmen_yellow, "yellow");
  _add_color(&carmen_green, "green");
  _add_color(&carmen_light_blue, "DodgerBlue");
  _add_color(&carmen_black, "black");
  _add_color(&carmen_orange, "tomato");
  _add_color(&carmen_grey, "ivory4");
  _add_color(&carmen_light_grey, "grey79");
  _add_color(&carmen_purple, "purple");
}

GdkColor carmen_graphics_add_color(char *name) 
{
  GdkColor color;

  _add_color(&color, name);
  return color;
}

GdkColor carmen_graphics_add_color_rgb(int r, int g, int b) 
{
  GdkColor color;

  if(cmap == NULL)
    cmap = gdk_colormap_get_system();

  color.red = r * 256;
  color.green = g * 256;
  color.blue = b * 256;

  if(!gdk_colormap_alloc_color(cmap, &color, FALSE, TRUE))
    g_error("couldn't allocate color");

  return color;
}

GdkPixmap * 
carmen_graphics_generate_pixmap(GtkWidget* drawing_area, unsigned char* image_data,
				carmen_map_config_p config, double zoom) 
{
  GdkImlibImage *image;
  GdkPixmap *pixmap;

  if (drawing_area == NULL || image_data == NULL) 
    {
      carmen_warn("carmen_graphics_generate_pixmap was passed bad arguments.\n");
      return NULL;
    }

  image =  gdk_imlib_create_image_from_data
    (image_data, (unsigned char *)NULL, config->y_size, config->x_size);
  
  gdk_imlib_rotate_image(image, -1);
  gdk_imlib_flip_image_vertical(image);
  gdk_imlib_render(image, config->x_size*zoom, config->y_size*zoom);

  pixmap = gdk_imlib_move_image(image);

  gdk_imlib_kill_image(image);  

  return pixmap;
}



#include "map_editor.h"
#include "map_editor_graphics.h"
#include "map_editor_drawing.h"
#include "map_editor_menus.h"

#define DEFAULT_MAX_SCREEN_WIDTH 600
#define DEFAULT_MAX_SCREEN_HEIGHT 400

static GtkObject *x_adjustment, *y_adjustment;
static GtkWidget *drawing_table, *window_box;

static GdkFont *place_font;
static int current_place = -1;
GtkItemFactory *item_factory;

/*****************************************************************
 * Conversions from the coordinates on the screen to the         *
 * coordinates on the map and vis versa                          *
 *****************************************************************/

/* converts an x position on the drawing area to an x grid
   on the map */
inline double 
pix_x_to_map(double pix_x)
{
  return (double)(pix_x)/mult + (double)xstart;
}

/* converts a y position on the drawing area to a y grid
   on the map */
inline double 
pix_y_to_map(double pix_y)
{
  return (double)yend - (double)(pix_y)/mult;
}
 
/* converts an x grid to an x position on the drawing area */
inline double 
map_x_to_pix(int map_x)
{
  return (double)(map_x-xstart)*mult;
}

/* converts a y grid to a y position on the drawing area */
inline double 
map_y_to_pix(int map_y)
{
  return (double)(yend-map_y)*mult;
}

/* converts an x grid to a position on the map_pixmap */
inline double 
map_x_to_map_pix(int map_x)
{
  return (double)(map_x)*mult;
}

/* converts an x grid to a position on the map_pixmap */
inline double 
map_y_to_map_pix(int map_y)
{
  return (double)(map->config.y_size-map_y)*mult;
}


/*****************************************************************
 * Grafics helper functions                                      *
 *****************************************************************/

void 
draw_offlimits(int i)
{
  int pix_x1, pix_y1, pix_x2, pix_y2;

  pix_x1 = map_x_to_pix(offlimits_array[i].x1);
  pix_y1 = map_y_to_pix(offlimits_array[i].y1);
  pix_x2 = map_x_to_pix(offlimits_array[i].x2);
  pix_y2 = map_y_to_pix(offlimits_array[i].y2);
	
  switch (offlimits_array[i].type) {
  case CARMEN_OFFLIMITS_POINT_ID:
    break;
  case CARMEN_OFFLIMITS_LINE_ID:
    gdk_draw_line (map_pixmap, Drawing_GC, pix_x1, pix_y1, pix_x2, 
		   pix_y2);
    break;
  case CARMEN_OFFLIMITS_RECT_ID:
    gdk_draw_rectangle(map_pixmap, Drawing_GC, 1, pix_x1, pix_y2, 
		       pix_x2-pix_x1, pix_y1-pix_y2);
    break;
  default:
    break;
  }
}

void 
draw_place_name(int i, GdkPixmap *pixmap, GdkColor *draw_color)
{
  int pix_x1, pix_y1;

  pix_x1 = map_x_to_pix(place_list->places[i].x/map->config.resolution);
  pix_y1 = map_y_to_pix(place_list->places[i].y/map->config.resolution);
	
  gdk_gc_set_foreground (Drawing_GC, draw_color);

  gdk_draw_arc(pixmap, Drawing_GC, 1, pix_x1-5, pix_y1-5,
	       10, 10, 0, 360*64);

  gdk_gc_set_foreground (Drawing_GC, &carmen_black);

  gdk_draw_arc(pixmap, Drawing_GC, 0, pix_x1-5, pix_y1-5,
	       10, 10, 0, 360*64); 

  gdk_gc_set_foreground (Drawing_GC, &carmen_black);
  gdk_draw_string(pixmap, place_font, Drawing_GC, pix_x1, pix_y1-5,
		  place_list->places[i].name);
}

/* draws the map_pixmap onto the tmp_pixmap */
void 
map_to_tmp(void)
{
  
  int xsrc, ysrc, xdest, ydest, w,h;
  xsrc = 0;
  xdest = 0;
  w = xend*mult;
  ysrc = 0;
  ydest = 0;
  h = yend*mult;

  gdk_gc_set_foreground(Drawing_GC, &purple);
  gdk_draw_rectangle(tmp_pixmap, Drawing_GC, TRUE, 0, 0,
		     drawing_area->allocation.width,
		     drawing_area->allocation.height);

  gdk_draw_pixmap(tmp_pixmap, 
		  drawing_area->style->fg_gc[GTK_WIDGET_STATE (drawing_area)],
		  map_pixmap, xsrc, ysrc, xdest, ydest, w, h);
}


/* sets up the default colors and Drawing_GC */
void 
setup_colors(void)
{
  if (Drawing_GC == NULL) 
    Drawing_GC = gdk_gc_new(drawing_area->window);
  if (Drawing_GC == NULL)	
    carmen_die("Drawing_GC could not be initialized\n");

  yellow = carmen_graphics_add_color("Yellow");
  blue = carmen_graphics_add_color_rgb(0, 0, 255);
  red = carmen_graphics_add_color_rgb(255, 0, 0);
  purple = carmen_graphics_add_color_rgb(150, 0, 150);
}

/* redraws the pixmap onto the drawing area */
void 
redraw(void)
{
  int i;

  if(map_pixmap == NULL)
    {
      if (image_data != NULL)
	free(image_data);
      setup_colors();
      image_data = carmen_graphics_convert_to_image(map, 0);
      map_pixmap = carmen_graphics_generate_pixmap(drawing_area, image_data, 
						   &(map->config), mult);

      if (show_offlimits) 
	{
	  gdk_gc_set_foreground (Drawing_GC, &red);
	  for (i = 0; i < num_offlimits_segments; i++) 
	    draw_offlimits(i);
	}

      if (show_place_names && place_list)
	{
	  for (i = 0; i < place_list->num_places; i++) 
	    draw_place_name(i, map_pixmap, &carmen_red);
	}

      tmp_pixmap = gdk_pixmap_new(drawing_area->window,
				  drawing_area->allocation.width,
				  drawing_area->allocation.height,
				  -1);
      map_to_tmp();
    }	
  
  gdk_draw_pixmap(drawing_area->window,   //dest
		  drawing_area->style->fg_gc[GTK_WIDGET_STATE (drawing_area)],
		  tmp_pixmap,             //src
		  0, 0, 0, 0, -1, -1);

}

/***************************************************************************
 * Event handlers for the drawing area                                     *
 ***************************************************************************/

/* handles a button press event in the drawing area.
   calls the apropriate tool depending upon utensil's setting 
   also creates a backup for undo and incrememnts modified if necessary */
static gint 
button_press_event( GtkWidget *widget, GdkEventButton *event )
{  
  if(map_pixmap == NULL)
    return -1;

  if (deleting_placename)
    return TRUE;

  if (adding_placename)
    return TRUE;

  if (adding_door)
    return TRUE;

  if (event->button == 1)
    {
      switch(utensil)
	{
	case CARMEN_MAP_BRUSH:
	  memcpy(backup, map->complete_map, sizeof(float) * map->config.x_size * 
		 map->config.y_size); 
	  modified ++;
	  draw_brush(widget, event->x, event->y);
	  break;
	case CARMEN_MAP_RECTANGLE:
	  memcpy(backup, map->complete_map, sizeof(float) * map->config.x_size * 
		 map->config.y_size);
	  modified ++;
	  creating_rectangle(widget, 0, event->x, event->y);
	  break;
	case CARMEN_MAP_CROP:
	  memcpy(backup, map->complete_map, sizeof(float) * map->config.x_size * 
		 map->config.y_size);
	  modified ++;
	  cropping(widget, 0, event->x, event->y);
	  break;
	case CARMEN_MAP_LINE:
	  memcpy(backup, map->complete_map, sizeof(float) * map->config.x_size * 
		 map->config.y_size);
	  modified ++;
	  creating_line(0, event->x, event->y);
	  break;
	case CARMEN_MAP_FILL:
	  memcpy(backup, map->complete_map, sizeof(float) * map->config.x_size * 
		 map->config.y_size);
	  modified ++;
	  create_fill(event->x, event->y);
	  break;
	case CARMEN_MAP_FUZZY_FILL:
	  memcpy(backup, map->complete_map, sizeof(float) * map->config.x_size * 
		 map->config.y_size);
	  modified ++;
	  create_fuzzy_fill(event->x, event->y);
	  break;
	case CARMEN_MAP_EYEDROPPER:
	  sample(event->x, event->y);
	  break;
	case CARMEN_MAP_ZOOM:
	  zoom_in(event->x, event->y);
	  break;
	case CARMEN_MAP_MOVER:
	  move(0, event->x, event->y);
	default:
	  break;
	}
    }
  else if (event->button == 3 && utensil == CARMEN_MAP_ZOOM)
    zoom_out(event->x, event->y);

  redraw();
  return TRUE;
}

/* handles the mouse button being released inside the drawing area 
   calls the apropriate tool function, or none */
static gint 
button_release_event(GtkWidget *widget, GdkEventButton *event )
{
  double x, y;

  if (deleting_placename) {
    do_delete_placename(current_place);
    return TRUE;
  }
  if (adding_placename) {
    x = pix_x_to_map(event->x)*map->config.resolution;
    y = pix_y_to_map(event->y)*map->config.resolution;
    start_add_placename(x, y);
    return TRUE;
  }
  if (adding_door) {
    x = pix_x_to_map(event->x)*map->config.resolution;
    y = pix_y_to_map(event->y)*map->config.resolution;
    if (adding_door == 2)
      start_add_door(x, y);
    else
      finish_add_door(x, y);
    return TRUE;
  }

  if (event->button == 1 && map_pixmap != NULL)
    switch(utensil)
      {
      case CARMEN_MAP_RECTANGLE:
	creating_rectangle(widget, 2, event->x, event->y);
	break;
      case CARMEN_MAP_CROP:
	cropping(widget, 2, event->x, event->y);
	break;
      case CARMEN_MAP_LINE:
	creating_line(2, event->x, event->y);
	break;
      case CARMEN_MAP_MOVER:
	move(2, event->x, event->y);
      default:
	break;
      }

  redraw();
  return 1;
}

static void
handle_deleting_placename_move(int map_x, int map_y)
{
  double closest_distance;
  int closest_place;

  double distance;
  int i;

  closest_distance = hypot(map_x-place_list->places[0].x/
			   map->config.resolution,
			   map_y-place_list->places[0].y/
			   map->config.resolution);
  closest_place = 0;
  for (i = 0; i < place_list->num_places; i++) 
    {
      distance = hypot(map_x-place_list->places[i].x/
		       map->config.resolution,
		       map_y-place_list->places[i].y/
		       map->config.resolution);
      if (distance < closest_distance) 
	{
	  closest_distance = distance;
	  closest_place = i;
	}
    }

  if (closest_place != current_place && current_place >= 0)
    draw_place_name(current_place, tmp_pixmap, &carmen_red);

  current_place = closest_place;
  draw_place_name(closest_place, tmp_pixmap, &carmen_yellow);

}


/* handles the mouse moving in the drawing area
   only calls the appropriate tool function if the mouse button is down.
   always sets the cursor to the appropriate symbol */
static gint 
motion_notify_event( GtkWidget *widget, GdkEventMotion *event )
{
  int x, y;
  GdkModifierType state;
  static char label_buffer[255];
  static carmen_map_point_t map_point;
  static carmen_world_point_t world_point;
	

  GTK_WIDGET_CLASS(GTK_OBJECT(vrule)->klass)->motion_notify_event(vrule, event);
  GTK_WIDGET_CLASS(GTK_OBJECT(hrule)->klass)->motion_notify_event(hrule, event);

  //   map_to_tmp();

  if (event->is_hint)
    gdk_window_get_pointer (event->window, &x, &y, &state);
  else
    {
      x = event->x;
      y = event->y;
      state = event->state;
    }
	
  map_point.x = pix_x_to_map(x);
  map_point.y = pix_y_to_map(y);
  map_point.map = map;
  carmen_map_to_world(&map_point, &world_point);
  sprintf(label_buffer, "X: %6.2f Y: %6.2f", world_point.pose.x, 
	  world_point.pose.y);
  gtk_label_set_text(GTK_LABEL(coords_label), label_buffer);

  if (map_pixmap == NULL)
    return TRUE;

  if (deleting_placename) 
    {
      handle_deleting_placename_move(map_point.x, map_point.y);
    }
  else if ((state & GDK_BUTTON1_MASK)) 
    {
      switch(utensil)
	{
	case CARMEN_MAP_BRUSH:
	  draw_brush (widget, x, y);
	  x = x/mult - brush_size + .5;
	  y = y/mult - brush_size + .5;
	  break;
	case CARMEN_MAP_RECTANGLE:
	  creating_rectangle(widget, 1, x, y);
	  break;
	case CARMEN_MAP_CROP:
	  cropping(widget, 1, x, y);
	  break;
	case CARMEN_MAP_LINE:
	  creating_line(1, x, y);
	  break;
	case CARMEN_MAP_EYEDROPPER:
	  sample(x, y);
	  break;
	case CARMEN_MAP_MOVER:
	  move(1, x, y);
	default:
	  break;
	}
    }

  redraw();
  
  return TRUE;
}

static gint
configure_event (GtkWidget *widget __attribute__ ((unused)),
		 GdkEventConfigure *event __attribute__ ((unused)))
{
  GtkAdjustment *adjustment;
  int width, height, x, y;
  width = event->width;
  height = event->height;
  x = event->x;
  y = event->y;

  adjustment = gtk_scrolled_window_get_hadjustment((GtkScrolledWindow *)scrolled_window);
  adjustment->lower = xstart*mult;
  adjustment->upper = xend*mult;

  adjustment->page_increment = (xend-xstart)*mult;
  gtk_signal_emit_by_name (GTK_OBJECT (adjustment), "value_changed");
  gtk_signal_emit_by_name (GTK_OBJECT (adjustment), "changed");

  //  gtk_ruler_set_range (GTK_RULER (hrule), xstart*mult, xend*mult, xstart*mult, xend*mult);

  adjustment = gtk_scrolled_window_get_vadjustment((GtkScrolledWindow *)scrolled_window);
  adjustment->upper = (map->config.y_size - ystart)*mult;
  adjustment->lower = (map->config.y_size - yend)*mult;

  adjustment->page_increment = (yend-ystart)*mult;
  gtk_signal_emit_by_name (GTK_OBJECT (adjustment), "value_changed");
  gtk_signal_emit_by_name (GTK_OBJECT (adjustment), "changed");

if(map_pixmap)
    {
      gdk_pixmap_unref(map_pixmap);
      gdk_pixmap_unref(tmp_pixmap);
    }
  map_pixmap = NULL;
  tmp_pixmap = NULL;

  redraw();
  return FALSE;
}


static gint 
expose_event(GtkWidget *widget __attribute__ ((unused)), 
	     GdkEventExpose *event __attribute__ ((unused)))
{
  redraw();
  return FALSE;
}

/*************************************************************************
 * event handlers for the scroll bars -- DEPRICATED                      *
 *************************************************************************/
gint 
horizontal(GtkAdjustment *adj)
     /* depricated */
{
  GtkAdjustment *adjustment;
  int delta;
  int map_x;

  map_x = adj->value/mult;
  delta = (xend-xstart);
  xend = xstart + delta;
  
  gtk_ruler_set_range(GTK_RULER(hrule),
		      map->config.resolution*xstart/100.0, 
		      map->config.resolution*xend/100.0, 
		      map->config.resolution * map_x / 100.0, 
		      map->config.resolution * map->config.x_size / 100.0);
  
  
// This will not happen any more because of scrolled-window technology
  adjustment = gtk_scrolled_window_get_hadjustment((GtkScrolledWindow *)scrolled_window);
  
  if(adjustment->lower/mult < 0 && adjustment->lower/mult < xstart)
    {
      if(xstart > 0)
	adjustment->lower = 0;
      else
	adjustment->lower = xstart*mult;
      gtk_signal_emit_by_name (GTK_OBJECT (adjustment), "changed");
    }
  if(adjustment->upper/mult > map->config.x_size && adjustment->upper/mult > xend)
    {
      if(xend < map->config.x_size)
	adjustment->upper = map->config.x_size*mult;
      else
	adjustment->upper = xend*mult;
      gtk_signal_emit_by_name (GTK_OBJECT (adjustment), "changed");
    }
  

  if(tmp_pixmap)
    {
      map_to_tmp();
      redraw();
    }
  return 1;
}

gint
vertical(GtkAdjustment *adj)
     /* depricated */
{
  GtkAdjustment *adjustment;
  int delta;
  int map_y;

  map_y = map->config.y_size - adj->value/mult;
  delta = (yend-ystart);
  ystart = yend - delta;
  
  gtk_ruler_set_range(GTK_RULER(vrule),
		      map->config.resolution*yend/100.0,  
		      map->config.resolution*ystart/100.0,
		      map->config.resolution * map_y / 100.0, 
		      map->config.resolution * map->config.y_size / 100.0);
  
// This will not happen any more because of scrolled-window technology
    adjustment = gtk_scrolled_window_get_vadjustment((GtkScrolledWindow *)scrolled_window);


  if(map->config.y_size - adjustment->upper/mult < 0 && map->config.y_size - adjustment->upper/mult < ystart)
    {
      if(ystart > 0)// fprintf(stderr,"1");
      	adjustment->upper = map->config.y_size*mult;
      else //fprintf(stderr,"2");
      	adjustment->upper = (map->config.y_size - ystart)*mult;
      gtk_signal_emit_by_name (GTK_OBJECT (adjustment), "changed");
    }
  if(map->config.y_size - adjustment->lower/mult > map->config.y_size && map->config.y_size - adjustment->lower/mult > yend)
    {
      if(yend < map->config.y_size) //fprintf(stderr,"3");
      	adjustment->lower = 0;
      else //fprintf(stderr,"4");
      	adjustment->lower = (map->config.y_size - yend)*mult;
      gtk_signal_emit_by_name (GTK_OBJECT (adjustment), "changed");
    }

    if(tmp_pixmap)
    {
      map_to_tmp();
      redraw();
    }
  
  return 1;
}

/*************************************************************************
 * event handlers for the main window                                    *
 *************************************************************************/

gint 
off_pixmap(void)
{
  if(tmp_pixmap)
    {
      map_to_tmp();
      redraw();
    }
  fprintf(stderr, "off pixmap\n");
  set_point();
  return 1;
}

/*************************************************************************
 * startup                                                               *
 *************************************************************************/

static GtkItemFactoryEntry menu_items[] = {
  { "/_File",         NULL,         NULL, 0, "<Branch>" },
  { "/File/_New Map...",    "<control>N", new_map_menu, 0, NULL },
  { "/File/_Open Map...",    "<control>O", open_map_menu, 0, NULL },
  { "/File/_Save Map",    "<control>S", save_map_menu, 0, NULL },
  { "/File/Save Map _As...", NULL,         save_map_as_menu, 0, NULL },
  { "/File/_Export...", NULL,         NULL, 0, NULL },
  { "/File/sep1",    NULL,        NULL, 0, "<Separator>" },
  { "/File/_Quit",    "<control>Q", quit_menu, 0, NULL },
  { "/_Edit",         NULL,         NULL, 0, "<Branch>" },
  { "/Edit/_Undo",    "<control>Z",         undo_menu, 0, NULL },
  { "/Edit/sep1",    NULL,         NULL, 0, "<Separator>" },
  //	{ "/Edit/_Minimize",    NULL,         minimize_map, 0, NULL },
  //	{ "/Edit/sep1",    NULL,         NULL, 0, "<Separator>" },
  { "/Edit/_Add Placename", NULL,  add_placename, 0, NULL },
  { "/Edit/_Delete Placename", NULL, delete_placename, 0, NULL },
  { "/Edit/sep2",    NULL,         NULL, 0, "<Separator>" },
  { "/Edit/Add Doo_r", NULL,  add_door, 0, NULL },
  { "/_View",         NULL,         NULL, 0, "<Branch>" },
  { "/View/Show _Placenames", NULL, toggle_view, 1, "<ToggleItem>" },
  { "/View/Show _Offlimits",  NULL, toggle_view, 2, "<ToggleItem>" },
  { "/Help",         NULL,         NULL, 0, "<LastBranch>" },
  { "/Help/About",   NULL,         help_menu, 0, NULL },
};

void get_main_menu(GtkWidget  *the_window, GtkWidget **menubar)
{
  GtkAccelGroup *accel_group;
  gint nmenu_items = sizeof(menu_items) / sizeof(menu_items[0]);
  GtkWidget *menu_item;

  accel_group = gtk_accel_group_new();
  item_factory = gtk_item_factory_new(GTK_TYPE_MENU_BAR, "<main>",
				      accel_group);
  gtk_item_factory_create_items(item_factory, nmenu_items, menu_items, NULL);
  gtk_window_add_accel_group(GTK_WINDOW (the_window), accel_group);
  if(menubar)
    *menubar = gtk_item_factory_get_widget(item_factory, "<main>");

  menu_item = gtk_item_factory_get_item(item_factory, "/View/Show Placenames");
  ((struct _GtkCheckMenuItem *)menu_item)->active = show_place_names;

  menu_item = gtk_item_factory_get_item(item_factory, "/View/Show Offlimits");
  ((struct _GtkCheckMenuItem *)menu_item)->active = show_offlimits;
}

void 
set_up_map_widgets(void)
{ 
  GtkAdjustment *adjustment;

  if (!map)
    return;

  gtk_widget_hide(window);

  xend = map->config.x_size;
  yend = map->config.y_size;

  if(map->config.x_size > map->config.y_size) {
    screen_width = DEFAULT_MAX_SCREEN_WIDTH;
    mult = (double)screen_width / (double)map->config.x_size;
    screen_height = (double)map->config.y_size * (double)mult;
  }
  else {
    screen_height = DEFAULT_MAX_SCREEN_HEIGHT;
    mult = (double)screen_height / (double)map->config.y_size;
    screen_width = (double)map->config.x_size * (double)mult;
    if (screen_width < DEFAULT_MAX_SCREEN_WIDTH)
      screen_width = DEFAULT_MAX_SCREEN_WIDTH;
  }

  gtk_drawing_area_size (GTK_DRAWING_AREA(drawing_area), screen_width, 
			 screen_height);
  /*
  gtk_ruler_set_range(GTK_RULER(hrule),
		      map->config.resolution*xstart/1.0, 
		      map->config.resolution*xend/1.0,  
		      0,
		      map->config.resolution*map->config.x_size / 1.0);
  gtk_ruler_set_range(GTK_RULER(vrule),
		      map->config.resolution*yend/100.0,  
		      map->config.resolution*ystart/100.0,
		      0,
		      map->config.resolution * map->config.y_size / 100.0);
  */
  /*  gtk_ruler_set_range(GTK_RULER(hrule),
      0,drawing_area->allocation.width,0,drawing_area->allocation.width);*/
  
  adjustment = gtk_scrolled_window_get_hadjustment((GtkScrolledWindow *)scrolled_window);
  adjustment->value = 0;
  adjustment->lower = 0;
  adjustment->upper = xend-xstart;
  adjustment->step_increment = 1;
  adjustment->page_increment = xend-xstart;
  adjustment->page_size = xend-xstart;

  gtk_signal_emit_by_name (GTK_OBJECT (adjustment), "changed");

  adjustment = gtk_scrolled_window_get_vadjustment((GtkScrolledWindow *)scrolled_window);
  adjustment->value = 0;
  adjustment->lower = 0;
  adjustment->upper = yend-ystart;
  adjustment->step_increment = 1;
  adjustment->page_increment = yend-ystart;
  adjustment->page_size = yend-ystart;

  gtk_signal_emit_by_name (GTK_OBJECT (adjustment), "changed");  

  gtk_widget_show_all(window);
}

/* builds the main window and drawing area for the editor */
void 
start_drawing_window(void)
{
  GtkWidget *box2, *box3, *box4, *box5;
  GtkWidget *separator;
  GtkWidget *text;
  GtkWidget *button;
  GtkWidget *menu_bar;
  GtkObject *adjustment;
  char brush_text[10];
  char ink_text[10];
  char line_text[10];
  char fuzzyness_text[10];

  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title (GTK_WINDOW (window), "Map");
  gtk_signal_connect (GTK_OBJECT (window), "delete_event", 
		      GTK_SIGNAL_FUNC (gtk_exit), NULL);
  gtk_widget_set_events (window, GDK_POINTER_MOTION_MASK);

  window_box = gtk_vbox_new(FALSE, 0);
  gtk_container_add(GTK_CONTAINER(window), window_box);
  gtk_widget_show(window_box);

  /* create menus */

  get_main_menu(window, &menu_bar);
  gtk_box_pack_start(GTK_BOX(window_box), menu_bar, FALSE, TRUE, 0);
  gtk_widget_show(menu_bar);

  /* tools */

  box2 = gtk_hbox_new(FALSE, 2);
  gtk_box_pack_start(GTK_BOX(window_box), box2, FALSE, FALSE, 0);
  gtk_widget_show(box2);
  
  separator = gtk_vseparator_new();
  gtk_box_pack_start(GTK_BOX(box2), separator, FALSE, FALSE, 0);
  gtk_widget_show(separator);

  text = gtk_label_new(" Tools: ");
  gtk_label_set_pattern(GTK_LABEL(text), "_____ ");
  gtk_box_pack_start(GTK_BOX(box2), text, FALSE, FALSE, 0);
  gtk_widget_show(text);

  button = gtk_button_new_with_label (" brush ");
  gtk_signal_connect (GTK_OBJECT (button), "clicked", GTK_SIGNAL_FUNC (set_brush), NULL);
  gtk_box_pack_start(GTK_BOX(box2), button, FALSE, FALSE, 0);
  gtk_widget_show(button);

  button = gtk_button_new_with_label (" rectangle ");
  gtk_signal_connect (GTK_OBJECT (button), "clicked", GTK_SIGNAL_FUNC (set_rectangle), NULL);
  gtk_box_pack_start(GTK_BOX(box2), button, FALSE, FALSE, 0);
  gtk_widget_show(button);

  button = gtk_button_new_with_label (" line ");
  gtk_signal_connect (GTK_OBJECT (button), "clicked", GTK_SIGNAL_FUNC (set_line), NULL);
  gtk_box_pack_start(GTK_BOX(box2), button, FALSE, FALSE, 0);
  gtk_widget_show(button);

  button = gtk_button_new_with_label (" fill ");
  gtk_signal_connect (GTK_OBJECT (button), "clicked", GTK_SIGNAL_FUNC (set_fill), NULL);
  gtk_box_pack_start(GTK_BOX(box2), button, FALSE, FALSE, 0);
  gtk_widget_show(button);

  button = gtk_button_new_with_label (" fuzzy fill ");
  gtk_signal_connect (GTK_OBJECT (button), "clicked", GTK_SIGNAL_FUNC (set_fuzzy_fill), NULL);
  gtk_box_pack_start(GTK_BOX(box2), button, FALSE, FALSE, 0);
  gtk_widget_show(button);

  button = gtk_button_new_with_label (" eye dropper ");
  gtk_signal_connect (GTK_OBJECT (button), "clicked", GTK_SIGNAL_FUNC (set_sample), NULL);
  gtk_box_pack_start(GTK_BOX(box2), button, FALSE, FALSE, 0);
  gtk_widget_show(button);

  button = gtk_button_new_with_label (" zoom ");
  gtk_signal_connect (GTK_OBJECT (button), "clicked", GTK_SIGNAL_FUNC (set_zoom), NULL);
  gtk_box_pack_start(GTK_BOX(box2), button, FALSE, FALSE, 0);
  gtk_widget_show(button);
  
  button = gtk_button_new_with_label (" mover ");
  gtk_signal_connect (GTK_OBJECT (button), "clicked", GTK_SIGNAL_FUNC (set_mover), NULL);
  gtk_box_pack_start(GTK_BOX(box2), button, FALSE, FALSE, 0);
  gtk_widget_show(button);

  button = gtk_button_new_with_label (" crop ");
  gtk_signal_connect (GTK_OBJECT (button), "clicked", GTK_SIGNAL_FUNC (set_crop), NULL);
  gtk_box_pack_start(GTK_BOX(box2), button, FALSE, FALSE, 0);
  gtk_widget_show(button);

  cur_label_label = gtk_label_new(" current tool: ");
  gtk_box_pack_start(GTK_BOX(box2), cur_label_label, TRUE, TRUE, 0);
  gtk_widget_show(cur_label_label);
  
  tool_label = gtk_label_new("brush");
  gtk_box_pack_start(GTK_BOX(box2), tool_label, TRUE, TRUE, 0);
  gtk_widget_show(tool_label);
  set_brush();

  coords_label = gtk_label_new("X:    0 Y:    0 ");
  gtk_box_pack_start(GTK_BOX(box2), coords_label, FALSE, FALSE, 0);
  gtk_widget_show(coords_label);

  separator = gtk_hseparator_new();
  gtk_widget_set_usize(separator, 700, 15);
  gtk_box_pack_start(GTK_BOX(window_box), separator, FALSE, FALSE, 0);
  gtk_widget_show(separator);

  /* table */
  drawing_table = gtk_table_new(2,2,FALSE);
  gtk_box_pack_start(GTK_BOX(window_box), drawing_table, TRUE, TRUE, 0);

  /* (m) */
  text = gtk_label_new("m");
  gtk_table_attach(GTK_TABLE(drawing_table), text,
		   0,1,0,1,
		   GTK_SHRINK,
		   GTK_SHRINK,
                   1,1);
 
  /* Create Scrolled-Window */
  scrolled_window = gtk_scrolled_window_new (NULL,NULL);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_window),
                                  GTK_POLICY_ALWAYS, GTK_POLICY_ALWAYS);
  gtk_table_attach(GTK_TABLE(drawing_table), scrolled_window,
		   1,2,1,2,
		   GTK_EXPAND|GTK_SHRINK|GTK_FILL,
		   GTK_EXPAND|GTK_SHRINK|GTK_FILL,
                   1,1);
  gtk_widget_show (scrolled_window);

  /* Create the drawing area */

  drawing_area = gtk_drawing_area_new ();
  gtk_drawing_area_size (GTK_DRAWING_AREA (drawing_area),200,200);

  gtk_widget_set_events (drawing_area, GDK_EXPOSURE_MASK
			 | GDK_BUTTON_PRESS_MASK
			 | GDK_POINTER_MOTION_MASK
			 | GDK_BUTTON_RELEASE_MASK);

  gtk_scrolled_window_add_with_viewport (
                   GTK_SCROLLED_WINDOW (scrolled_window), drawing_area);
  gtk_widget_show (drawing_area);

  /* scale */
  hrule = gtk_hruler_new();
  vrule = gtk_vruler_new();
  gtk_ruler_set_metric( GTK_RULER(hrule), GTK_PIXELS);
  gtk_ruler_set_metric( GTK_RULER(vrule), GTK_PIXELS);

  gtk_table_attach(GTK_TABLE(drawing_table), hrule,
		   1,2,0,1,
		   GTK_FILL|GTK_EXPAND|GTK_SHRINK,
		   GTK_FILL,
                   1,1);
  gtk_table_attach(GTK_TABLE(drawing_table), vrule,
		   0,1,1,2,
		   GTK_FILL,
		   GTK_EXPAND|GTK_SHRINK|GTK_FILL,
                   1,1);

  gtk_widget_show (hrule);
  gtk_widget_show (vrule);

  /* create the scroll bars for the drawing area */

  x_adjustment = (GTK_OBJECT (gtk_scrolled_window_get_hadjustment((GtkScrolledWindow *)scrolled_window)));

  //  gtk_signal_connect (GTK_OBJECT (x_adjustment), "value_changed",
  //		      GTK_SIGNAL_FUNC (horizontal), NULL);
  //  gtk_signal_connect (GTK_OBJECT (x_adjustment), "changed",
  //		      GTK_SIGNAL_FUNC (horizontal), NULL);
  
  y_adjustment = (GTK_OBJECT (gtk_scrolled_window_get_vadjustment((GtkScrolledWindow *)scrolled_window)));

  //  gtk_signal_connect (GTK_OBJECT (y_adjustment), "value_changed",
  //		      GTK_SIGNAL_FUNC (vertical), NULL);
  // gtk_signal_connect (GTK_OBJECT (y_adjustment), "changed",
  //		      GTK_SIGNAL_FUNC (vertical), NULL);
  
  /* Signals used to handle backing pixmap */
  gtk_signal_connect (GTK_OBJECT (drawing_area), "expose_event", 
		      (GtkSignalFunc) expose_event, NULL);
  gtk_signal_connect (GTK_OBJECT (drawing_area), "button_press_event",
		      (GtkSignalFunc) button_press_event, NULL);
  gtk_signal_connect (GTK_OBJECT (drawing_area), "motion_notify_event",
		      (GtkSignalFunc) motion_notify_event, NULL);
  gtk_signal_connect (GTK_OBJECT (drawing_area), "button_release_event",
		      (GtkSignalFunc) button_release_event, NULL);
  gtk_signal_connect (GTK_OBJECT(drawing_area),"configure_event",
		      (GtkSignalFunc) configure_event, NULL); 

  separator = gtk_hseparator_new();
  gtk_box_pack_start(GTK_BOX(window_box), separator, FALSE, FALSE, 0);
  gtk_widget_show(separator);

  /* buttons */
  box2 = gtk_hbox_new(FALSE, 0);
  gtk_box_pack_start(GTK_BOX(window_box), box2, FALSE, FALSE, 0);
  gtk_widget_show(box2);

  separator = gtk_vseparator_new();
  gtk_box_pack_start(GTK_BOX(box2), separator, FALSE, FALSE, 5);
  gtk_widget_show(separator);

  /*ink */
  box3 = gtk_vbox_new(TRUE, 0);
  gtk_box_pack_start(GTK_BOX(box2), box3, TRUE, TRUE, 0);
  gtk_widget_show(box3);

  box4 = gtk_hbox_new(FALSE, 0);
  gtk_box_pack_start(GTK_BOX(box3), box4, TRUE, TRUE, 0);
  gtk_widget_show(box4);
	
  box5 = gtk_vbox_new(TRUE, 0);
  gtk_box_pack_start(GTK_BOX(box4), box5, TRUE, TRUE, 0);
  gtk_widget_show(box5);

  button = gtk_button_new_with_label ("unknown");
  gtk_signal_connect (GTK_OBJECT (button), "clicked", 
		      GTK_SIGNAL_FUNC (set_unknown), NULL);
  gtk_box_pack_start(GTK_BOX(box5), button, FALSE, TRUE, 0);
  gtk_widget_show(button);

  button = gtk_button_new_with_label ("offlimits");
  gtk_signal_connect (GTK_OBJECT (button), "clicked", 
		      GTK_SIGNAL_FUNC (set_offlimits), NULL);
  gtk_box_pack_start(GTK_BOX(box5), button, FALSE, TRUE, 0);
  gtk_widget_show(button);

  if(ink < -1) {
    ink = .90;
    color = carmen_graphics_add_color_rgb(255.0*(1.0-ink),
					  255.0*(1.0-ink), 
					  255.0*(1.0-ink));
  }

  adjustment = gtk_adjustment_new(ink, 0.0, 1.0, 0.01, 0.1, 0.0);
  gtk_signal_connect (GTK_OBJECT (adjustment), "value_changed",
		      GTK_SIGNAL_FUNC (set_ink), NULL);
  gtk_signal_connect (GTK_OBJECT (adjustment), "changed",
		      GTK_SIGNAL_FUNC (set_ink), NULL);
  ink_scale = gtk_hscale_new(GTK_ADJUSTMENT(adjustment));
  gtk_scale_set_draw_value(GTK_SCALE(ink_scale), FALSE);

  box5 = gtk_vbox_new(TRUE, 0);
  gtk_box_pack_start(GTK_BOX(box4), box5, TRUE, TRUE, 0);
  gtk_widget_show(box5);
  
  text = gtk_label_new("Ink (Probability)");
  gtk_box_pack_start(GTK_BOX(box5), text, TRUE, TRUE, 0);
  gtk_widget_show(text);
  
  gtk_box_pack_start(GTK_BOX(box5), ink_scale, TRUE, TRUE, 0);
  gtk_widget_show(ink_scale);
  
  sprintf(ink_text, "%.2f", ink);
  ink_label = gtk_label_new(ink_text);
  gtk_box_pack_start(GTK_BOX(box5), ink_label, TRUE, TRUE, 0);
  gtk_widget_show(ink_label);

  separator = gtk_vseparator_new();
  gtk_box_pack_start(GTK_BOX(box2), separator, FALSE, FALSE, 5);
  gtk_widget_show(separator);

  /* fuzzyness */
  box3 = gtk_vbox_new(TRUE, 0);
  gtk_box_pack_start(GTK_BOX(box2), box3, TRUE, TRUE, 0);
  gtk_widget_show(box3);

  text = gtk_label_new("Fuzziness");
  gtk_box_pack_start(GTK_BOX(box3), text, TRUE, TRUE, 0);
  gtk_widget_show(text);

  adjustment = gtk_adjustment_new(fuzzyness, 0.0, 1.0, 0.01, 0.1, 0.0);
  gtk_signal_connect (GTK_OBJECT (adjustment), "value_changed",
		      GTK_SIGNAL_FUNC (set_fuzzyness), NULL);
  fuzzy_scale = gtk_hscale_new(GTK_ADJUSTMENT(adjustment));
  gtk_scale_set_draw_value(GTK_SCALE(fuzzy_scale), FALSE);

  gtk_box_pack_start(GTK_BOX(box3), fuzzy_scale, TRUE, TRUE, 0);
  gtk_widget_show(fuzzy_scale);

  sprintf(fuzzyness_text, "%.2f", fuzzyness);
  fuzzyness_label = gtk_label_new(fuzzyness_text);
  gtk_box_pack_start(GTK_BOX(box3), fuzzyness_label, TRUE, TRUE, 0);
  gtk_widget_show(fuzzyness_label);

  separator = gtk_vseparator_new();
  gtk_box_pack_start(GTK_BOX(box2), separator, FALSE, FALSE, 5);
  gtk_widget_show(separator);

  /* brush size */
  box3 = gtk_vbox_new(TRUE, 0);
  gtk_box_pack_start(GTK_BOX(box2), box3, TRUE, TRUE, 0);
  gtk_widget_show(box3);

  text = gtk_label_new("Brush size");
  gtk_box_pack_start(GTK_BOX(box3), text, TRUE, TRUE, 0);
  gtk_widget_show(text);

  box4 = gtk_hbox_new(FALSE, 0);
  gtk_box_pack_start(GTK_BOX(box3), box4, TRUE, TRUE, 0);
  gtk_widget_show(box4);

  /* brush decr */
  button = gtk_button_new_with_label ("smaller");
  gtk_signal_connect (GTK_OBJECT (button), "clicked", GTK_SIGNAL_FUNC (brush_decr), NULL);
  gtk_box_pack_start(GTK_BOX(box4), button, TRUE, TRUE, 0);
  gtk_widget_show(button);

  /* brush incr */
  button = gtk_button_new_with_label ("larger");
  gtk_signal_connect (GTK_OBJECT (button), "clicked", GTK_SIGNAL_FUNC (brush_incr), NULL);
  gtk_box_pack_start(GTK_BOX(box4), button, TRUE, TRUE, 0);
  gtk_widget_show(button);

  sprintf(brush_text, "%d", carmen_trunc(2.0 * brush_size));
  brush_label = gtk_label_new(brush_text);
  gtk_box_pack_start(GTK_BOX(box3), brush_label, FALSE, FALSE, 0);
  gtk_widget_show(brush_label);

  separator = gtk_vseparator_new();
  gtk_box_pack_start(GTK_BOX(box2), separator, FALSE, FALSE, 5);
  gtk_widget_show(separator);

  /* line_size */
  box3 = gtk_vbox_new(TRUE, 0);
  gtk_box_pack_start(GTK_BOX(box2), box3, TRUE, TRUE, 0);
  gtk_widget_show(box3);

  text = gtk_label_new("Line size");
  gtk_box_pack_start(GTK_BOX(box3), text, TRUE, TRUE, 0);
  gtk_widget_show(text);

  box4 = gtk_hbox_new(FALSE, 0);
  gtk_box_pack_start(GTK_BOX(box3), box4, TRUE, TRUE, 0);
  gtk_widget_show(box4);

  button = gtk_button_new_with_label ("smaller");
  gtk_signal_connect (GTK_OBJECT (button), "clicked", GTK_SIGNAL_FUNC (line_decr), NULL);
  gtk_box_pack_start(GTK_BOX(box4), button, TRUE, TRUE, 0);
  gtk_widget_show(button);

  button = gtk_button_new_with_label ("larger");
  gtk_signal_connect (GTK_OBJECT (button), "clicked", GTK_SIGNAL_FUNC (line_incr), NULL);
  gtk_box_pack_start(GTK_BOX(box4), button, TRUE, TRUE, 0);
  gtk_widget_show(button);

  sprintf(line_text, "%d", carmen_trunc(line_size));
  line_label = gtk_label_new(line_text);
  gtk_box_pack_start(GTK_BOX(box3), line_label, FALSE, FALSE, 0);
  gtk_widget_show(line_label);
  
  separator = gtk_vseparator_new();
  gtk_box_pack_start(GTK_BOX(box2), separator, FALSE, FALSE, 5);
  gtk_widget_show(separator);

  /* filled */
  box3 = gtk_vbox_new(TRUE, 0);
  gtk_box_pack_start(GTK_BOX(box2), box3, TRUE, TRUE, 0);
  gtk_widget_show(box3);

  text = gtk_label_new("Shape fill");
  gtk_box_pack_start(GTK_BOX(box3), text, TRUE, TRUE, 0);
  gtk_widget_show(text);

  box4 = gtk_hbox_new(FALSE, 0);
  gtk_box_pack_start(GTK_BOX(box3), box4, TRUE, TRUE, 0);
  gtk_widget_show(box4);

  button = gtk_button_new_with_label ("not filled");
  gtk_signal_connect (GTK_OBJECT (button), "clicked", 
		      GTK_SIGNAL_FUNC (set_not_filled), NULL);
  gtk_box_pack_start(GTK_BOX(box4), button, TRUE, TRUE, 0);
  gtk_widget_show(button);

  button = gtk_button_new_with_label ("filled");
  gtk_signal_connect (GTK_OBJECT (button), "clicked", 
		      GTK_SIGNAL_FUNC (set_filled), NULL);
  gtk_box_pack_start(GTK_BOX(box4), button, TRUE, TRUE, 0);
  gtk_widget_show(button);

  fill_label = gtk_label_new("not filled");
  gtk_box_pack_start(GTK_BOX(box3), fill_label, FALSE, FALSE, 0);
  gtk_widget_show(fill_label);

  separator = gtk_vseparator_new();
  gtk_box_pack_start(GTK_BOX(box2), separator, FALSE, FALSE, 5);
  gtk_widget_show(separator);

  if (map)
    set_up_map_widgets();
  else
    gtk_widget_show(window);

  place_font = gdk_font_load ("fixed");

}

/* calls the necessary graphics initialization functions */
void init_graphics(int argc, char **argv)
{
  //gnome_init("map_editor", "1.0", argc, argv);
  gtk_init(&argc, &argv);
  gdk_imlib_init();	
  carmen_graphics_setup_colors();

}
