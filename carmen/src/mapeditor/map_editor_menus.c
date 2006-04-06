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
 * map_editor_menus implements the *
 * menu items in the map_editor    *
 ***********************************/

#include <gtk/gtk.h>
#include <carmen/carmen.h>

extern GdkColor carmen_red, carmen_blue, carmen_white, carmen_yellow, 
  carmen_green, carmen_light_blue, carmen_black, carmen_orange, 
  carmen_grey, carmen_light_grey, carmen_purple;

#include "map_editor.h"
#include "map_editor_menus.h"
#include "map_editor_graphics.h"

static GtkWidget *height_entry, *width_entry, *resolution_entry;
static GtkWidget *placename_dialog;
static GtkWidget *name_label, *x_label, *y_label, *theta_label;
static GtkWidget *name_entry, *x_entry, *y_entry, *theta_entry, *x_std_entry, 
  *y_std_entry, *theta_std_entry;

/********************************************************
 *  Menu functions                                      *
 ********************************************************/

void toggle_view(gpointer callback_data __attribute__ ((unused)), 
		 guint callback_action, 
		 GtkWidget *w __attribute__ ((unused)))
{
  if (callback_action == 1) {
    GtkWidget *menu_item = 
      gtk_item_factory_get_widget(item_factory, "/View/Show Placenames");
    show_place_names = ((struct _GtkCheckMenuItem *)menu_item)->active;
  } else if (callback_action == 2) {
    GtkWidget *menu_item = 
      gtk_item_factory_get_widget(item_factory, "/View/Show Offlimits");
    show_offlimits = ((struct _GtkCheckMenuItem *)menu_item)->active;
  } 
  gdk_pixmap_unref(map_pixmap);
  map_pixmap = NULL;
  redraw();
}

void add_placename(gpointer callback_data __attribute__ ((unused)), 
		   guint callback_action __attribute__ ((unused)), 
		   GtkWidget *w __attribute__ ((unused)))
{
  adding_placename = 1;
}

void delete_placename(gpointer callback_data __attribute__ ((unused)), 
		      guint callback_action __attribute__ ((unused)), 
		      GtkWidget *w __attribute__ ((unused)))
{
  deleting_placename = 1;
}

void add_door(gpointer callback_data __attribute__ ((unused)), 
	      guint callback_action __attribute__ ((unused)), 
	      GtkWidget *w __attribute__ ((unused)))
{
  adding_door = 2;
}

void do_delete_placename(int i)
{
  int num_to_move = place_list->num_places-i;

  memmove(place_list->places+i, place_list->places+i+1,
	  sizeof(carmen_place_t)*num_to_move);

  place_list->num_places--;

  gdk_pixmap_unref(map_pixmap);
  map_pixmap = NULL;
  redraw();

  modified++;
  deleting_placename = 0;
}

static void highlight(GtkWidget *label, GtkWidget *entry)
{
  GtkStyle *style;

  style = gtk_widget_get_style(entry);
  style = gtk_style_copy(style);
  style->fg[0] = carmen_red;
  style->fg[1] = carmen_red;
  style->fg[2] = carmen_red;
  style->fg[3] = carmen_red;
  style->fg[4] = carmen_red;
  gtk_widget_set_style(entry, style);
  gtk_widget_set_style(label, style);  
}

static void restore(GtkWidget *label, GtkWidget *entry)
{
  gtk_widget_restore_default_style (label);
  gtk_widget_restore_default_style (entry);
}

static int
is_not_blanks(char *errs)
{
  if (errs == NULL || strlen(errs) == 0)
    return 0;
  if (strcspn(gtk_entry_get_text(GTK_ENTRY(name_entry)), " \t\r\n") == 0)
    return 0;
  return 1;
}

static
void add_place_button(GtkWidget *button __attribute__ ((unused)),
		      gpointer user_data __attribute__ ((unused)))
{
  int num_places;
  int errors = 0;
  double x, y, theta;
  char *errs;

  if (place_list == NULL) {
    place_list = (carmen_map_placelist_p) calloc(1, sizeof(carmen_map_placelist_t));
    carmen_test_alloc(place_list);
  }

  num_places = place_list->num_places;
  place_list->places = realloc(place_list->places, 
			       sizeof(carmen_place_t)*(num_places+1));
  carmen_test_alloc(place_list->places);
  place_list->places[num_places].type = CARMEN_NAMED_POSITION_TYPE;

  if (strlen(gtk_entry_get_text(GTK_ENTRY(name_entry))) == 0 ||
      strcspn(gtk_entry_get_text(GTK_ENTRY(name_entry)), " \t\r\n") == 0)
    {
      errors = 1;
      highlight(name_label, name_entry);
    }
  else {
    restore(name_label, name_entry);
    strcpy(place_list->places[num_places].name, 
	   gtk_entry_get_text(GTK_ENTRY(name_entry)));
  }
  
  x = strtod(gtk_entry_get_text(GTK_ENTRY(x_entry)), &errs);
  if (strlen(gtk_entry_get_text(GTK_ENTRY(x_entry))) == 0 ||
      is_not_blanks(errs) || x < 0 || x >= map->config.x_size)
    {
      errors = 1;
      highlight(x_label, x_entry);
    }
  else
    {
      restore(x_label, x_entry);
      place_list->places[num_places].x = x;
    }

  y = strtod(gtk_entry_get_text(GTK_ENTRY(y_entry)), &errs);
  if (strlen(gtk_entry_get_text(GTK_ENTRY(y_entry))) == 0 ||
      is_not_blanks(errs) || y < 0 || y >= map->config.y_size)
    {
      errors = 1;
      highlight(y_label, y_entry);
    }
  else
    {
      restore(y_label, y_entry);
      place_list->places[num_places].y = y;
    }

  if (strlen(gtk_entry_get_text(GTK_ENTRY(theta_entry))) > 0)
    {
      theta = strtod(gtk_entry_get_text(GTK_ENTRY(theta_entry)), &errs);
      if (is_not_blanks(errs))
	{
	  errors = 1;
	  highlight(theta_label, theta_entry);
	}
      else
	{
	  restore(theta_label, theta_entry);
	  place_list->places[num_places].theta = 
	    carmen_degrees_to_radians(theta);
	}
    }

  if (strlen(gtk_entry_get_text(GTK_ENTRY(x_std_entry))) > 0 &&
      strlen(gtk_entry_get_text(GTK_ENTRY(y_std_entry))) > 0 &&
      strlen(gtk_entry_get_text(GTK_ENTRY(theta_std_entry))) > 0)
    {
      place_list->places[num_places].x_std = 
	strtod(gtk_entry_get_text(GTK_ENTRY(x_std_entry)), NULL);
      place_list->places[num_places].y_std = 
	strtod(gtk_entry_get_text(GTK_ENTRY(y_std_entry)), NULL);
      place_list->places[num_places].theta_std = 
	strtod(gtk_entry_get_text(GTK_ENTRY(theta_std_entry)), NULL);
      place_list->places[num_places].theta_std =      
	carmen_degrees_to_radians(place_list->places[num_places].theta_std);
      place_list->places[num_places].type = CARMEN_LOCALIZATION_INIT_TYPE;
    }

  if (errors)
    return;
  
  place_list->num_places++;

  gdk_pixmap_unref(map_pixmap);
  map_pixmap = NULL;
  redraw();

  modified++;
  adding_placename = 0;
  gtk_widget_destroy(placename_dialog);
}

