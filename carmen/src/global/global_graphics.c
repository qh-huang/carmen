/*********************************************************
 *
 * This source code is part of the Carnegie Mellon Robot
 * Navigation Toolkit (CARMEN)
 *
 * CARMEN Copyright (c) 2002 Michael Montemerlo, Nicholas
 * Roy, and Sebastian Thrun
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

#include <carmen/carmen_graphics.h>
#include <sys/types.h>

fd_set *x_ipcGetConnections(void);
int x_ipcGetMaxConnection(void);

GdkColor carmen_red, carmen_blue, carmen_white, carmen_yellow, carmen_green, 
  carmen_light_blue, carmen_black, carmen_orange, carmen_grey, 
  carmen_light_grey, carmen_purple;

inline int 
carmen_graphics_map_to_screen(carmen_map_point_p map_point, 
			      carmen_graphics_screen_point_p screen_point, 
			      carmen_graphics_screen_config_p screen) 
{  
  int x = carmen_round((map_point->x / 
			(double)map_point->map->config.x_size) * 
		       screen->width);
  int y = (screen->height - 1) - 
    carmen_round((map_point->y / (double)map_point->map->config.y_size) * 
		 (screen->height - 1));

  screen_point->x = x;
  screen_point->y = y;
  screen_point->config = screen;

  return 0;
}

inline int 
carmen_graphics_screen_to_map(carmen_graphics_screen_point_p screen_point, 
			      carmen_map_point_p map_point, carmen_map_p map) 
{
  double scale_x, scale_y;
  int x, y;

  scale_x = screen_point->config->width / (double)map->config.x_size;
  scale_y = (screen_point->config->height -1) / (double)map->config.y_size;

  x = carmen_round((screen_point->x  - 0.5*scale_x) / scale_x);
  y = carmen_round((screen_point->config->height - 1 - screen_point->y - 
		    0.5*scale_y)/ scale_y);
  
  map_point->x = x;
  map_point->y = y;
  map_point->map = map;

  return 0;
}

inline int 
carmen_graphics_world_to_screen(carmen_world_point_p world_point, 
				carmen_graphics_screen_point_p screen_point, 
				carmen_graphics_screen_config_p screen) 
{  
  double scale_x, scale_y;
  int x, y;
  carmen_map_p map;

  map = world_point->map;
  scale_x = screen->width / 
    (double)(map->config.x_size * map->config.resolution);
  scale_y = (screen->height-1) / 
    (double)(map->config.y_size * map->config.resolution);

  x = carmen_round(world_point->pose.x * scale_x + 
		   0.5*scale_x*map->config.resolution);
  y = (screen->height - 1) - carmen_round(world_point->pose.y * scale_y + 
					  0.5*scale_y*map->config.resolution);

  screen_point->x = x;
  screen_point->y = y;
  screen_point->config = screen;

  return 0;
}

inline int 
carmen_graphics_screen_to_world(carmen_graphics_screen_point_p screen_point, 
				carmen_world_point_p world_point, 
				carmen_map_p map) 
{
  double scale_x, scale_y;
  float x, y;

  scale_x = screen_point->config->width / 
    (double)(map->config.x_size * map->config.resolution);
  scale_y = (screen_point->config->height-1) / 
    (double)(map->config.y_size * map->config.resolution);

  x = carmen_round((screen_point->x - 0.5*scale_x*map->config.resolution)/ 
		   scale_x * 100.0)/ 100.0;
  y = carmen_round((screen_point->config->height - 1 - screen_point->y - 
		    0.5*scale_y*map->config.resolution)/scale_y*100.0)/100.0;

  world_point->pose.x = x;
  world_point->pose.y = y;
  world_point->pose.theta = 0;
  world_point->map = map;

  return 0;
}

inline int 
carmen_graphics_trajectory_to_screen(carmen_traj_point_p traj_point, 
				     carmen_graphics_screen_point_p 
				     screen_point, 
				     carmen_graphics_screen_config_p screen) 
{  
  int x = carmen_round(traj_point->x * screen->zoom);
  int y = (screen->height - 1) - carmen_round((traj_point->y * screen->zoom));

  screen_point->x = x;
  screen_point->y = y;
  screen_point->config = screen;

  return 0;
}

inline int 
carmen_graphics_screen_to_trajectory(carmen_graphics_screen_point_p 
				     screen_point, 
				     carmen_traj_point_p traj_point) 
{
  int x = carmen_round(screen_point->x / screen_point->config->zoom);
  int y = carmen_round(screen_point->config->height - 1 - 
		       screen_point->y  / screen_point->config->zoom);

  traj_point->x = x;
  traj_point->y = y;
  traj_point->theta = 0;
  traj_point->t_vel = 0.0;
  traj_point->r_vel = 0.0;

  return 0;
}

void 
carmen_graphics_verify_list_length(carmen_graphics_callback **list, 
				   int *list_length, int *list_capacity)
{
  carmen_graphics_callback *new_mem;

  if (*list == NULL) 
    {
      *list_capacity = 2;
      *list = (carmen_graphics_callback *)
	calloc(*list_capacity, sizeof(carmen_graphics_callback));
      carmen_test_alloc(*list);
      return;
    }

  if (*list_length == *list_capacity) 
    {
      *list_capacity += 2;
      new_mem = realloc
	(*list, *list_capacity*sizeof(carmen_graphics_callback));
      carmen_test_alloc(new_mem);
      *list = new_mem;
    }
}

void 
carmen_graphics_update_ipc_callbacks(GdkInputFunction callback_Func) 
{
  fd_set *open_fds;
  int max_connection;
  int index;
  int callback_index;
  static carmen_graphics_callback *existing_callbacks = NULL;
  static int num_callbacks = 0;
  static int listsize = 0;

  for (index = 0; index < num_callbacks; index++)
    existing_callbacks[index].ok = 0;

  open_fds = x_ipcGetConnections();
  max_connection = x_ipcGetMaxConnection();
  for (index = 0; index <= max_connection; index++) 
    {
      if (FD_ISSET(index, open_fds)) 
	{
	  for (callback_index = 0; callback_index < num_callbacks; 
	       callback_index++) 
	    {
	      if (index == existing_callbacks[callback_index].fd) 
		{
		  existing_callbacks[callback_index].ok = 1;
		  break;
		}
	    } 
	  if (callback_index == num_callbacks) 
	    {
	      carmen_graphics_verify_list_length(&existing_callbacks, 
						 &num_callbacks, &listsize);
	      existing_callbacks[num_callbacks].fd = index;
	      existing_callbacks[num_callbacks].ok = 1;
	      existing_callbacks[num_callbacks].callback_id = 
		gdk_input_add(index, GDK_INPUT_READ, callback_Func, NULL);
	      num_callbacks++;
	    }
	} /* End of if (FD_ISSET(index, open_fds)) */
    } /* End of for (index = 0; index <= max_connection; index++) */

  for (index = 0; index < num_callbacks; index++) 
    {
      if (existing_callbacks[index].ok == 0) 
	{
	  gdk_input_remove(existing_callbacks[index].callback_id);
	  existing_callbacks[index] = existing_callbacks[num_callbacks-1];
	  num_callbacks--;
	}
    }
}

