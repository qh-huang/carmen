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

#include <gtk/gtk.h>
#include <carmen/carmen.h>
#include "map_editor.h"
#include "map_editor_menus.h"
#include "map_editor_drawing.h"
#include "map_editor_graphics.h"

#include "cursors.h"              /* cursor bitmaps */

char map_filename[255];

carmen_map_p map = NULL;
int num_offlimits_segments = 0;
int offlimits_capacity = 0;
int show_offlimits = 1;
carmen_offlimits_p offlimits_array = NULL;
int drawing_offlimits = 0;
int show_place_names = 1;
carmen_map_placelist_p place_list = NULL;
int places_capacity = 0;
int deleting_placename;
int adding_placename;
int adding_door;
int current_door_num;

int screen_width;
int screen_height;
int xstart, ystart;
int xend, yend;
double mult;

unsigned char * image_data;
GtkWidget *window;
GdkPixmap *map_pixmap;
GdkPixmap *tmp_pixmap;
GtkWidget *drawing_area;
GtkWidget *scrolled_window;
GdkGC     *Drawing_GC;
GdkColor   color;
GdkColor   yellow, purple, blue, red;
GtkWidget *save_file;
GtkWidget *entry_x_size;
GtkWidget *entry_y_size;
GtkWidget *entry_resolution;
GtkWidget *cur_label_label;
GtkWidget *tool_label;
GtkWidget *coords_label;
GtkWidget *ink_label;
GtkWidget *fuzzyness_label;
GtkWidget *brush_label;
GtkWidget *line_label;
GtkWidget *fill_label;
GtkWidget *hrule, *vrule;
GtkWidget *ink_scale;
GtkWidget *fuzzy_scale;
GtkWidget *hscroll;
GtkWidget *vscroll;
GdkCursor *ccross, *cfill, *cpoint, *cselect;

float     *backup;

int modified;
int saveas_binary;
int no_quit;
int get_name;
int save_cancel;

carmen_utensil_t utensil;
double ink;
double brush_size;
double line_size;
int filled;
int move_enable;

double fuzzyness;

/* signal handler for control C */
void 
shutdown_module(int x)
{
  if(x == SIGINT)
    exit(1);
}

void create_cursors(void)
{
  static GdkColor color1={0,0xFFFF,0xFFFF,0xFFFF};
  static GdkColor color2={0,0x0000,0x0000,0x0000};
  GdkBitmap *curs_pix, *msk_pix;

  curs_pix = gdk_bitmap_create_from_data(NULL, (const gchar *)point_bits, 
					 point_width, point_height);
  msk_pix = gdk_bitmap_create_from_data(NULL, (const gchar *)pointmsk_bits, 
					pointmsk_width, pointmsk_height);
  cpoint = gdk_cursor_new_from_pixmap(curs_pix, msk_pix, &color2, &color1,
				      3,0);
  gdk_bitmap_unref(curs_pix);
  gdk_bitmap_unref(msk_pix);

  curs_pix = gdk_bitmap_create_from_data(NULL, (const gchar *)pour_bits, 
					 pour_width, pour_height);
  msk_pix = gdk_bitmap_create_from_data(NULL, (const gchar *)pourmsk_bits, 
					pourmsk_width, pourmsk_height);
  cfill = gdk_cursor_new_from_pixmap(curs_pix, msk_pix, &color2, &color1,
				     14, 13);
  gdk_bitmap_unref(curs_pix);
  gdk_bitmap_unref(msk_pix);
  
  curs_pix = gdk_bitmap_create_from_data(NULL, (const gchar *)cross_bits, 
					 cross_width, cross_height);
  msk_pix = gdk_bitmap_create_from_data(NULL, (const gchar *)crossmsk_bits, 
					crossmsk_width, crossmsk_height);
  ccross = gdk_cursor_new_from_pixmap(curs_pix, msk_pix, &color2, &color1,
				      8,8);
  gdk_bitmap_unref(curs_pix);
  gdk_bitmap_unref(msk_pix);
  
  curs_pix = gdk_bitmap_create_from_data(NULL, (const gchar *)select_bits, 
					 select_width, select_height);
  msk_pix = gdk_bitmap_create_from_data(NULL, (const gchar *)selectmsk_bits, 
					selectmsk_width, selectmsk_height);
  cselect = gdk_cursor_new_from_pixmap(curs_pix, msk_pix, &color2, &color1,
				       0,select_height);
  gdk_bitmap_unref(curs_pix);
  gdk_bitmap_unref(msk_pix);
}

int 
main(int argc, char ** argv)
{
  signal(SIGINT, shutdown_module);

  map_filename[0] = '\0';
  backup = NULL;
  modified = 0;
  no_quit = 1;
  get_name = 0;

  saveas_binary = -1;
  
  if(argc >= 2) {    
    if (map_open(argv[1], 0) < 0)
      exit(-1);
  }

  init_graphics(argc, argv);

  brush_size = 1.5;
  line_size = 2;
  ink = -2;

  map_pixmap = NULL;
  tmp_pixmap = NULL;
  Drawing_GC = NULL;
  filled = 0;
  fuzzyness = .1;
  move_enable = 0;

  image_data = NULL;

  create_cursors();

  while(1) 
    { fprintf(stderr, "_NOW_");	      
      xstart = 0;
      ystart = 0;
      xend = 0;
      yend = 0;

      start_drawing_window();
      gtk_main();
      if(window) 
	{
	  gtk_widget_hide(window);
	  gtk_widget_destroy(window);
	}
    }
  return 0;
}