void start_add_placename(double x, double y)
{
  GtkWidget *hbox, *label, *button;
  char buffer[10];

  placename_dialog = gtk_dialog_new();
  hbox = gtk_hbox_new(FALSE, 0);
  gtk_box_pack_start (GTK_BOX (GTK_DIALOG (placename_dialog)->vbox),
		      hbox, TRUE, TRUE, 0);
  name_label = gtk_label_new("Place name: ");
  gtk_box_pack_start (GTK_BOX (hbox), name_label, TRUE, TRUE, 0);
  name_entry = gtk_entry_new_with_max_length(21);
  gtk_widget_set_usize(name_entry, 90, 20);
  gtk_box_pack_start (GTK_BOX(hbox), name_entry, TRUE, TRUE, 0);
  
  hbox = gtk_hbox_new(FALSE, 3);
  gtk_box_pack_start (GTK_BOX (GTK_DIALOG (placename_dialog)->vbox),
		      hbox, TRUE, TRUE, 0);
  
  x_label = gtk_label_new("X: ");
  gtk_box_pack_start (GTK_BOX (hbox), x_label, TRUE, TRUE, 0);
  x_entry = gtk_entry_new_with_max_length(5);
  gtk_widget_set_usize(x_entry, 45, 20);
  gtk_box_pack_start (GTK_BOX (hbox), x_entry, TRUE, TRUE, 0);
  sprintf(buffer, "%.2f", x);
  gtk_entry_set_text(GTK_ENTRY(x_entry), buffer);

  y_label = gtk_label_new("Y: ");
  gtk_box_pack_start (GTK_BOX (hbox), y_label, TRUE, TRUE, 0);
  y_entry = gtk_entry_new_with_max_length(5);
  gtk_widget_set_usize(y_entry, 45, 20);
  gtk_box_pack_start (GTK_BOX (hbox), y_entry, TRUE, TRUE, 0);
  sprintf(buffer, "%.2f", y);
  gtk_entry_set_text(GTK_ENTRY(y_entry), buffer);
  
  theta_label = gtk_label_new("Theta: ");
  gtk_box_pack_start (GTK_BOX (hbox), theta_label, TRUE, TRUE, 0);
  theta_entry = gtk_entry_new_with_max_length(5);
  gtk_widget_set_usize(theta_entry, 45, 20);
  gtk_box_pack_start (GTK_BOX (hbox), theta_entry, TRUE, TRUE, 0);
  
  hbox = gtk_hbox_new(FALSE, 3);
  gtk_box_pack_start (GTK_BOX (GTK_DIALOG (placename_dialog)->vbox),
		      hbox, TRUE, TRUE, 0);
  
  label = gtk_label_new("Std X: ");
  gtk_box_pack_start (GTK_BOX (hbox), label, TRUE, TRUE, 0);
  x_std_entry = gtk_entry_new_with_max_length(5);
  gtk_widget_set_usize(x_std_entry, 45, 20);
  gtk_box_pack_start (GTK_BOX (hbox), x_std_entry, TRUE, TRUE, 0);
  
  label = gtk_label_new("Std Y: ");
  gtk_box_pack_start (GTK_BOX (hbox), label, TRUE, TRUE, 0);
  y_std_entry = gtk_entry_new_with_max_length(5);
  gtk_widget_set_usize(y_std_entry, 45, 20);
  gtk_box_pack_start (GTK_BOX (hbox), y_std_entry, TRUE, TRUE, 0);
  
  label = gtk_label_new("Std Theta: ");
  gtk_box_pack_start (GTK_BOX (hbox), label, TRUE, TRUE, 0);
  theta_std_entry = gtk_entry_new_with_max_length(5);
  gtk_widget_set_usize(theta_std_entry, 45, 20);
  gtk_box_pack_start (GTK_BOX (hbox), theta_std_entry, TRUE, TRUE, 0);
  
  hbox = GTK_DIALOG(placename_dialog)->action_area;
  
  button = gtk_button_new_with_label("OK");
  gtk_box_pack_start(GTK_BOX(hbox), button, TRUE, TRUE, 5);
  
  gtk_signal_connect(GTK_OBJECT(button), "clicked", 
			 (GtkSignalFunc)add_place_button, NULL);
  
  button = gtk_button_new_with_label("Cancel");
  gtk_box_pack_start(GTK_BOX(hbox), button, TRUE, TRUE, 5);		
  
  gtk_signal_connect_object(GTK_OBJECT(button),
			    "clicked", (GtkSignalFunc)gtk_widget_destroy, 
			    (gpointer)placename_dialog);	

  gtk_widget_show_all(placename_dialog);
}

int next_door_num()
{
  int i, n, k, xxx, loop;

  if (place_list == NULL)
    return 0;

  n = 0;
  loop = 1;
  while (loop) {
    loop = 0;
    for (i = 0; i < place_list->num_places; i++) {
      if (sscanf(place_list->places[i].name, "b%d.%d", &k, &xxx) == 2) {
	if (k == n) {
	  n++;
	  loop = 1;
	}
      }
    }
  }	

  return n;
}