unsigned char *
carmen_graphics_convert_to_image(carmen_map_p map, int flags) 
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

  if (map == NULL) 
    {
      carmen_warn("carmen_graphics_convert_to_image was passed NULL map.\n");
      return NULL;
    }

  x_size = map->config.x_size;
  y_size = map->config.y_size;
  image_data = (unsigned char *)calloc(x_size*y_size*3, sizeof(unsigned char));
  carmen_test_alloc(image_data);

  if (rescale)
    {
      max_val = -MAXDOUBLE;
      min_val = MAXDOUBLE;
      data_ptr = map->complete_map;
      for (index = 0; index < map->config.x_size*map->config.y_size; index++)
	{
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
  for (x_index = 0; x_index < x_size; x_index++) 
    {
      for (y_index = 0; y_index < y_size; y_index++) 
	{
	  value = *(data_ptr++);	    
	  if (rotate)
	    image_ptr = image_data+y_index*x_size+x_index;
	  if (value < 0) 
	    {
	      if (black_and_white) 
		{
		  *(image_ptr++) = 255;
		  *(image_ptr++) = 255;
		  *(image_ptr++) = 255;
		} 
	      else
		{
		  *(image_ptr++) = 0;
		  *(image_ptr++) = 0;
		  *(image_ptr++) = 255;
		}
	    }
	  else if(!rescale && value > 1.0) 
	    {
	      if (black_and_white) 
		{
		  *(image_ptr++) = 128;
		  *(image_ptr++) = 128;
		  *(image_ptr++) = 128;
		}
	      else
		{
		  *(image_ptr++) = 255;
		  *(image_ptr++) = 0;
		  *(image_ptr++) = 0;
		}
	    }
	  else 
	    {
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

/* generates a pixmap from Image_Data */
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

void 
carmen_graphics_initialize_screenshot(void)
{
  gdk_imlib_init();
}

void 
carmen_graphics_save_pixmap_screenshot(char *basefilename, GdkPixmap *pixmap, 
				       int x, int y, int w, int h)
{
  GdkImlibImage *screenshot;
  char filename[100];
  static int image_count = 0;

  sprintf(filename, "%s%04d.png", basefilename, image_count);
  fprintf(stderr, "Saving image to %s... ", filename);
  
  screenshot = gdk_imlib_create_image_from_drawable(pixmap, NULL, x, y, w, h);
  if (gdk_imlib_save_image(screenshot, filename, NULL) < 0)
    carmen_die_syserror("Could not save to %s", filename);
  gdk_imlib_kill_image(screenshot);
  image_count++;
  fprintf(stderr, "Done 1.\n");
}

static GdkColormap *cmap = NULL;

static void 
_add_color(GdkColor *color, char *name)
{
  if(cmap == NULL)
    cmap = gdk_colormap_get_system();

  if (!gdk_color_parse (name, color)) 
    {
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

GdkColor 
carmen_graphics_add_color(char *name) 
{
  GdkColor color;

  _add_color(&color, name);
  return color;
}

GdkColor 
carmen_graphics_add_color_rgb(int r, int g, int b) 
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

void 
carmen_graphics_write_pixmap_as_png(GdkPixmap *pixmap, char *user_filename,
				    int x, int y, int w, int h)
{
  char basefilename[100] = "video";
  char filename[100];

  GdkImlibImage *screenshot;
  static int image_count = 0;

  if (user_filename == NULL)
    sprintf(filename, "%s%04d.png", basefilename, image_count);
  else
    snprintf(filename, 100, "%s", user_filename);

  carmen_verbose("Saving image to %s... ", filename);
  screenshot = gdk_imlib_create_image_from_drawable(pixmap, NULL, x, y, w, h);
  gdk_imlib_save_image(screenshot, filename, NULL);
  gdk_imlib_kill_image(screenshot);

  image_count++;
}

void 
carmen_graphics_write_data_as_png(unsigned char *data, char *user_filename,
				  int w, int h)
{
  char basefilename[100] = "video";
  char filename[100];

  GdkImlibImage *screenshot;
  static int image_count = 0;

  if (user_filename == NULL)
    sprintf(filename, "%s%04d.png", basefilename, image_count);
  else
    snprintf(filename, 100, "%s", user_filename);

  carmen_verbose("Saving image to %s... ", filename);
  screenshot = gdk_imlib_create_image_from_data(data, NULL, w, h);
  gdk_imlib_save_image(screenshot, filename, NULL);
  gdk_imlib_kill_image(screenshot);

  image_count++;
}

#define ELLIPSE_POINTS 30

void
carmen_graphics_draw_ellipse(GdkPixmap *pixmap, GdkGC *GC, double x, double y,
			     double a, double b, double c, double k);

/* There was a copy of draw_ellipse from localizegraph.c here, but it takes
   things in funny units. x/y should be world points, and a/b/c should also
   be defined with respect to the world, not the map. This needs to be
   fixed. */