void start_add_door(double x, double y)
{
  int num_places;

  if (place_list == NULL) {
    place_list = (carmen_map_placelist_p) calloc(1, sizeof(carmen_map_placelist_t));
    carmen_test_alloc(place_list);
  }

  num_places = place_list->num_places;
  place_list->places = realloc(place_list->places, 
			       sizeof(carmen_place_t)*(num_places+1));
  carmen_test_alloc(place_list->places);
  place_list->places[num_places].type = CARMEN_NAMED_POSITION_TYPE;

  current_door_num = next_door_num();
  sprintf(place_list->places[num_places].name, "b%d.1", current_door_num);
  
  place_list->places[num_places].x = x;
  place_list->places[num_places].y = y;
  place_list->places[num_places].theta = 0;
  place_list->places[num_places].x_std = 0;
  place_list->places[num_places].y_std = 0;
  place_list->places[num_places].theta_std = 0;
  
  place_list->num_places++;

  gdk_pixmap_unref(map_pixmap);
  map_pixmap = NULL;
  redraw();

  modified++;
  adding_door--;
}

void finish_add_door(double x, double y)
{
  int num_places;

  if (place_list == NULL) {
    place_list = (carmen_map_placelist_p) calloc(1, sizeof(carmen_map_placelist_t));
    carmen_test_alloc(place_list);
  }

  num_places = place_list->num_places;
  place_list->places = realloc(place_list->places, 
			       sizeof(carmen_place_t)*(num_places+1));
  carmen_test_alloc(place_list->places);
  place_list->places[num_places].type = CARMEN_NAMED_POSITION_TYPE;

  sprintf(place_list->places[num_places].name, "b%d.2", current_door_num);
  
  place_list->places[num_places].x = x;
  place_list->places[num_places].y = y;
  place_list->places[num_places].theta = 0;
  place_list->places[num_places].x_std = 0;
  place_list->places[num_places].y_std = 0;
  place_list->places[num_places].theta_std = 0;
  
  place_list->num_places++;

  gdk_pixmap_unref(map_pixmap);
  map_pixmap = NULL;
  redraw();

  modified++;
  adding_door--;
}

gint map_save(char *filename)
{
  carmen_FILE *fp_in, *fp_out;
  char cmd[100];

  if(carmen_map_file(map_filename)) {
    fp_in = carmen_fopen(map_filename, "r");
#ifdef NO_ZLIB
    fp_out = carmen_fopen("/tmp/tmp.cmf", "w");
#else
    fp_out = carmen_fopen("/tmp/tmp.cmf.gz", "w");
#endif
    if(carmen_map_vstrip(fp_in, fp_out, 3, CARMEN_MAP_GRIDMAP_CHUNK,
			 CARMEN_MAP_OFFLIMITS_CHUNK,
			 CARMEN_MAP_PLACES_CHUNK) < 0)
      carmen_warn("Could not strip gridmap from original map.\n");
    if(carmen_map_write_gridmap_chunk(fp_out, map->map, map->config.x_size,
				      map->config.y_size, 
				      map->config.resolution) < 0)
      carmen_warn("Could not write gridmap.\n");

    if (num_offlimits_segments > 0)
      if(carmen_map_write_offlimits_chunk(fp_out, offlimits_array, 
					  num_offlimits_segments) < 0)
	carmen_warn("Could not write offlimits chunk.\n");

    if (place_list != NULL && place_list->num_places > 0)
      if(carmen_map_write_places_chunk(fp_out, place_list->places, 
				       place_list->num_places) < 0)
	carmen_warn("Could not write offlimits chunk.\n");
		
    carmen_fclose(fp_in);
    carmen_fclose(fp_out);
#ifdef NO_ZLIB
    sprintf(cmd, "mv /tmp/tmp.cmf %s", filename);
#else
    sprintf(cmd, "mv /tmp/tmp.cmf.gz %s", filename);
#endif
    system(cmd);
  }
  else {
    fp_out = carmen_fopen(filename, "w");
    if(carmen_map_write_comment_chunk(fp_out, map->config.x_size, 
				      map->config.y_size, 
				      map->config.resolution,
				      "map_editor",
				      "unknown") < 0)
      carmen_warn("Could not write comment chunk to map file.\n");
    if(carmen_map_write_id(fp_out) < 0)
      carmen_warn("Could not write id to map file.\n");
    if(carmen_map_write_creator_chunk(fp_out, "map_editor", "unknown") < 0)
      carmen_warn("Could not write creator chunk to map file.\n");
    if(carmen_map_write_gridmap_chunk(fp_out, map->map, map->config.x_size,
				      map->config.y_size, 
				      map->config.resolution) < 0)
      carmen_warn("Could not write gridmap.\n");
    if (num_offlimits_segments > 0)
      if(carmen_map_write_offlimits_chunk(fp_out, offlimits_array, 
					  num_offlimits_segments) < 0)
	carmen_warn("Could not write offlimits chunk.\n");
    if (place_list && place_list->num_places > 0)
      if(carmen_map_write_places_chunk(fp_out, place_list->places, 
				       place_list->num_places) < 0)
	carmen_warn("Could not write offlimits chunk.\n");
		
    carmen_fclose(fp_out);
  }
  strcpy(map_filename, filename);
  modified = 0;
  return 1;
}

int map_open(char *filename, int have_graphics __attribute__ ((unused)))
{
  char error_str[100];
  carmen_map_p new_map;
  int err;

  if(!carmen_map_file(filename)) {
    sprintf(error_str,
	    "Error: %s does not appear to be a valid carmen map file;\n" 
	    "if it is gzipped, make sure it has a \".gz\" extension\n",
	    filename);
    carmen_warn("%s\n", error_str);

    return -1;
  }

  if (strlen(filename) >= 254)
    return -1;

  strcpy(map_filename, filename);

  new_map = (carmen_map_p)calloc(1, sizeof(carmen_map_t));
  carmen_test_alloc(new_map);

  if (!carmen_map_chunk_exists(filename, CARMEN_MAP_GRIDMAP_CHUNK)) {
    sprintf(error_str, "The map file %s contains no gridmap.", filename);
    /* dbug
      if (have_graphics) 
        gnome_error_dialog_parented(error_str, GTK_WINDOW(window));
	else 
    */
    carmen_warn("%s\n", error_str);
    return -1;
  }

  err = carmen_map_read_gridmap_chunk(map_filename, new_map);
  if (err < 0) {
    sprintf(error_str, "Error reading file %s : %s.", filename,
	    strerror(errno));
    /* dbug
      if (have_graphics) 
        gnome_error_dialog_parented(error_str, GTK_WINDOW(window));
	else 
    */
    carmen_warn("%s\n", error_str);

    if (new_map->map)
      free(new_map->map);
    if (new_map->complete_map)
      free(new_map->complete_map);
    free(new_map);

    return -1;
  }

  if(new_map->config.x_size == 0 || new_map->config.y_size == 0 || 
     new_map->config.resolution == 0.0) 
    {
      sprintf(error_str, "File %s contains no gridmap.", filename);
      /* dbug
	if (have_graphics) 
	  gnome_error_dialog_parented(error_str, GTK_WINDOW(window));
	  else 
      */
      carmen_warn("%s\n", error_str);

      free(new_map);
			
      return -1;
    }

  if (offlimits_array != NULL) {
    free(offlimits_array);
    offlimits_array = NULL;
    num_offlimits_segments = 0;
    offlimits_capacity = 0;
  }

  if (carmen_map_chunk_exists(filename, CARMEN_MAP_OFFLIMITS_CHUNK)) {
    carmen_map_read_offlimits_chunk(map_filename, &offlimits_array, 
				    &num_offlimits_segments);
    if (num_offlimits_segments > 0)
      offlimits_capacity = num_offlimits_segments;
  }

  if (place_list != NULL)
    { 
      free(place_list->places);
      free(place_list);
      place_list = NULL;
      places_capacity = 0;
    }

  if (carmen_map_chunk_exists(filename, CARMEN_MAP_PLACES_CHUNK)) {
    place_list = (carmen_map_placelist_p)calloc(1, sizeof(carmen_map_placelist_t));
    carmen_test_alloc(place_list);
    carmen_map_read_places_chunk(map_filename, place_list);
    if (place_list->num_places > 0)
      places_capacity = place_list->num_places;
  }

  if(map) {
    free(map->map);
    free(map->complete_map);
    free(map);
  } 

  map = new_map;

  if(map_pixmap) {
    gdk_pixmap_unref(map_pixmap);
    gdk_pixmap_unref(tmp_pixmap);
  }

  map_pixmap = NULL;
  tmp_pixmap = NULL;

  if(backup)
    free(backup);
  backup = (float *)calloc(map->config.x_size * map->config.y_size, 
			   sizeof(float));
  carmen_test_alloc(backup);
  memcpy(backup, map->complete_map, sizeof(float) * map->config.x_size * 
	 map->config.y_size);

  modified = 0;
  return 1;
}

void map_open_ok_button(GtkWidget *selector_button __attribute__ ((unused)),
			gpointer user_data)
{
  gchar *filename;
  GtkWidget *selector = (GtkWidget *)user_data;
  
  filename =
    gtk_file_selection_get_filename(GTK_FILE_SELECTION(selector));
  map_open(filename, 1);

  set_up_map_widgets();
}

void create_open_map_selector(void)
{
  GtkWidget *file_select;

  file_select = gtk_file_selection_new("Please select a map for editing.");
  if(strlen(map_filename))
    gtk_file_selection_set_filename(GTK_FILE_SELECTION(file_select), 
				    map_filename);
  gtk_signal_connect(GTK_OBJECT(GTK_FILE_SELECTION
				(file_select)->ok_button),
		     "clicked", (GtkSignalFunc)map_open_ok_button,
		     file_select);

  gtk_signal_connect_object(GTK_OBJECT(GTK_FILE_SELECTION
				       (file_select)->cancel_button),
			    "clicked", (GtkSignalFunc)gtk_widget_destroy,
			    (gpointer)file_select);
  gtk_signal_connect_object(GTK_OBJECT(GTK_FILE_SELECTION
				       (file_select)->ok_button),
			    "clicked", (GtkSignalFunc)gtk_widget_destroy, 
			    (gpointer)file_select);
  gtk_widget_show(file_select);
}

void new_map_button(GtkWidget *selector_button __attribute__ ((unused)),
		    gpointer user_data __attribute__ ((unused)))
{
  int width = atoi(gtk_entry_get_text(GTK_ENTRY(width_entry)));
  int height = atoi(gtk_entry_get_text(GTK_ENTRY(height_entry)));
  double resolution = atof(gtk_entry_get_text(GTK_ENTRY(resolution_entry)));
  int i;
  carmen_map_p new_map;

  if (width <= 0)
    {
      /* dbug
      gnome_error_dialog_parented("Invalid width : should be greater than 0",
				  GTK_WINDOW(window));
      */
      carmen_warn("Invalid width : should be greater than 0");
      return;
    }

  if (height <= 0)
    {
      /* dbug
      gnome_error_dialog_parented("Invalid height : should be greater than 0",
				  GTK_WINDOW(window));
      */
      carmen_warn("Invalid height : should be greater than 0");
      return;
    }
	
  if (resolution <= 0)
    {
      /* dbug
      gnome_error_dialog_parented("Invalid resolution : should be greater "
				  "than 0", GTK_WINDOW(window));
      */
      carmen_warn("Invalid resolution : should be greater ");
      return;
    }
	
  new_map = (carmen_map_p)calloc(1, sizeof(carmen_map_t));
  carmen_test_alloc(new_map);

  new_map->config.x_size = width;
  new_map->config.y_size = height;
  new_map->config.resolution = resolution;

  new_map->complete_map = (float *)calloc(width*height, sizeof(float));
  carmen_test_alloc(new_map->complete_map);

  new_map->map = (float **)calloc(width*height, sizeof(float *));
  carmen_test_alloc(new_map->map);
  for (i = 0; i < width; i++)
    new_map->map[i] = new_map->complete_map+i*height;
	
  if (offlimits_array != NULL) {
    free(offlimits_array);
    offlimits_array = NULL;		
    num_offlimits_segments = 0;
    offlimits_capacity = 0;
  }

  if (place_list != NULL)
    { 
      free(place_list->places);
      free(place_list);
      place_list = NULL;
      places_capacity = 0;
    }

  if(map) {
    free(map->map);
    free(map->complete_map);
    free(map);
  } 

  map = new_map;

  if(map_pixmap) {
    gdk_pixmap_unref(map_pixmap);
    gdk_pixmap_unref(tmp_pixmap);
  }

  map_pixmap = NULL;
  tmp_pixmap = NULL;

  if(backup)
    free(backup);
  backup = (float *)calloc(map->config.x_size * map->config.y_size, 
			   sizeof(float));
  carmen_test_alloc(backup);
  memcpy(backup, map->complete_map, sizeof(float) * map->config.x_size * 
	 map->config.y_size);

  modified = 0;

  set_up_map_widgets();
  return;	
}

void handle_key_in_new_map_box( GtkWidget *widget, GdkEventKey *event )
{
  if ((event->keyval & 0x7f) == 13)
    new_map_button(widget, NULL);
  if ((event->keyval & 0x7f) == 13 || (event->keyval & 0x7f) == 27)
    gtk_widget_destroy(widget);
}

void create_new_map(void)
{
  GtkWidget *dialog = NULL;
  GtkWidget *vbox, *hbox, *hbox2, *label, *button;
	
  dialog = gtk_dialog_new();
  vbox = GTK_DIALOG(dialog)->vbox;
  gtk_window_set_modal(&(GTK_DIALOG(dialog)->window), 1);
  gtk_widget_grab_focus(dialog);

  gtk_widget_set_events (dialog, GDK_KEY_PRESS_MASK);

  gtk_signal_connect(GTK_OBJECT(dialog), "key_press_event", 
		     (GtkSignalFunc)handle_key_in_new_map_box, NULL);

  label = gtk_label_new("New map size: ");		
  gtk_box_pack_start(GTK_BOX(vbox), label, TRUE, TRUE, 5);

  hbox = gtk_hbox_new(FALSE, 5);
  gtk_container_add(GTK_CONTAINER(vbox), hbox);
	
  label = gtk_label_new("Width: ");
  gtk_box_pack_start(GTK_BOX(hbox), label, TRUE, TRUE, 5);
	
  width_entry = gtk_entry_new_with_max_length(4);
  gtk_widget_set_usize(width_entry, 50, 20);
  gtk_entry_prepend_text(GTK_ENTRY(width_entry), "100");
  gtk_box_pack_start(GTK_BOX(hbox), width_entry, TRUE, TRUE, 5);
	
  label = gtk_label_new("Height: ");
  gtk_box_pack_start(GTK_BOX(hbox), label, TRUE, TRUE, 5);
	
  height_entry = gtk_entry_new_with_max_length(4);
  gtk_widget_set_usize(height_entry, 50, 20);
  gtk_entry_prepend_text(GTK_ENTRY(height_entry), "100");
  gtk_box_pack_start(GTK_BOX(hbox), height_entry, TRUE, TRUE, 5);

  hbox2 = gtk_hbox_new(FALSE, 5);
  gtk_container_add(GTK_CONTAINER(vbox), hbox2);
	
  label = gtk_label_new("Resolution (# of meters per grid cell): ");
  gtk_box_pack_start(GTK_BOX(hbox2), label, TRUE, TRUE, 5);
	
  resolution_entry = gtk_entry_new_with_max_length(4);
  gtk_widget_set_usize(resolution_entry, 50, 20);
  gtk_entry_prepend_text(GTK_ENTRY(resolution_entry), "0.1");
  gtk_box_pack_start(GTK_BOX(hbox2), resolution_entry, TRUE, TRUE, 5);
	
  hbox = GTK_DIALOG(dialog)->action_area;
	
  button = gtk_button_new_with_label("OK");
  gtk_box_pack_start(GTK_BOX(hbox), button, TRUE, TRUE, 5);
	
  gtk_signal_connect(GTK_OBJECT(button), "clicked", 
		     (GtkSignalFunc)new_map_button, NULL);
	
  gtk_signal_connect_object(GTK_OBJECT(button),
			    "clicked", (GtkSignalFunc)gtk_widget_destroy,
			    (gpointer)dialog);

  button = gtk_button_new_with_label("Cancel");
  gtk_box_pack_start(GTK_BOX(hbox), button, TRUE, TRUE, 5);		
	
  gtk_signal_connect_object(GTK_OBJECT(button),
			    "clicked", (GtkSignalFunc)gtk_widget_destroy, 
			    (gpointer)dialog);


  gtk_widget_show_all(dialog);
	
}

void save_map_ok_button(GtkWidget *selector_button __attribute__ ((unused)),
			gpointer user_data)
{
  gchar *filename;
  GtkWidget *selector = (GtkWidget *)user_data;
  
  filename =
    gtk_file_selection_get_filename(GTK_FILE_SELECTION(selector));
  map_save(filename);
}

void create_save_map_selector(void)
{
  GtkWidget *file_select;

  file_select = gtk_file_selection_new("Choose a name for the new map.");
  if(strlen(map_filename))
    gtk_file_selection_set_filename(GTK_FILE_SELECTION(file_select), 
				    map_filename);
  gtk_signal_connect(GTK_OBJECT(GTK_FILE_SELECTION
				(file_select)->ok_button),
		     "clicked", (GtkSignalFunc)save_map_ok_button,
		     file_select);
  gtk_signal_connect_object(GTK_OBJECT(GTK_FILE_SELECTION
				       (file_select)->cancel_button),
			    "clicked", (GtkSignalFunc)gtk_widget_destroy,
			    (gpointer)file_select);
  gtk_signal_connect_object(GTK_OBJECT(GTK_FILE_SELECTION
				       (file_select)->ok_button),
			    "clicked", (GtkSignalFunc)gtk_widget_destroy, 
			    (gpointer)file_select);
  gtk_widget_show(file_select);
}

/* dbug
void dont_save_question(gint reply, gpointer data)
{
  if(reply == GNOME_YES) 
    {
      if ((int)data == 1)
	create_open_map_selector();
      else if ((int)data == 2)
	create_new_map();
    }
}
*/

void new_map_menu(gpointer callback_data __attribute__ ((unused)), 
		  guint callback_action __attribute__ ((unused)), 
		  GtkWidget *w __attribute__ ((unused)))
{
  /* dbug
  if(modified)
    gnome_ok_cancel_dialog_modal_parented
      ("The map has been modified.\n"
       "Are you sure you want to\n"
       "throw these changes away?",
       (GnomeReplyCallback)dont_save_question, (gpointer)2, GTK_WINDOW(window));
  else
  */

  create_new_map();
}

void open_map_menu(gpointer callback_data __attribute__ ((unused)), 
		   guint callback_action __attribute__ ((unused)), 
		   GtkWidget *w __attribute__ ((unused)))
{
  /* dbug
  if(modified)
    gnome_ok_cancel_dialog_modal_parented
      ("The map has been modified.\n"
       "Are you sure you want to\n"
       "throw these changes away?",
       (GnomeReplyCallback)dont_save_question,(gpointer)1, GTK_WINDOW(window));
  else
  */

  create_open_map_selector();
}

void save_map_menu(gpointer callback_data __attribute__ ((unused)), 
		   guint callback_action __attribute__ ((unused)), 
		   GtkWidget *w __attribute__ ((unused)))
{
  if(modified)
    map_save(map_filename);
}

void save_map_as_menu(gpointer callback_data __attribute__ ((unused)), 
		      guint callback_action __attribute__ ((unused)), 
		      GtkWidget *w __attribute__ ((unused)))
{
  create_save_map_selector();
}

/* dbug
void quit_question(gint reply __attribute__ ((unused)),
		   gpointer data __attribute__ ((unused)))
{
  if(reply == GNOME_YES)
    gtk_exit(1);
}
*/

void quit_menu(gpointer callback_data __attribute__ ((unused)), 
	       guint callback_action __attribute__ ((unused)), 
	       GtkWidget *w __attribute__ ((unused)))
{
  /* dbug
  if(modified)
    gnome_ok_cancel_dialog_modal_parented("The map has been modified.\n"
					  "Are you sure you want to\n"
					  "quit and throw these changes away?",
					  (GnomeReplyCallback)
					  quit_question,
					  NULL, GTK_WINDOW(window));
  else
  */
  
  gtk_exit(1);
}

/* undoes the emidiatly previous action (only one)*/
void undo_menu(gpointer callback_data __attribute__ ((unused)), 
	       guint callback_action __attribute__ ((unused)), 
	       GtkWidget *w __attribute__ ((unused)))
{
  if(!modified || !backup)
    return;
  memcpy(map->complete_map, backup, 
	 sizeof(float) * map->config.x_size * map->config.y_size);
  modified--;
  if(map_pixmap) {
    gdk_pixmap_unref(map_pixmap);
    gdk_pixmap_unref(tmp_pixmap);
  }
  map_pixmap = NULL;
  tmp_pixmap = NULL;
  redraw();
}

/* Opens a new window containing helpful info */
void help_menu(gpointer callback_data __attribute__ ((unused)), 
	       guint callback_action __attribute__ ((unused)), 
	       GtkWidget *w __attribute__ ((unused)))
{
  GtkWidget *tree;
  GtkWidget *sub_tree1, *sub_tree2, *sub_tree3;
  GtkWidget *item;
  static GtkWidget *window1 = NULL;
  GtkWidget *scrolled_win;
  GtkWidget *text;
  GtkWidget *box;

  if(window1)
    return;

  window1 = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_signal_connect (GTK_OBJECT(window1), "delete_event",
		      GTK_SIGNAL_FUNC (gtk_widget_destroy), window1);
 
  box = gtk_vbox_new(FALSE, 0);
  gtk_container_add(GTK_CONTAINER(window1), box);
  gtk_widget_show(box);

  tree = gtk_tree_new();
  scrolled_win = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_win),
				  GTK_POLICY_AUTOMATIC,
				  GTK_POLICY_AUTOMATIC);
  gtk_widget_set_usize (scrolled_win, 420, 500);
  gtk_box_pack_start(GTK_BOX(box), scrolled_win, TRUE, TRUE, 0);
  gtk_widget_show (scrolled_win);
  
  gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(scrolled_win), tree);

  item = gtk_tree_item_new_with_label("Menus");
  gtk_tree_append(GTK_TREE(tree), item);
  gtk_widget_show(item);

  sub_tree1 = gtk_tree_new();
  gtk_tree_item_set_subtree(GTK_TREE_ITEM(item), sub_tree1);

  item = gtk_tree_item_new_with_label("File");
  gtk_tree_append(GTK_TREE(sub_tree1), item);
  gtk_widget_show(item);

  sub_tree2 = gtk_tree_new();
  gtk_tree_item_set_subtree(GTK_TREE_ITEM(item), sub_tree2);

  item = gtk_tree_item_new_with_label("Open Map");
  gtk_tree_append(GTK_TREE(sub_tree2), item);
  gtk_widget_show(item);

  sub_tree3 = gtk_tree_new();
  gtk_tree_item_set_subtree(GTK_TREE_ITEM(item), sub_tree3);

  item = gtk_tree_item_new();
  text = gtk_label_new("Opens a previously created map file.");
  gtk_label_set_justify(GTK_LABEL(text), GTK_JUSTIFY_LEFT);
  gtk_container_add (GTK_CONTAINER (item), text);
  gtk_widget_show(text);
  gtk_tree_append(GTK_TREE(sub_tree3), item);
  gtk_widget_show(item);

  item = gtk_tree_item_new_with_label("Save Map");
  gtk_tree_append(GTK_TREE(sub_tree2), item);
  gtk_widget_show(item);

  sub_tree3 = gtk_tree_new();
  gtk_tree_item_set_subtree(GTK_TREE_ITEM(item), sub_tree3);

  item = gtk_tree_item_new();
  text = gtk_label_new("Saves the current map to its original file name.");
  gtk_label_set_justify(GTK_LABEL(text), GTK_JUSTIFY_LEFT);
  gtk_container_add (GTK_CONTAINER (item), text);
  gtk_widget_show(text);
  gtk_tree_append(GTK_TREE(sub_tree3), item);
  gtk_widget_show(item);

  item = gtk_tree_item_new_with_label("Save Map as...");
  gtk_tree_append(GTK_TREE(sub_tree2), item);
  gtk_widget_show(item);

  sub_tree3 = gtk_tree_new();
  gtk_tree_item_set_subtree(GTK_TREE_ITEM(item), sub_tree3);

  item = gtk_tree_item_new();
  text = gtk_label_new("Saves the current map to a new file name.");
  gtk_label_set_justify(GTK_LABEL(text), GTK_JUSTIFY_LEFT);
  gtk_container_add (GTK_CONTAINER (item), text);
  gtk_widget_show(text);
  gtk_tree_append(GTK_TREE(sub_tree3), item);
  gtk_widget_show(item);

  item = gtk_tree_item_new_with_label("Quit");
  gtk_tree_append(GTK_TREE(sub_tree2), item);
  gtk_widget_show(item);

  sub_tree3 = gtk_tree_new();
  gtk_tree_item_set_subtree(GTK_TREE_ITEM(item), sub_tree3);

  item = gtk_tree_item_new();
  text = gtk_label_new("Exits the program.");
  gtk_label_set_justify(GTK_LABEL(text), GTK_JUSTIFY_LEFT);
  gtk_container_add (GTK_CONTAINER (item), text);
  gtk_widget_show(text);
  gtk_tree_append(GTK_TREE(sub_tree3), item);
  gtk_widget_show(item);

  item = gtk_tree_item_new_with_label("Edit");
  gtk_tree_append(GTK_TREE(sub_tree1), item);
  gtk_widget_show(item);

  sub_tree2 = gtk_tree_new();
  gtk_tree_item_set_subtree(GTK_TREE_ITEM(item), sub_tree2);

  item = gtk_tree_item_new_with_label("Undo");
  gtk_tree_append(GTK_TREE(sub_tree2), item);
  gtk_widget_show(item);

  sub_tree3 = gtk_tree_new();
  gtk_tree_item_set_subtree(GTK_TREE_ITEM(item), sub_tree3);

  item = gtk_tree_item_new();
  text = gtk_label_new("Undoes the last edit action.");
  gtk_label_set_justify(GTK_LABEL(text), GTK_JUSTIFY_LEFT);
  gtk_container_add (GTK_CONTAINER (item), text);
  gtk_widget_show(text);
  gtk_tree_append(GTK_TREE(sub_tree3), item);
  gtk_widget_show(item);

  item = gtk_tree_item_new_with_label("tools");
  gtk_tree_append(GTK_TREE(tree), item);
  gtk_widget_show(item);

  sub_tree1 = gtk_tree_new();
  gtk_tree_item_set_subtree(GTK_TREE_ITEM(item), sub_tree1);

  item = gtk_tree_item_new_with_label("Brush");
  gtk_tree_append(GTK_TREE(sub_tree1), item);
  gtk_widget_show(item);

  sub_tree2 = gtk_tree_new();
  gtk_tree_item_set_subtree(GTK_TREE_ITEM(item), sub_tree2);

  item = gtk_tree_item_new();
  text = gtk_label_new("The brush draws squares ('brush size' on a side) of \n"
		       "probability given by 'ink' onto the map. The brush is activated \n"
		       "by clicking the left mouse button and dragging the cursor \n"
		       "across the screen.");
  gtk_label_set_justify(GTK_LABEL(text), GTK_JUSTIFY_LEFT);
  gtk_container_add (GTK_CONTAINER (item), text);
  gtk_widget_show(text);
  gtk_tree_append(GTK_TREE(sub_tree2), item);
  gtk_widget_show(item);

  item = gtk_tree_item_new_with_label("Rectangle");
  gtk_tree_append(GTK_TREE(sub_tree1), item);
  gtk_widget_show(item);

  sub_tree2 = gtk_tree_new();
  gtk_tree_item_set_subtree(GTK_TREE_ITEM(item), sub_tree2);

  item = gtk_tree_item_new();
  text = gtk_label_new("The rectangle draws a rectangle of probability 'ink'\n"
		       "(either filled or not) with edges of width 'line size' onto the map. The \n"
		       "rectangle is activated by clicking the left mouse button, dragging the\n"
		       "cursor across the screen, and releasing.");  
  gtk_label_set_justify(GTK_LABEL(text), GTK_JUSTIFY_LEFT);
  gtk_container_add (GTK_CONTAINER (item), text);
  gtk_widget_show(text);
  gtk_tree_append(GTK_TREE(sub_tree2), item);
  gtk_widget_show(item);

  item = gtk_tree_item_new_with_label("Line");
  gtk_tree_append(GTK_TREE(sub_tree1), item);
  gtk_widget_show(item);

  sub_tree2 = gtk_tree_new();
  gtk_tree_item_set_subtree(GTK_TREE_ITEM(item), sub_tree2);

  item = gtk_tree_item_new();
  text = gtk_label_new("The line draws a line of probablilty 'ink' onto the \n"
		       "map. The width of the line is (approximately) 'line size'. The line is \n"
		       "activated by clicking the left mouse button at the starting location, \n"
		       "dragging the cursor across the screen, and releasing.");
  gtk_label_set_justify(GTK_LABEL(text), GTK_JUSTIFY_LEFT);
  gtk_container_add (GTK_CONTAINER (item), text);
  gtk_widget_show(text);
  gtk_tree_append(GTK_TREE(sub_tree2), item);
  gtk_widget_show(item);

  item = gtk_tree_item_new_with_label("Fill");
  gtk_tree_append(GTK_TREE(sub_tree1), item);
  gtk_widget_show(item);

  sub_tree2 = gtk_tree_new();
  gtk_tree_item_set_subtree(GTK_TREE_ITEM(item), sub_tree2);

  item = gtk_tree_item_new();
  text = gtk_label_new("The fill tool fills in a contiguous area of the \n"
		       "same probability with probability 'ink'. The fill tool is activated by \n"
		       "clicking the left mouse button on a location in the area you desire to fill. \n");
  gtk_label_set_justify(GTK_LABEL(text), GTK_JUSTIFY_LEFT);
  gtk_container_add (GTK_CONTAINER (item), text);
  gtk_widget_show(text);
  gtk_tree_append(GTK_TREE(sub_tree2), item);
  gtk_widget_show(item);

  item = gtk_tree_item_new_with_label("Fuzzy Fill");
  gtk_tree_append(GTK_TREE(sub_tree1), item);
  gtk_widget_show(item);

  sub_tree2 = gtk_tree_new();
  gtk_tree_item_set_subtree(GTK_TREE_ITEM(item), sub_tree2);

  item = gtk_tree_item_new();
  text = gtk_label_new("The fuzzy fill tool fills probability 'ink' into a \n"
		       "contiguous area of probability within 'fuzziness' of the probability at the \n"
		       "point you clicked. The fill tool is activated by clicking the left mouse \n"
		       "button on a location in the area you desire to fill.");
  gtk_label_set_justify(GTK_LABEL(text), GTK_JUSTIFY_LEFT);
  gtk_container_add (GTK_CONTAINER (item), text);
  gtk_widget_show(text);
  gtk_tree_append(GTK_TREE(sub_tree2), item);
  gtk_widget_show(item);

  item = gtk_tree_item_new_with_label("Eye Dropper");
  gtk_tree_append(GTK_TREE(sub_tree1), item);
  gtk_widget_show(item);

  sub_tree2 = gtk_tree_new();
  gtk_tree_item_set_subtree(GTK_TREE_ITEM(item), sub_tree2);

  item = gtk_tree_item_new();
  text = gtk_label_new("The eye dropper changes the current 'ink' probability \n \
to the probability of the grid clicked upon. The eye dropper will get \n \
both probabilities and unknown. The eye dropper is actived by \n \
clicking the left mouse button on a grid.");
  gtk_label_set_justify(GTK_LABEL(text), GTK_JUSTIFY_LEFT);
  gtk_container_add (GTK_CONTAINER (item), text);
  gtk_widget_show(text);
  gtk_tree_append(GTK_TREE(sub_tree2), item);
  gtk_widget_show(item);

  item = gtk_tree_item_new_with_label("Zoom");
  gtk_tree_append(GTK_TREE(sub_tree1), item);
  gtk_widget_show(item);

  sub_tree2 = gtk_tree_new();
  gtk_tree_item_set_subtree(GTK_TREE_ITEM(item), sub_tree2);

  item = gtk_tree_item_new();
  text = gtk_label_new("The zoom tool allows you to zoom in or out of the \n \
map. Clicking the left mouse button zooms in to the point you clicked. \n \
Clicking the right mouse button zooms out from the point you \n \
clicked. (currently gtk complains if you zoom in more than twice)");
  gtk_label_set_justify(GTK_LABEL(text), GTK_JUSTIFY_LEFT);
  gtk_container_add (GTK_CONTAINER (item), text);
  gtk_widget_show(text);
  gtk_tree_append(GTK_TREE(sub_tree2), item);
  gtk_widget_show(item);

  item = gtk_tree_item_new_with_label("Mover");
  gtk_tree_append(GTK_TREE(sub_tree1), item);
  gtk_widget_show(item);

  sub_tree2 = gtk_tree_new();
  gtk_tree_item_set_subtree(GTK_TREE_ITEM(item), sub_tree2);

  item = gtk_tree_item_new();
  text = gtk_label_new("The mover tool allows you to move the contents of the window.\n \
Clicking the left mouse button at the point you want to move, dragging \n \
the mouse to where you want that point to be and releasing the button \n \
here will move the map in the window as desired. Note that the mover \n \
utility won't change your map (as does the zoom)");
  gtk_label_set_justify(GTK_LABEL(text), GTK_JUSTIFY_LEFT);
  gtk_container_add (GTK_CONTAINER (item), text);
  gtk_widget_show(text);
  gtk_tree_append(GTK_TREE(sub_tree2), item);
  gtk_widget_show(item);

  item = gtk_tree_item_new_with_label("settings");
  gtk_tree_append(GTK_TREE(tree), item);
  gtk_widget_show(item);
  sub_tree1 = gtk_tree_new();
  gtk_tree_item_set_subtree(GTK_TREE_ITEM(item), sub_tree1);

  item = gtk_tree_item_new_with_label("Ink (probability)");
  gtk_tree_append(GTK_TREE(sub_tree1), item);
  gtk_widget_show(item);

  sub_tree2 = gtk_tree_new();
  gtk_tree_item_set_subtree(GTK_TREE_ITEM(item), sub_tree2);

  item = gtk_tree_item_new(); 
  text = gtk_label_new("Ink sets the probability that the tools write onto the map.          ");
  gtk_label_set_justify(GTK_LABEL(text), GTK_JUSTIFY_LEFT);
  gtk_container_add (GTK_CONTAINER (item), text);
  gtk_widget_show(text);
  gtk_tree_append(GTK_TREE(sub_tree2), item);
  gtk_widget_show(item);

  item = gtk_tree_item_new_with_label("Fuzziness");
  gtk_tree_append(GTK_TREE(sub_tree1), item);
  gtk_widget_show(item);

  sub_tree2 = gtk_tree_new();
  gtk_tree_item_set_subtree(GTK_TREE_ITEM(item), sub_tree2);

  item = gtk_tree_item_new();
  text = gtk_label_new("Fuzziness sets the range accepted for fuzzy fill.");
  gtk_label_set_justify(GTK_LABEL(text), GTK_JUSTIFY_LEFT);
  gtk_container_add (GTK_CONTAINER (item), text);
  gtk_widget_show(text);
  gtk_tree_append(GTK_TREE(sub_tree2), item);
  gtk_widget_show(item);

  item = gtk_tree_item_new_with_label("Brush Size");
  gtk_tree_append(GTK_TREE(sub_tree1), item);
  gtk_widget_show(item);

  sub_tree2 = gtk_tree_new();
  gtk_tree_item_set_subtree(GTK_TREE_ITEM(item), sub_tree2);

  item = gtk_tree_item_new();
  text = gtk_label_new("Brush size sets the length of a side of the brush. \n"
		       "(the brush is a square)");
  gtk_label_set_justify(GTK_LABEL(text), GTK_JUSTIFY_LEFT);
  gtk_container_add (GTK_CONTAINER (item), text);
  gtk_widget_show(text);
  gtk_tree_append(GTK_TREE(sub_tree2), item);
  gtk_widget_show(item);

  item = gtk_tree_item_new_with_label("Line Size");
  gtk_tree_append(GTK_TREE(sub_tree1), item);
  gtk_widget_show(item);

  sub_tree2 = gtk_tree_new();
  gtk_tree_item_set_subtree(GTK_TREE_ITEM(item), sub_tree2);

  item = gtk_tree_item_new();
  text = gtk_label_new("Line size sets the width of lines for the edges of\n"
		       "any shapes (rectangles) and the width of lines.");
  gtk_label_set_justify(GTK_LABEL(text), GTK_JUSTIFY_LEFT);
  gtk_container_add (GTK_CONTAINER (item), text);
  gtk_widget_show(text);
  gtk_tree_append(GTK_TREE(sub_tree2), item);
  gtk_widget_show(item);

  item = gtk_tree_item_new_with_label("Shape Fill");
  gtk_tree_append(GTK_TREE(sub_tree1), item);
  gtk_widget_show(item);
  
  sub_tree2 = gtk_tree_new();
  gtk_tree_item_set_subtree(GTK_TREE_ITEM(item), sub_tree2);

  item = gtk_tree_item_new();
  text = gtk_label_new("Shape fill sets whether or not the shapes are filled\n"
		       "or not. (currently the only shape is a recangle)");
  gtk_label_set_justify(GTK_LABEL(text), GTK_JUSTIFY_LEFT);
  gtk_container_add (GTK_CONTAINER (item), text);
  gtk_widget_show(text);
  gtk_tree_append(GTK_TREE(sub_tree2), item);
  gtk_widget_show(item);

  gtk_widget_show(tree);
  gtk_widget_show(window1);
  return;
}
