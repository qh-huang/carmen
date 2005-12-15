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
#include <X11/Xlib.h>
#include <X11/cursorfont.h>
#include <ctype.h>

#include "navigator.h"
#include "navigator_panel.h"
#include "navigator_graphics.h"

#undef USE_DOT

#ifdef USE_DOT 

#include <carmen/dot.h>
#include <carmen/dot_messages.h>
#include <carmen/dot_interface.h>

#endif

#define BUTTON_WIDTH 250
#define BUTTON_HEIGHT 30
#define GRADIENT_COLORS 40

#define ALWAYS_REDRAW 0

#define DEFAULT_ROBOT_COLOUR carmen_red
#define DEFAULT_GOAL_COLOUR carmen_yellow
#define DEFAULT_PATH_COLOUR carmen_blue

#define DEFAULT_PEOPLE_COLOUR carmen_orange
#define DEFAULT_TRASH_COLOUR carmen_green
#define DEFAULT_DOOR_COLOUR carmen_purple

#define DEFAULT_TRACK_ROBOT 1
#define DEFAULT_DRAW_WAYPOINTS 1
#define DEFAULT_SHOW_PARTICLES 0
#define DEFAULT_SHOW_GAUSSIANS 0
#define DEFAULT_SHOW_LASER 0
#define DEFAULT_SHOW_SIMULATOR 0

#define DEFAULT_SHOW_TRACKED_OBJECTS 1

typedef void *(*void_func)(void *);

typedef enum {NO_PLACEMENT, PLACING_ROBOT, ORIENTING_ROBOT,
	      PLACING_GOAL, PLACING_PERSON, ORIENTING_PERSON,
	      PLACING_SIMULATOR, ORIENTING_SIMULATOR} placement_t;

static carmen_navigator_map_t display;
static carmen_map_placelist_p placelist = NULL;

static double time_of_last_redraw = 0;
static int display_needs_updating = 0;
static int num_path_points;
static carmen_world_point_t *path = NULL;
static carmen_world_point_t goal;
static carmen_world_point_t robot;
static carmen_traj_point_t  robot_traj;
static carmen_world_point_t last_robot;
static carmen_world_point_t new_person;
static carmen_world_point_t new_simulator;

static GdkColor robot_colour, goal_colour, people_colour, trash_colour,
  door_colour, path_colour;

static int black_and_white = 0;
static int is_filming = 0;
static guint filming_timeout = 0;

static placement_t placement_status = 0;
static carmen_world_point_t cursor_pos;
static double last_navigator_update = 0;
static double last_simulator_update = 0;

static carmen_localize_globalpos_message *globalpos;
static carmen_localize_particle_message particle_msg;
static carmen_localize_sensor_message sensor_msg;
GdkColor RedBlueGradient[GRADIENT_COLORS];

static int ignore_click;

static GtkMapViewer *map_view;

static GtkWidget *window;
static GtkItemFactory *item_factory;
static GtkWidget *autonomous_button;
static GtkWidget *place_robot_button;
static GtkWidget *place_goal_button;
static GtkWidget *robot_status_label;
static GtkWidget *robot_speed_label;
static GtkWidget *goal_status_label;
static GtkWidget *cursor_status_label;
static GtkWidget *value_label;
static GtkWidget *map_status_label;
static GtkWidget *simulator_box;
static GtkWidget *filler_box;
static GtkWidget *place_simulator_button;
static GtkWidget *sync_mode_button;
static GtkWidget *next_tick_button;

static carmen_world_point_t simulator_trueposition = {{0, 0, 0}, NULL};
static double time_of_simulator_update = 0;
static double simulator_hidden;

static carmen_list_t *simulator_objects = NULL;
static carmen_list_t *people = NULL;
static carmen_list_t *trash = NULL;
static carmen_list_t *doors = NULL;

static carmen_robot_config_t *robot_config;
static carmen_navigator_config_t *nav_config;
static carmen_navigator_panel_config_t *nav_panel_config;

static void switch_display(GtkWidget *w, carmen_navigator_map_t new_display);
static void switch_localize_display(GtkWidget *w, int arg);
static void set_location(GtkWidget *w, int place_index);
static void start_filming(GtkWidget *w __attribute__ ((unused)), 
			  int arg __attribute__ ((unused)));
static gint save_image(gpointer data, guint action, GtkWidget *widget);
static int colour_equals(GdkColor colour1, GdkColor colour2);

static void sync_mode_change_handler(char *module, char *variable, 
				     char *value);

static void 
delete_event(GtkWidget *widget, GdkEvent *event, gpointer data) 
{
  widget = widget; event = event; data = data;

  gtk_main_quit ();
}

static void
label_autonomy_button(char *str)
{
  GtkWidget *label;
  
  label = GTK_BIN(autonomous_button)->child;
  gtk_label_set_text(GTK_LABEL(label), str);
}

static void 
go_autonomous(GtkWidget *widget __attribute__ ((unused)), 
	      gpointer data __attribute__ ((unused))) 
{
  GtkWidget *label;

  if (!ignore_click) 
    {
      if (GTK_TOGGLE_BUTTON (autonomous_button)->active) 
	{
	  label = GTK_BIN(autonomous_button)->child;
	  gtk_label_set_text(GTK_LABEL(label), "Stop");
	  navigator_start_moving();
	}
      else 
	{
	  label = GTK_BIN(autonomous_button)->child;
	  gtk_label_set_text(GTK_LABEL(label), "Go");
	  navigator_stop_moving();
	}
    } 
  else
    ignore_click = 0;
}

static void 
next_tick(GtkWidget *widget __attribute__ ((unused)), 
	  gpointer data __attribute__ ((unused))) 
{
  if (GTK_TOGGLE_BUTTON(sync_mode_button)->active)
    {
      carmen_simulator_next_tick();
    }
}

static void 
sync_mode(GtkWidget *widget __attribute__ ((unused)), 
	  gpointer data __attribute__ ((unused))) 
{
  carmen_param_set_module(NULL);
  carmen_param_set_onoff
    ("simulator_sync_mode", GTK_TOGGLE_BUTTON(sync_mode_button)->active,
     NULL);
}

typedef struct _GdkCursorPrivate
{
  GdkCursor cursor;
  Cursor xcursor;
  Display *xdisplay;
} GdkCursorPrivate;

static void 
place_robot(GtkWidget *widget __attribute__ ((unused)), 
	    gpointer data __attribute__ ((unused))) 
{
  XColor xfg, xbg;
  GdkCursor *cursor = gdk_cursor_new(GDK_DOT);

  xfg.pixel = carmen_red.pixel;
  xfg.red = carmen_red.red;
  xfg.blue = carmen_red.blue;
  xfg.green = carmen_red.green;

  xbg.pixel = carmen_black.pixel;
  xbg.red = carmen_black.red;
  xbg.blue = carmen_black.blue;
  xbg.green = carmen_black.green;

  XRecolorCursor(((GdkCursorPrivate *)cursor)->xdisplay, 
		 ((GdkCursorPrivate *)cursor)->xcursor, &xfg, &xbg);

  gdk_window_set_cursor (map_view->image_widget->window, cursor);

  placement_status = PLACING_ROBOT;
}

static void 
place_simulator(GtkWidget *widget __attribute__ ((unused)), 
		gpointer data __attribute__ ((unused))) 
{
  XColor xfg, xbg;
  GdkCursor *cursor = gdk_cursor_new(GDK_DOT);

  xfg.pixel = carmen_red.pixel;
  xfg.red = carmen_red.red;
  xfg.blue = carmen_red.blue;
  xfg.green = carmen_red.green;

  xbg.pixel = carmen_black.pixel;
  xbg.red = carmen_black.red;
  xbg.blue = carmen_black.blue;
  xbg.green = carmen_black.green;

  XRecolorCursor(((GdkCursorPrivate *)cursor)->xdisplay, 
		 ((GdkCursorPrivate *)cursor)->xcursor, &xfg, &xbg);

  gdk_window_set_cursor (map_view->image_widget->window, cursor);

  placement_status = PLACING_SIMULATOR;
}

static void 
place_person(GtkWidget *w __attribute__ ((unused)), 
	     int arg __attribute__ ((unused)))
{
  XColor xfg, xbg;
  GdkCursor *cursor = gdk_cursor_new(GDK_DOT);

  xfg.pixel = carmen_orange.pixel;
  xfg.red = carmen_orange.red;
  xfg.blue = carmen_orange.blue;
  xfg.green = carmen_orange.green;

  xbg.pixel = carmen_black.pixel;
  xbg.red = carmen_black.red;
  xbg.blue = carmen_black.blue;
  xbg.green = carmen_black.green;

  XRecolorCursor(((GdkCursorPrivate *)cursor)->xdisplay, 
		 ((GdkCursorPrivate *)cursor)->xcursor, &xfg, &xbg);

  gdk_window_set_cursor (map_view->image_widget->window, cursor);

  placement_status = PLACING_PERSON;
}

static void 
clear_objects(GtkWidget *w __attribute__ ((unused)), 
	      int arg __attribute__ ((unused)))
{
  carmen_simulator_clear_objects();
}

static void 
place_goal(GtkWidget *widget __attribute__ ((unused)), 
	    gpointer data __attribute__ ((unused))) 
{
  XColor xfg, xbg;
  GdkCursor *cursor = gdk_cursor_new(GDK_DOT);

  xfg.pixel = carmen_yellow.pixel;
  xfg.red = carmen_yellow.red;
  xfg.blue = carmen_yellow.blue;
  xfg.green = carmen_yellow.green;

  xbg.pixel = carmen_black.pixel;
  xbg.red = carmen_black.red;
  xbg.blue = carmen_black.blue;
  xbg.green = carmen_black.green;

  XRecolorCursor(((GdkCursorPrivate *)cursor)->xdisplay, 
		 ((GdkCursorPrivate *)cursor)->xcursor, &xfg, &xbg);

  gdk_window_set_cursor (map_view->image_widget->window, cursor);

  placement_status = PLACING_GOAL;
}

static GtkItemFactoryEntry menu_items[] = {
  { "/_File",         NULL,          NULL, 0, "<Branch>" },
  { "/File/_Screen Shot", "<control>S",  (GtkItemFactoryCallback)save_image, 
    0, NULL },
  { "/File/Start Filming", NULL, start_filming, 0, NULL },
  { "/File/Stop Filming", NULL, start_filming, 0, NULL },
  { "/File/sep1",     NULL,          NULL, 0, "<Separator>" },
  { "/File/_Quit",     "<control>Q",  gtk_main_quit, 0, NULL },
  { "/_Maps",         NULL,          NULL, 0, "<Branch>" },
  { "/Maps/Map",      "<control>M", switch_display, CARMEN_NAVIGATOR_MAP_v, 
    "<RadioItem>"},
  { "/Maps/Utility",  NULL, switch_display, CARMEN_NAVIGATOR_UTILITY_v,
    "/Maps/Map"},
  { "/Maps/Costs",    NULL, switch_display, CARMEN_NAVIGATOR_COST_v,  
    "/Maps/Map"},
  { "/Maps/Likelihood", NULL, switch_display, CARMEN_LOCALIZE_LMAP_v,  
    "/Maps/Map"},
  { "/Maps/Global Likelihood", NULL, switch_display, CARMEN_LOCALIZE_GMAP_v,  
    "/Maps/Map"},
  { "/_Display",         NULL,          NULL, 0, "<Branch>" },
  { "/Display/Track Robot",   NULL, switch_localize_display, 1, 
    "<ToggleItem>"}, 
  { "/Display/Draw Waypoints",   NULL, switch_localize_display, 2, 
    "<ToggleItem>"}, 
  { "/Display/sep1",     NULL,          NULL, 0, "<Separator>" },
  { "/Display/Show Particles",   NULL, switch_localize_display, 3, 
    "<ToggleItem>"}, 
  { "/Display/Show Gaussians",   NULL, switch_localize_display, 4, 
    "<ToggleItem>"}, 
  { "/Display/Show Laser Data",   NULL, switch_localize_display, 5, 
    "<ToggleItem>"},
#ifdef USE_DOT
  { "/Display/sep1",     NULL,          NULL, 0, "<Separator>" },
  { "/Display/Show Tracked Objects",   NULL, switch_localize_display, 9, 
    "<ToggleItem>"},
#endif
  { "/Display/sep1",     NULL,          NULL, 0, "<Separator>" },
  { "/Display/Black&White", NULL, switch_localize_display, 8,  "<ToggleItem>" },
  { "/_Simulator",         NULL,          NULL, 0, "<Branch>" },
  { "/Simulator/Show True Position",   NULL, 
    switch_localize_display, 6, "<ToggleItem>"},
  { "/Simulator/Show Objects", NULL, switch_localize_display, 7, "<ToggleItem>"}, 
  { "/Simulator/sep1",     NULL,          NULL, 0, "<Separator>" },
  { "/Simulator/Add Person", NULL, place_person, 7, NULL}, 
  { "/Simulator/Clear Objects", NULL, clear_objects, 7, NULL},
  { "/_Start Location", NULL,          NULL, 0, "<Branch>" },
  { "/Start Location/Global Localization", NULL, set_location, 0, NULL },
  { "/Start Location/sep1", NULL, NULL, 0, "<Separator>" },
  { "/_Goals",         NULL,           NULL, 0, "<Branch>" },
  { "/_Help",         NULL,          NULL, 0, "<LastBranch>" },
  { "/_Help/About",   NULL,          NULL, 0, NULL }
};

/*
static void 
switch_people_display(GtkWidget *w __attribute__ ((unused)), 
		      int arg  __attribute__ ((unused)))
{
  GtkWidget *menu_item = gtk_item_factory_get_widget(item_factory, 
						     "/Display/People");
  display_people = ((struct _GtkCheckMenuItem *)menu_item)->active;  
}
*/

static void 
switch_localize_display(GtkWidget *w __attribute__ ((unused)), 
			int arg)
		 
{
  if (arg == 1) 
    {
      GtkWidget *menu_item = 
	gtk_item_factory_get_widget(item_factory,"/Display/Track Robot");
      nav_panel_config->track_robot = ((struct _GtkCheckMenuItem *)menu_item)->active;
      if (nav_panel_config->track_robot) 
	carmen_map_graphics_adjust_scrollbars(map_view, &robot);    
    } 
  else if (arg == 2) 
    {
      GtkWidget *menu_item = 
	gtk_item_factory_get_widget
	(item_factory,"/Display/Draw Waypoints");
      nav_panel_config->draw_waypoints = ((struct _GtkCheckMenuItem *)menu_item)->active;
    } 
  else if (arg == 3) 
    {
      GtkWidget *menu_item = 
	gtk_item_factory_get_widget(item_factory, 
				    "/Display/Show Particles");
      nav_panel_config->show_particles = ((struct _GtkCheckMenuItem *)menu_item)->active;
      if (nav_panel_config->show_particles == 1 && !nav_panel_config->show_gaussians) 
	carmen_localize_subscribe_particle_message
	  (&particle_msg, NULL, CARMEN_SUBSCRIBE_LATEST);
      else if (!nav_panel_config->show_particles && !nav_panel_config->show_gaussians)
	carmen_localize_subscribe_particle_message(NULL, NULL, 
						   CARMEN_UNSUBSCRIBE);
    }
  else if (arg == 4) 
    {
      GtkWidget *menu_item = 
	gtk_item_factory_get_widget(item_factory, 
				    "/Display/Show Gaussians");
      nav_panel_config->show_gaussians = ((struct _GtkCheckMenuItem *)menu_item)->active;
      if (nav_panel_config->show_gaussians == 1 && !nav_panel_config->show_particles) 
	carmen_localize_subscribe_particle_message
	  (&particle_msg, NULL, CARMEN_SUBSCRIBE_LATEST);
      else if (!nav_panel_config->show_gaussians && !nav_panel_config->show_particles)
	carmen_localize_subscribe_particle_message(NULL, NULL, 
						   CARMEN_UNSUBSCRIBE);
    }
  else if (arg == 5) 
    {
      GtkWidget *menu_item = 
	gtk_item_factory_get_widget(item_factory, 
				    "/Display/Show Laser Data");
      nav_panel_config->show_lasers = ((struct _GtkCheckMenuItem *)menu_item)->active;
      if (nav_panel_config->show_lasers == 1) 
	carmen_localize_subscribe_sensor_message
	  (&sensor_msg, NULL, CARMEN_SUBSCRIBE_LATEST);
      else
	carmen_localize_subscribe_sensor_message(NULL, NULL, 
						 CARMEN_UNSUBSCRIBE);
    }
  else if (arg == 6) 
    {
      GtkWidget *menu_item = 
	gtk_item_factory_get_widget(item_factory,"/Simulator/Show True Position");
      nav_panel_config->show_true_pos = ((struct _GtkCheckMenuItem *)menu_item)->active;
    } 
  else if (arg == 7) 
    {
      GtkWidget *menu_item = 
	gtk_item_factory_get_widget(item_factory,"/Simulator/Show Objects");
      nav_panel_config->show_simulator_objects = ((struct _GtkCheckMenuItem *)menu_item)->active;
    } 
  else if (arg == 8) 
    {
      GtkWidget *menu_item = 
	gtk_item_factory_get_widget(item_factory,"/Display/Black&White");
      black_and_white = ((struct _GtkCheckMenuItem *)menu_item)->active;
      if (black_and_white) 
	{
	  if (colour_equals(robot_colour, DEFAULT_ROBOT_COLOUR))
	    robot_colour = carmen_grey;
	  if (colour_equals(goal_colour, DEFAULT_GOAL_COLOUR))
	    goal_colour = carmen_grey;
	  if (colour_equals(path_colour, DEFAULT_PATH_COLOUR))
	    path_colour = carmen_black;
	}
      else
	{
	  if (colour_equals(robot_colour, carmen_grey))
	    robot_colour = DEFAULT_ROBOT_COLOUR;
	  if (colour_equals(goal_colour, carmen_grey))
	    goal_colour = DEFAULT_GOAL_COLOUR;
	  if (colour_equals(path_colour, carmen_black))
	    path_colour = DEFAULT_PATH_COLOUR;
	}
      switch_display(NULL, display);
    } 
  else if (arg == 9) 
    {
      GtkWidget *menu_item = 
	gtk_item_factory_get_widget(item_factory,"/Display/Show Tracked Objects");
      nav_panel_config->show_tracked_objects = ((struct _GtkCheckMenuItem *)menu_item)->active;
    } 
}

static gint
film_image(gpointer data)
{
  return save_image(data, 0, NULL);
}

static void 
start_filming(GtkWidget *w __attribute__ ((unused)), 
	      int arg __attribute__ ((unused)))
{
  GtkWidget *menu_item;

  if (is_filming) 
    {
      menu_item = 
	gtk_item_factory_get_widget(item_factory, 
				    "/File/Stop Filming");
      gtk_widget_hide(menu_item);
      menu_item = 
	gtk_item_factory_get_widget(item_factory, 
				    "/File/Start Filming");
      gtk_widget_show(menu_item);
      gtk_timeout_remove(filming_timeout);
      is_filming = 0;
    } 
  else
    {
      menu_item = 
	gtk_item_factory_get_widget(item_factory, 
				    "/File/Start Filming");
      gtk_widget_hide(menu_item);
      menu_item = 
	gtk_item_factory_get_widget(item_factory, 
				    "/File/Stop Filming");
      gtk_widget_show(menu_item);
      is_filming = 1;
      filming_timeout = gtk_timeout_add
	(1000, (GtkFunction)film_image, NULL);
    }
}

static int
colour_equals(GdkColor colour1, GdkColor colour2)
{
  if (colour1.red == colour2.red &&
      colour1.blue == colour2.blue &&
      colour1.green == colour2.green)
    return 1;
  else
    return 0;
}


static void
assign_colour(GdkColor *colour, int new_colour)
{
  colour->pixel = new_colour;
  colour->blue = new_colour & 0xff;
  new_colour >>= 8;
  colour->green = new_colour & 0xff;
  new_colour >>= 8;
  colour->red = new_colour & 0xff;
  new_colour >>= 8;
}

static void
assign_variable(char *menu_text, int value, int default_value)
{
  GtkWidget *menu_item;
  int state;

  if (value > 2)
    return;

  menu_item = gtk_item_factory_get_widget(item_factory,menu_text);
  state = ((struct _GtkCheckMenuItem *)menu_item)->active;
  if (value == -1)
    value = default_value;
  if (state != value)
     gtk_menu_item_activate(GTK_MENU_ITEM(menu_item));
}

void 
navigator_graphics_reset(void)
{
  robot_colour = DEFAULT_ROBOT_COLOUR;
  goal_colour = DEFAULT_GOAL_COLOUR;
  path_colour = DEFAULT_PATH_COLOUR;
  trash_colour = DEFAULT_TRASH_COLOUR;
  people_colour = DEFAULT_PEOPLE_COLOUR;
  door_colour = DEFAULT_DOOR_COLOUR;

  assign_variable("/Display/Track Robot", -1, DEFAULT_TRACK_ROBOT);
  assign_variable("/Display/Draw Waypoints", -1, DEFAULT_DRAW_WAYPOINTS);
  assign_variable("/Display/Show Particles", -1, DEFAULT_SHOW_PARTICLES);
  assign_variable("/Display/Show Gaussians", -1, DEFAULT_SHOW_GAUSSIANS);
  assign_variable("/Display/Show Laser Data", -1, DEFAULT_SHOW_LASER);
  assign_variable("/Display/Show Simulator True Position", -1,
		  DEFAULT_SHOW_SIMULATOR);
  assign_variable("/Display/Show Tracked Objects", -1, DEFAULT_SHOW_TRACKED_OBJECTS);
}

void 
navigator_graphics_display_config
(char *attribute, int value, char *new_status_message __attribute__ ((unused)))
{
  if (strncmp(attribute, "robot colour", 12) == 0) 
    {
      if (value == -1)
	robot_colour = DEFAULT_ROBOT_COLOUR;
      else
	assign_colour(&robot_colour, value);
    }

  else if (strncmp(attribute, "goal colour", 11) == 0) 
    {
      if (value == -1)
	goal_colour = DEFAULT_GOAL_COLOUR;
      else
	assign_colour(&goal_colour, value);
    }

  else if (strncmp(attribute, "path colour", 11) == 0) 
    {
      if (value == -1)
	path_colour = DEFAULT_PATH_COLOUR;
      else
	assign_colour(&path_colour, value);
    }
  else if (strncmp(attribute, "people colour", 11) == 0) 
    {
      if (value == -1)
	path_colour = DEFAULT_PATH_COLOUR;
      else
	assign_colour(&people_colour, value);
    }
  else if (strncmp(attribute, "trash colour", 11) == 0) 
    {
      if (value == -1)
	path_colour = DEFAULT_TRASH_COLOUR;
      else
	assign_colour(&trash_colour, value);
    }
  else if (strncmp(attribute, "door colour", 11) == 0) 
    {
      if (value == -1)
	path_colour = DEFAULT_DOOR_COLOUR;
      else
	assign_colour(&door_colour, value);
    }
  else if (strncmp(attribute, "track robot", 11) == 0)
    assign_variable("/Display/Track Robot", value, 
		    DEFAULT_TRACK_ROBOT);
  else if (strncmp(attribute, "draw waypoints", 14) == 0)
    assign_variable("/Display/Draw Waypoints", value, 
		    DEFAULT_DRAW_WAYPOINTS);
  else if (strncmp(attribute, "show particles", 14) == 0)
    assign_variable("/Display/Show Particles", value, 
		    DEFAULT_SHOW_PARTICLES);
  else if (strncmp(attribute, "show gaussians", 14) == 0)
    assign_variable("/Display/Show Gaussians", value,
		    DEFAULT_SHOW_GAUSSIANS);
  else if (strncmp(attribute, "show laser", 10) == 0)
    assign_variable("/Display/Show Laser Data", value,
		    DEFAULT_SHOW_LASER);
  else if (strncmp(attribute, "show simulator", 14) == 0)
    assign_variable("/Display/Show Simulator True Position", value,
		    DEFAULT_SHOW_SIMULATOR);
  else if (strncmp(attribute, "show tracked objects", 20) == 0)
    assign_variable("/Display/Show Tracked Objects", value,
		    DEFAULT_SHOW_TRACKED_OBJECTS);
  /*
  else if (strncmp(attribute, "show people", 11) == 0)
    assign_variable("/Show/People", value, DEFAULT_SHOW_PEOPLE);
  */
  carmen_map_graphics_redraw(map_view);
}


static void switch_display(GtkWidget *w __attribute__ ((unused)), 
			   carmen_navigator_map_t new_display) {
  navigator_get_map(new_display);
}

static void 
get_main_menu(GtkWidget *new_window, GtkWidget **menubar) 
{
  GtkAccelGroup *accel_group;
  gint nmenu_items = sizeof (menu_items) / sizeof (menu_items[0]);
  GtkWidget *menu_item;

  accel_group = gtk_accel_group_new ();
  
  item_factory = gtk_item_factory_new (GTK_TYPE_MENU_BAR, "<main>", 
				       accel_group);
  gtk_item_factory_create_items (item_factory, nmenu_items, menu_items, NULL);
  gtk_accel_group_attach (accel_group, GTK_OBJECT (new_window));
  if (menubar)
    *menubar = gtk_item_factory_get_widget (item_factory, "<main>");

  menu_item = 
    gtk_item_factory_get_item(item_factory, "/Display/Track Robot");
  ((struct _GtkCheckMenuItem *)menu_item)->active = nav_panel_config->track_robot;

  menu_item = 
    gtk_item_factory_get_item(item_factory, "/Display/Draw Waypoints");
  ((struct _GtkCheckMenuItem *)menu_item)->active = nav_panel_config->draw_waypoints;

  menu_item = 
    gtk_item_factory_get_item(item_factory, "/Display/Show Particles");
  ((struct _GtkCheckMenuItem *)menu_item)->active = nav_panel_config->show_particles;

  menu_item = 
    gtk_item_factory_get_item(item_factory, "/Display/Show Gaussians");
  ((struct _GtkCheckMenuItem *)menu_item)->active = nav_panel_config->show_gaussians;

  menu_item = 
    gtk_item_factory_get_item(item_factory, "/Display/Show Laser Data");
  ((struct _GtkCheckMenuItem *)menu_item)->active = nav_panel_config->show_lasers;

#ifdef USE_DOT
  menu_item = 
    gtk_item_factory_get_item(item_factory, "/Display/Show Tracked Objects");
  ((struct _GtkCheckMenuItem *)menu_item)->active = nav_panel_config->show_tracked_objects;
#endif

  menu_item = 
    gtk_item_factory_get_item(item_factory, "/Simulator/Show True Position");
  ((struct _GtkCheckMenuItem *)menu_item)->active = nav_panel_config->show_true_pos;

  menu_item = 
    gtk_item_factory_get_item(item_factory, "/Simulator/Show Objects");
  ((struct _GtkCheckMenuItem *)menu_item)->active = nav_panel_config->show_simulator_objects;

  if (nav_panel_config->show_particles || nav_panel_config->show_gaussians) 
    carmen_localize_subscribe_particle_message
      (&particle_msg, NULL, CARMEN_SUBSCRIBE_LATEST);
  if (nav_panel_config->show_lasers) 
    carmen_localize_subscribe_sensor_message(&sensor_msg, NULL, 
					     CARMEN_SUBSCRIBE_LATEST);
}

static GtkWidget *
new_label(char *s, GtkWidget *box) 
{
  GtkWidget *the_new_label;

  the_new_label = gtk_label_new(s);
  gtk_box_pack_start(GTK_BOX(box), the_new_label, TRUE, TRUE, 0);

  return the_new_label;
}

static GtkWidget *
construct_status_frame(GtkWidget *parent) 
{
  GtkWidget *status_frame;
  GtkWidget *status_box;

  status_frame = gtk_frame_new("Status");
  gtk_container_set_border_width (GTK_CONTAINER (status_frame), 10);
  gtk_container_add(GTK_CONTAINER(parent), status_frame);

  status_box = gtk_vbox_new(FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (status_box), 10);
  gtk_container_add(GTK_CONTAINER(status_frame), status_box);

  map_status_label = new_label("No map", status_box);  

  return status_box;
}

static void
do_redraw(void)
{
  if (display_needs_updating &&
      (carmen_get_time() - time_of_last_redraw > 0.3 || ALWAYS_REDRAW))
    {
      carmen_map_graphics_redraw(map_view);
      time_of_last_redraw = carmen_get_time();
      display_needs_updating = 0;
    }
}

static int received_robot_pose(void)
{
  return (robot.pose.x > 0 && robot.pose.y > 0 && robot.map != NULL);
}

static void draw_particles(GtkMapViewer *the_map_view, double pixel_size)
{
  int index;
  carmen_map_point_t map_particle;
  carmen_world_point_t particle;

  if (!nav_panel_config->show_particles || particle_msg.particles == NULL)
    return;

  map_particle.map = the_map_view->internal_map;
  
  for(index = 0; index < particle_msg.num_particles; index++) {
    particle.pose.x = particle_msg.particles[index].x;
    particle.pose.y = particle_msg.particles[index].y;
    particle.map = the_map_view->internal_map;
    carmen_map_graphics_draw_circle(the_map_view, &robot_colour, TRUE, 
				    &particle, pixel_size);
  }

}

static void draw_gaussians(GtkMapViewer *the_map_view, 
			   double pixel_size __attribute__ ((unused)))
{
  carmen_world_point_t mean;

  if (!nav_panel_config->show_gaussians || particle_msg.particles == NULL)
    return;
    
  mean = robot;
  mean.pose.x = globalpos->globalpos.x;
  mean.pose.y = globalpos->globalpos.y;
  mean.pose.theta = globalpos->globalpos.theta;
  
  carmen_map_graphics_draw_ellipse
    (the_map_view, &carmen_black, &mean, 
     carmen_square(globalpos->globalpos_std.x), 
     globalpos->globalpos_xy_cov, 
     carmen_square(globalpos->globalpos_std.y), 4);    
}

static void draw_lasers(GtkMapViewer *the_map_view, double pixel_size)
{
  int dot_size;
  int index;
  carmen_world_point_t particle;
  double angle;

  dot_size = 3*pixel_size;

  if (!nav_panel_config->show_lasers)
    return;

  particle = robot; 
  for(index = 0; index < sensor_msg.num_readings; 
      index += sensor_msg.laser_skip) {
    angle = sensor_msg.pose.theta - M_PI_2 + 
      index / (float)(sensor_msg.num_readings - 1) * M_PI;
    particle.pose.x = sensor_msg.pose.x + sensor_msg.range[index] *
      cos(angle);
    particle.pose.y = sensor_msg.pose.y + sensor_msg.range[index] *
      sin(angle);
    if(sensor_msg.mask[index]) 
      carmen_map_graphics_draw_circle(the_map_view, 
				      &carmen_green, TRUE, 
				      &particle, dot_size);
    else
      carmen_map_graphics_draw_circle(the_map_view, 
				      &carmen_yellow, TRUE, 
				      &particle, dot_size);
  }
      
#ifdef blah
  /* rear laser */
  for(index = 0; index < sensor_msg.rear_laser_scan.num_readings; 
      index++) {
    colour = sensor_msg.rear_laser_scan.scan[index].mean_prob *
      (GRADIENT_COLORS - 1);
    
    particle.pose.x = sensor_msg.rear_laser_scan.scan[index].mean_x;
    particle.pose.y = sensor_msg.rear_laser_scan.scan[index].mean_y;
    carmen_map_graphics_draw_circle(the_map_view, 
				    &RedBlueGradient[colour], TRUE, 
				    &particle, dot_size);
  }
#endif
}

static double x_coord(double x, double y, carmen_world_point_t *offset)
{
  return x*cos(offset->pose.theta)-y*sin(offset->pose.theta)+offset->pose.x;
}

static double y_coord(double x, double y, carmen_world_point_t *offset)
{
  return x*sin(offset->pose.theta)+y*cos(offset->pose.theta)+offset->pose.y;
}

static void draw_robot_shape(GtkMapViewer *the_map_view, 
			     carmen_world_point_t *location, int filled, 
			     GdkColor *colour, double pixel_size)
{
  double robot_radius;
  carmen_world_point_t wp[5];
  double width2, length2;

  if (!robot_config->rectangular) {
    robot_radius = robot_config->width/2.0;
    if (robot_radius < pixel_size*5)
      robot_radius = pixel_size*5;
    
    carmen_map_graphics_draw_circle(the_map_view, colour, filled, 
				    location, robot_radius);
    return;
  } 

  width2 = robot_config->width/2;
  length2 = robot_config->length/2;

  if (width2 < pixel_size*5)
    width2 = pixel_size*5;
  if (length2 < pixel_size*5)
    length2 = pixel_size*5;

  wp[0].pose.x = x_coord(length2, width2, location);
  wp[0].pose.y = y_coord(length2, width2, location);
  wp[1].pose.x = x_coord(length2, -width2, location);
  wp[1].pose.y = y_coord(length2, -width2, location);
  wp[2].pose.x = x_coord(-length2, -width2, location);
  wp[2].pose.y = y_coord(-length2, -width2, location);
  wp[3].pose.x = x_coord(-length2, width2, location);
  wp[3].pose.y = y_coord(-length2, width2, location);
  wp[4].pose.x = wp[0].pose.x;
  wp[4].pose.y = wp[0].pose.y;

  wp[0].map = wp[1].map = wp[2].map = wp[3].map = wp[4].map = location->map;

  carmen_map_graphics_draw_polygon(the_map_view, colour, wp, 5, filled);
}

static void draw_robot(GtkMapViewer *the_map_view, double pixel_size)
{
  carmen_world_point_t robot_radius;

  if (!nav_panel_config->show_particles && !nav_panel_config->show_gaussians) 
    draw_robot_shape(the_map_view, &robot, TRUE, &robot_colour, pixel_size);

  if (!nav_panel_config->show_gaussians)
    draw_robot_shape(the_map_view, &robot, FALSE, &carmen_black, pixel_size);
  
  robot_radius = robot;  
  robot_radius.pose.x = robot_radius.pose.x + 
    cos(robot_radius.pose.theta)*0.2;
  robot_radius.pose.y = robot_radius.pose.y + 
    sin(robot_radius.pose.theta)*0.2;
  
  carmen_map_graphics_draw_line(the_map_view, &carmen_black, &robot, 
				&robot_radius);  
}

static void draw_simulated_robot(GtkMapViewer *the_map_view, double pixel_size)
{
  double robot_size;
  carmen_world_point_t radius;

  if (!nav_panel_config->show_true_pos || simulator_trueposition.map == NULL)
    return;

  robot_size = robot_config->width/2.0;
  if (robot_size < pixel_size*5)
    robot_size = pixel_size*5;
  
  draw_robot_shape(the_map_view, &simulator_trueposition, TRUE, &carmen_blue,
		   pixel_size);
  draw_robot_shape(the_map_view, &simulator_trueposition, FALSE, &carmen_black,
		   pixel_size);
  
  radius = simulator_trueposition;  
  radius.pose.x = radius.pose.x + 
    cos(radius.pose.theta)*0.2;
  radius.pose.y = radius.pose.y + 
    sin(radius.pose.theta)*0.2;
  
  carmen_map_graphics_draw_line(the_map_view, &carmen_black, 
				&simulator_trueposition, &radius);  
}

void draw_robot_objects(GtkMapViewer *the_map_view) 
{
  int index;
  //  int colour;
  carmen_world_point_t path_x_1, path_x_2;
  carmen_world_point_t particle;
  double goal_size, pixel_size, robot_size, circle_size;
  GdkColor *colour = &carmen_black;
  carmen_world_point_t *draw_point = NULL;
  carmen_traj_point_t *simulator_object;

#ifdef USE_DOT
  carmen_world_point_t object_pt; 
  carmen_dot_person_t *person;
  carmen_dot_trash_t *trash_bin;
  carmen_dot_door_t *door;
  //  double vx, vxy, vy;
#endif

  if (the_map_view->internal_map == NULL)
    return;

  pixel_size = carmen_fmax
    (map_view->internal_map->config.x_size/(double)map_view->port_size_x,
     map_view->internal_map->config.y_size/(double)map_view->port_size_y);
  
  pixel_size *= map_view->internal_map->config.resolution * 
    (map_view->zoom/100.0);

  /* 
   * Draw robot features
   */

  if (received_robot_pose()) {
    draw_particles(the_map_view, pixel_size);
    draw_gaussians(the_map_view, pixel_size);
    draw_lasers(the_map_view, pixel_size);
    draw_robot(the_map_view, pixel_size);
  } 
  
  /* 
   * Draw path 
   */

  for (index = 1; index < num_path_points; index++) {
    if (path->map == NULL)
      break;
      
    carmen_map_graphics_draw_line(the_map_view, &path_colour, path+index-1, 
				  path+index);
      
    if (nav_panel_config->draw_waypoints) {
      path_x_1 = *(path+index);
      path_x_2 = *(path+index);
      
      path_x_1.pose.x -= path->map->config.resolution;
      path_x_1.pose.y -= path->map->config.resolution;
      path_x_2.pose.x += path->map->config.resolution;
      path_x_2.pose.y += path->map->config.resolution;
      carmen_map_graphics_draw_line(the_map_view, &path_colour, &path_x_1, 
				    &path_x_2);
      
      path_x_1.pose.y += path->map->config.resolution*2;
      path_x_2.pose.y -= path->map->config.resolution*2;
      carmen_map_graphics_draw_line(the_map_view, &path_colour, &path_x_1, 
				    &path_x_2);
    }
  }
  
  /* 
   * Draw goal
   */

  if (goal.pose.x > 0 && goal.pose.y > 0 && goal.map != NULL) {
    goal_size = nav_config->goal_size/2;
    if (goal_size < pixel_size*5)
      goal_size = pixel_size*5;
    
    carmen_map_graphics_draw_circle(the_map_view, &goal_colour, TRUE, &goal, 
				    goal_size);
    carmen_map_graphics_draw_circle(the_map_view, &carmen_black, FALSE, 
				    &goal, goal_size);
  }

  /* 
   * Draw simulator features
   */

  draw_simulated_robot(the_map_view, pixel_size);
  
  if (nav_panel_config->show_simulator_objects) {
    circle_size = robot_config->width/2.0;
    if (circle_size < pixel_size*5)
      circle_size = pixel_size*5;
    
    particle.map = the_map_view->internal_map;
    if (simulator_objects) {
      for (index = 0; index < simulator_objects->length; index++) {
	simulator_object = (carmen_traj_point_t *)
	  carmen_list_get(simulator_objects, index);
	particle.pose.x = simulator_object->x;
	particle.pose.y = simulator_object->y;
	if (black_and_white)
	  carmen_map_graphics_draw_circle(the_map_view, &carmen_grey, TRUE, 
					  &particle, circle_size);
	else
	  carmen_map_graphics_draw_circle(the_map_view, &carmen_orange, TRUE,
					  &particle, circle_size);
	carmen_map_graphics_draw_circle(the_map_view, &carmen_black, FALSE, 
					&particle, circle_size);
      }
    }
  }
  
#ifdef USE_DOT
  if (nav_panel_config->show_tracked_objects) {
    object_pt.map = map_view->internal_map;
    for (index = 0; people && index < people->length; index++) {
      person = (carmen_dot_person_t *)carmen_list_get(people, index);
      object_pt.pose.x = person->x;
      object_pt.pose.y = person->y;
      if (1)
	carmen_map_graphics_draw_ellipse(the_map_view, &people_colour, 
					 &object_pt, person->vx,
					 person->vxy, person->vy, 3);
      else
	carmen_map_graphics_draw_circle(the_map_view, &people_colour, FALSE, 
					&object_pt, 1);
    }
    for (index = 0; trash && index < trash->length; index++) {
      trash_bin = (carmen_dot_trash_t *)carmen_list_get(trash, index);
      object_pt.pose.x = trash_bin->x;
      object_pt.pose.y = trash_bin->y;
      //      carmen_eigs_to_covariance(trash_bin->theta, trash_bin->major/2, 
      //				trash_bin->minor, &vx, &vxy, &vy);
      carmen_map_graphics_draw_ellipse(the_map_view, &trash_colour, 
				       &object_pt, trash_bin->vx, 
				       trash_bin->vxy, trash_bin->vy, 1);
    }
    for (index = 0; doors && index < doors->length; index++) {
      door = (carmen_dot_door_t *)carmen_list_get(doors, index);
      object_pt.pose.x = door->x;
      object_pt.pose.y = door->y;
      if (1)
	carmen_map_graphics_draw_ellipse(the_map_view, &door_colour, 
					 &object_pt, door->vx,
					 door->vxy, door->vy, 3);
      else
	carmen_map_graphics_draw_circle(the_map_view, &door_colour, FALSE, 
					&object_pt, 1);
    }
  }
#endif

  if (placement_status != ORIENTING_ROBOT && 
      placement_status != ORIENTING_PERSON &&
      placement_status != ORIENTING_SIMULATOR) 
    return;

  /* Everything from here down is only used if we are orienting something.
     We have to draw the object itself (the robot, person, whatever) since
     in some cases the display hasn't actually published the fact that the
     feature has changed. 
   */

  if (placement_status == ORIENTING_ROBOT) {
    if (carmen_get_time() - last_navigator_update > 30 &&
	carmen_get_time() - last_simulator_update < 30)
      draw_point = &simulator_trueposition;
    else
      draw_point = &robot;
    colour = &carmen_red;
  } else if (placement_status == ORIENTING_PERSON) {
    draw_point = &new_person;
    colour = &carmen_orange;
  } else if (placement_status == ORIENTING_SIMULATOR) {
    draw_point = &new_simulator;
    colour = &carmen_blue;
  }

  robot_size = robot_config->width/2.0;
  if (robot_size < pixel_size*5)
    robot_size = pixel_size*5;
  
  if (black_and_white) {
    carmen_map_graphics_draw_circle(the_map_view, &carmen_grey, TRUE, 
				    draw_point, robot_size);
    carmen_map_graphics_draw_circle(the_map_view, &carmen_black, FALSE, 
				    draw_point, robot_size);
    
    carmen_map_graphics_draw_line(the_map_view, &carmen_black, draw_point, 
				  &cursor_pos);
  } else {
    carmen_map_graphics_draw_circle(the_map_view, colour, TRUE, 
				    draw_point, robot_size);
    carmen_map_graphics_draw_circle(the_map_view, &carmen_black, FALSE, 
				    draw_point, robot_size);
    
    carmen_map_graphics_draw_line(the_map_view, colour, draw_point, 
				  &cursor_pos);
  }
}

static gint
motion_handler (GtkMapViewer *the_map_view, carmen_world_point_p world_point, 
		GdkEventMotion *event __attribute__ ((unused))) 
{
  char buffer[255];
  carmen_map_point_t point;
  carmen_map_p the_map;

  the_map = the_map_view->internal_map;

  carmen_world_to_map(world_point, &point);
  sprintf(buffer, "Grid Cell: %d, %d (%.1f m, %.1f m)", point.x, point.y,
	  point.x * the_map->config.resolution, point.y * 
	  the_map->config.resolution);
  gtk_label_set_text(GTK_LABEL(cursor_status_label), buffer);  
  if (the_map != NULL) 
    {
      sprintf(buffer, "Value: %.2f", the_map->map[point.x][point.y]);
      gtk_label_set_text(GTK_LABEL(value_label), buffer);  
    }

  if (placement_status == ORIENTING_ROBOT || placement_status == ORIENTING_SIMULATOR ||
      placement_status == ORIENTING_SIMULATOR) 
    {
      cursor_pos = *world_point;
      display_needs_updating = 1;
      do_redraw();
    }

  return TRUE;
}

void 
resend_coords(GtkWidget *widget __attribute__ ((unused)), 
	      gpointer data __attribute__ ((unused))) 
{
  carmen_verbose("Robot: %.0f %.0f Goal: %.0f %.0f\n", last_robot.pose.x, 
		 last_robot.pose.y, goal.pose.x, goal.pose.y);
  if (goal.pose.x > 0)
    navigator_set_goal(goal.pose.x, goal.pose.y);
  if (last_robot.pose.x > 0)
    navigator_update_robot(&last_robot);
}

static int 
button_press_handler(GtkMapViewer *the_map_view __attribute__ ((unused)), 
		     carmen_world_point_p world_point __attribute__ ((unused)), 
		     GdkEventButton *event __attribute__ ((unused))) 
{

  if (the_map_view->internal_map == NULL)
    return TRUE;
  
  return TRUE;
}

static int 
button_release_handler(GtkMapViewer *the_map_view, 
		       carmen_world_point_p world_point, 
		       GdkEventButton *event __attribute__ ((unused))) 
{
  double angle, speed;
  GdkCursor *cursor;

  if (the_map_view->internal_map == NULL)
    return TRUE;
  
  if (placement_status == PLACING_ROBOT ||
      (placement_status == NO_PLACEMENT && 
       ((event->button == 1 && (event->state & GDK_CONTROL_MASK)) ||
	(event->button == 3)))) {
    if (GTK_TOGGLE_BUTTON (autonomous_button)->active) {
      placement_status = NO_PLACEMENT;
      return TRUE;
    }

    world_point->pose.theta = robot.pose.theta;
    robot = *world_point; 
    last_robot = *world_point;
    navigator_update_robot (world_point);
    if (placement_status == PLACING_ROBOT) {
      placement_status = ORIENTING_ROBOT;
      cursor = gdk_cursor_new(GDK_EXCHANGE);
      gdk_window_set_cursor (the_map_view->image_widget->window, cursor);
    } else {
      cursor = gdk_cursor_new(GDK_LEFT_PTR);
      gdk_window_set_cursor (the_map_view->image_widget->window, cursor);
    }
    return TRUE;
  }

  if (placement_status == PLACING_GOAL || 
      (placement_status == NO_PLACEMENT && event->button == 1)) {
    placement_status = NO_PLACEMENT;
    
    if (GTK_TOGGLE_BUTTON (autonomous_button)->active) 
      return TRUE;
    
    navigator_set_goal(world_point->pose.x, world_point->pose.y);
    cursor = gdk_cursor_new(GDK_LEFT_PTR);
    gdk_window_set_cursor (the_map_view->image_widget->window, cursor);
    return TRUE;
  }
  
  if (placement_status == PLACING_PERSON) {
    new_person = *world_point;
    cursor = gdk_cursor_new(GDK_EXCHANGE);
    gdk_window_set_cursor (the_map_view->image_widget->window, cursor);
    placement_status = ORIENTING_PERSON;
    return TRUE;
  }

  if (placement_status == PLACING_SIMULATOR) {
    new_simulator = *world_point;
    cursor = gdk_cursor_new(GDK_EXCHANGE);
    gdk_window_set_cursor (the_map_view->image_widget->window, cursor);
    placement_status = ORIENTING_SIMULATOR;
    return TRUE;
  }
  
  if (placement_status == ORIENTING_ROBOT || 
      (placement_status == NO_PLACEMENT && 
       ((event->button == 2 && (event->state & GDK_CONTROL_MASK)) ||
	(event->button == 3 && (event->state & GDK_CONTROL_MASK))))) { 
    placement_status = NO_PLACEMENT;
    
    if (GTK_TOGGLE_BUTTON (autonomous_button)->active) 
      return TRUE;    
    
    if (carmen_get_time() - last_navigator_update > 30 &&
	carmen_get_time() - last_simulator_update < 30) {
      angle = atan2(world_point->pose.y - simulator_trueposition.pose.y, 
		    world_point->pose.x - simulator_trueposition.pose.x);
      simulator_trueposition.pose.theta = angle;
      navigator_update_robot(&simulator_trueposition);
    } else {
      angle = atan2(world_point->pose.y - robot.pose.y, 
		    world_point->pose.x - robot.pose.x);    
      robot.pose.theta = angle;
      last_robot = robot;
      navigator_update_robot(&robot);
    }
    cursor = gdk_cursor_new(GDK_LEFT_PTR);
    gdk_window_set_cursor (the_map_view->image_widget->window, cursor);
  }

  if (placement_status == ORIENTING_PERSON) {
    placement_status = NO_PLACEMENT;
    
    angle = atan2(world_point->pose.y - new_person.pose.y, 
		  world_point->pose.x - new_person.pose.x);    
    speed = hypot(world_point->pose.y - new_person.pose.y, 
		  world_point->pose.x - new_person.pose.x);
    speed /= 10;
    new_person.pose.theta = angle;
    carmen_simulator_set_object(&(new_person.pose), speed, 
				CARMEN_SIMULATOR_RANDOM_OBJECT);
    cursor = gdk_cursor_new(GDK_LEFT_PTR);
    gdk_window_set_cursor (the_map_view->image_widget->window, cursor);
    return TRUE;
  }
  
  if (placement_status == ORIENTING_SIMULATOR) {
    placement_status = NO_PLACEMENT;
    angle = atan2(world_point->pose.y - new_person.pose.y, 
		  world_point->pose.x - new_person.pose.x);    
    new_simulator.pose.theta = angle;
    carmen_simulator_set_truepose(&(new_simulator.pose));
    cursor = gdk_cursor_new(GDK_LEFT_PTR);
    gdk_window_set_cursor (the_map_view->image_widget->window, cursor);
    return TRUE;
  }
  
  return TRUE;
}

static void
initialize_position(carmen_world_point_p point)
{
  point->pose.x = -1;
  point->pose.y = -1;
  point->pose.theta = 0.0;
  point->map = map_view->internal_map;
}

static void 
set_goal(GtkWidget *w __attribute__ ((unused)), int place_index) 
{
  navigator_set_goal_by_place(placelist->places+place_index);
}

static void 
set_location(GtkWidget *w __attribute__ ((unused)), int place_index) 
{
  carmen_world_point_t point;

  if (place_index == 0) 
    {
      carmen_verbose("Global localization\n"); 
      navigator_update_robot(NULL);
      return;
    }
  
  carmen_verbose("%f %f\n", placelist->places[place_index].x, 
		 placelist->places[place_index].y);  
  if (placelist->places[place_index].type == CARMEN_NAMED_POSITION_TYPE) 
    {
      point.pose.x = placelist->places[place_index].x;
      point.pose.y = placelist->places[place_index].y;
      point.pose.theta = robot.pose.theta;
    } 
  else 
    {
      point.pose.x = placelist->places[place_index].x;
      point.pose.y = placelist->places[place_index].y;
      point.pose.theta = placelist->places[place_index].theta;
    }
  navigator_update_robot(&point);
}

static gint
save_image(gpointer data __attribute__ ((unused)),
	   guint action __attribute__ ((unused)),
	   GtkWidget *widget  __attribute__ ((unused)))
{
  int x_size, y_size;
  int x_start, y_start;
  static int counter = 0;
  char filename[255];

  x_start = map_view->x_scroll_adj->value;
  y_start = map_view->y_scroll_adj->value;
  x_size = carmen_fmin(map_view->screen_defn.width, map_view->port_size_x);
  y_size = carmen_fmin(map_view->screen_defn.height, map_view->port_size_y);

  sprintf(filename, "%s%d.png", 
	  carmen_extract_filename(map_view->internal_map->config.map_name), 
	  counter++);

  if (display == CARMEN_NAVIGATOR_ENTROPY_v)
    carmen_graphics_write_pixmap_as_png(map_view->drawing_pixmap, filename, 
					x_start, y_start, x_size, y_size);
  else if (display == CARMEN_NAVIGATOR_UTILITY_v)
    carmen_graphics_write_pixmap_as_png(map_view->drawing_pixmap, filename, 
					x_start, y_start, x_size, y_size);
  else {
    carmen_graphics_write_pixmap_as_png(map_view->drawing_pixmap, filename, 
					0, 0, x_size, y_size);
  }

  return 1;
}
  
int 
navigator_graphics_init(int argc, char *argv[], 
			carmen_localize_globalpos_message *msg,
			carmen_robot_config_t *robot_conf_param,
			carmen_navigator_config_t *nav_conf_param,
			carmen_navigator_panel_config_t *nav_panel_conf_param) 
{
  /* GtkWidget is the storage type for widgets */
  GtkWidget *main_box;
  GtkWidget *panel_box;
  GtkWidget *status_box;
  GtkWidget *button_box;
  GtkWidget *label_box;
  GtkWidget *menubar;
  GtkWidget *vseparator, *hseparator;
  /*   GtkWidget *resend_button; */
  GtkWidget *menu_item;
  int index;
  int sync_mode_var;

  gtk_init (&argc, &argv);
  gdk_imlib_init();

  carmen_graphics_setup_colors();
  robot_colour = DEFAULT_ROBOT_COLOUR;
  goal_colour = DEFAULT_GOAL_COLOUR;
  path_colour = DEFAULT_PATH_COLOUR;
  people_colour = DEFAULT_PEOPLE_COLOUR;
  trash_colour = DEFAULT_TRASH_COLOUR;
  door_colour = DEFAULT_DOOR_COLOUR;

  nav_panel_config = nav_panel_conf_param;
  if (nav_panel_config->initial_map_zoom < 1.0 || nav_panel_config->initial_map_zoom > 100.0)
    nav_panel_config->initial_map_zoom = 100.0;

  
  /* Create a new window */
  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);

  gtk_signal_connect (GTK_OBJECT (window), "destroy", 
		      GTK_SIGNAL_FUNC (gtk_main_quit), 
		      "WM destroy");

  gtk_window_set_title (GTK_WINDOW (window), "CARMEN Planner");
  gtk_signal_connect (GTK_OBJECT (window), "delete_event",
		      GTK_SIGNAL_FUNC (delete_event), NULL);
  gtk_container_set_border_width (GTK_CONTAINER (window), 0);

  main_box = gtk_vbox_new(FALSE, 0);
  gtk_container_border_width (GTK_CONTAINER (main_box), 0); 
  gtk_container_add (GTK_CONTAINER (window), main_box);

  get_main_menu (window, &menubar);
  gtk_box_pack_start (GTK_BOX (main_box), menubar, FALSE, FALSE, 0);

  panel_box = gtk_hbox_new(FALSE, 0);
  gtk_container_border_width (GTK_CONTAINER (panel_box), 5); 
  gtk_container_add (GTK_CONTAINER (main_box), panel_box);

  map_view = carmen_map_graphics_new_viewer(400, 400, nav_panel_config->initial_map_zoom);
  gtk_box_pack_start(GTK_BOX (panel_box), map_view->map_box, TRUE, TRUE, 0);

  carmen_map_graphics_add_motion_event(map_view, 
				       (GtkSignalFunc)motion_handler);
  carmen_map_graphics_add_button_release_event
    (map_view, (GtkSignalFunc)button_release_handler);
  carmen_map_graphics_add_button_press_event
    (map_view, (GtkSignalFunc)button_press_handler);
  carmen_map_graphics_add_drawing_func(map_view, 
				       (drawing_func)draw_robot_objects);

  vseparator = gtk_vseparator_new();
  gtk_widget_set_usize(vseparator, 5, 
		       map_view->image_widget->allocation.height);
  gtk_box_pack_start(GTK_BOX(panel_box), vseparator, FALSE, FALSE, 0);
  
  label_box = gtk_vbox_new(FALSE, 0);
  gtk_widget_set_usize(label_box, BUTTON_WIDTH, 
		       map_view->image_widget->allocation.height);
  gtk_container_border_width (GTK_CONTAINER (label_box), 0); 

  gtk_box_pack_start(GTK_BOX (panel_box), label_box, FALSE, FALSE, 0);
  
  status_box = construct_status_frame(label_box);

  robot_status_label = new_label("Robot position: 0 0", status_box);
  robot_speed_label = new_label("Velocity: 0 cm/s 0 deg/s", status_box);
  goal_status_label = new_label("Goal position: 0 0", status_box);
  cursor_status_label = new_label("Grid Cell:", status_box);
  value_label = new_label("Value: 0.0", status_box);

  button_box = gtk_vbutton_box_new();
  gtk_container_border_width (GTK_CONTAINER (button_box), 5); 
  gtk_box_pack_start (GTK_BOX (label_box), button_box, FALSE, FALSE, 0);
  gtk_button_box_set_layout (GTK_BUTTON_BOX (button_box), GTK_BUTTONBOX_START);
  gtk_button_box_set_spacing (GTK_BUTTON_BOX (button_box), 10);
  gtk_button_box_set_child_size (GTK_BUTTON_BOX (button_box), BUTTON_WIDTH, 
				 BUTTON_HEIGHT);

  place_robot_button = gtk_button_new_with_label ("Place Robot");
  gtk_signal_connect (GTK_OBJECT (place_robot_button), "clicked",
		      GTK_SIGNAL_FUNC (place_robot), NULL);
  gtk_box_pack_start(GTK_BOX(button_box), place_robot_button, FALSE, FALSE, 0);
  place_goal_button = gtk_button_new_with_label ("Place Goal");
  gtk_signal_connect (GTK_OBJECT (place_goal_button), "clicked",
		      GTK_SIGNAL_FUNC (place_goal), NULL);
  gtk_box_pack_start(GTK_BOX(button_box), place_goal_button, FALSE, FALSE, 0);
  autonomous_button = gtk_toggle_button_new_with_label ("Go");
  gtk_signal_connect (GTK_OBJECT (autonomous_button), "clicked",
		      GTK_SIGNAL_FUNC (go_autonomous), (gpointer)"Autonomous");
  gtk_box_pack_start(GTK_BOX(button_box), autonomous_button, FALSE, FALSE, 0);

  hseparator = gtk_hseparator_new();
  gtk_box_pack_start (GTK_BOX (label_box), hseparator, FALSE, FALSE, 0);

  //  gtk_container_add(GTK_CONTAINER(label_box), hseparator);
  gtk_widget_set_usize(hseparator, BUTTON_WIDTH, 5);

  simulator_box = gtk_vbutton_box_new();
  gtk_container_border_width (GTK_CONTAINER (simulator_box), 5); 
  //  gtk_container_add (GTK_CONTAINER (label_box), simulator_box);
  gtk_box_pack_start (GTK_BOX (label_box), simulator_box, FALSE, FALSE, 0);

  gtk_button_box_set_layout (GTK_BUTTON_BOX (simulator_box), 
			     GTK_BUTTONBOX_START);
  gtk_button_box_set_spacing (GTK_BUTTON_BOX (simulator_box), 10);
  gtk_button_box_set_child_size (GTK_BUTTON_BOX (simulator_box), BUTTON_WIDTH, 
				 BUTTON_HEIGHT);

  place_simulator_button = gtk_button_new_with_label ("Place Simulator");
  gtk_signal_connect (GTK_OBJECT (place_simulator_button), "clicked",
		      GTK_SIGNAL_FUNC (place_simulator), NULL);
  gtk_box_pack_start(GTK_BOX(simulator_box), place_simulator_button, 
		     FALSE, FALSE, 0);
  next_tick_button = gtk_button_new_with_label ("Next Tick");
  gtk_signal_connect (GTK_OBJECT (next_tick_button), "clicked",
		      GTK_SIGNAL_FUNC (next_tick), NULL);
  gtk_box_pack_start(GTK_BOX(simulator_box), next_tick_button, 
		     FALSE, FALSE, 0);
  sync_mode_button = gtk_toggle_button_new_with_label ("Sync Mode");
  carmen_param_set_module(NULL);
  carmen_param_get_onoff("simulator_sync_mode", &sync_mode_var, NULL);
  carmen_param_subscribe_onoff("simulator", "sync_mode", NULL,
			       (carmen_param_change_handler_t)
			       sync_mode_change_handler);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON (sync_mode_button), 
			       sync_mode_var);
  gtk_signal_connect (GTK_OBJECT (sync_mode_button), "clicked",
		      GTK_SIGNAL_FUNC (sync_mode), (gpointer)"Autonomous");
  gtk_box_pack_start(GTK_BOX(simulator_box), sync_mode_button, FALSE, 
		     FALSE, 0);

  filler_box = gtk_vbox_new(FALSE, 0);
  gtk_widget_set_usize(filler_box, BUTTON_WIDTH, 100);
  gtk_container_border_width (GTK_CONTAINER (filler_box), 0); 
  //  gtk_container_add (GTK_CONTAINER (label_box), filler_box);
  gtk_box_pack_start (GTK_BOX (label_box), filler_box, FALSE, FALSE, 0);

  gtk_widget_show (filler_box);
	
  for(index = 0; index < GRADIENT_COLORS; index++)
    RedBlueGradient[index] = 
      carmen_graphics_add_color_rgb(255 - 255 * index / (float)GRADIENT_COLORS,
				    0, 255 * index / (float)GRADIENT_COLORS);

  gtk_widget_show_all (window);
  gtk_widget_hide(simulator_box);
  simulator_hidden = 1;

  gtk_widget_grab_focus(window);

  menu_item = gtk_item_factory_get_item(item_factory, "/File/Stop Filming");
  gtk_widget_hide(menu_item);

  globalpos = msg;

  robot_config = robot_conf_param;
  nav_config = nav_conf_param;

  cursor_pos.map = NULL;
  return(0);
}

void
navigator_graphics_add_ipc_handler(GdkInputFunction handle_ipc)
{
  carmen_graphics_update_ipc_callbacks(handle_ipc);
}

void
navigator_graphics_change_map(carmen_map_p new_map)
{
  char buffer[1024];

  carmen_map_graphics_add_map(map_view, new_map, 0);

  initialize_position(&robot);
  initialize_position(&goal);
  if (people)
    people->length = 0;
  if (doors)
    doors->length = 0;
  if (trash)
    trash->length = 0;
  initialize_position(&last_robot);

  sprintf(buffer, "Map: %s", 
	  carmen_extract_filename(new_map->config.map_name));
  gtk_label_set_text(GTK_LABEL(map_status_label), buffer);
}

void 
navigator_graphics_display_map(float *data, carmen_navigator_map_t type) 
{
  char name[10];
  int flags = 0;

  display = type;
  switch (type) {
  case CARMEN_NAVIGATOR_MAP_v: 
    strcpy(name, "Map");    
    break;
  case CARMEN_NAVIGATOR_ENTROPY_v: 
    strcpy(name, "Entropy");
    flags = CARMEN_GRAPHICS_RESCALE;
    break;
  case CARMEN_NAVIGATOR_COST_v: 
    strcpy(name, "Costs");
    flags = CARMEN_GRAPHICS_RESCALE;
    break;
  case CARMEN_NAVIGATOR_UTILITY_v: 
    strcpy(name, "Utility");
    flags = CARMEN_GRAPHICS_RESCALE | CARMEN_GRAPHICS_INVERT;
    break;
  default:
    return;
  }

  if (black_and_white)
    flags |= CARMEN_GRAPHICS_BLACK_AND_WHITE;

  carmen_map_graphics_modify_map(map_view, data, flags);
}

void 
navigator_graphics_add_placelist(carmen_map_placelist_p new_placelist)
{
  char buffer[1024], buffer2[1024];
  char accelerator_buffer[3];
  GtkItemFactoryEntry new_entry;
  int index;
  char *underscore;

  carmen_verbose("Received %d places\n", new_placelist->num_places);

  if (placelist != NULL)
    {
      for (index = 0; index < placelist->num_places; index++)
	{
	  sprintf(buffer, "/Goals/%s", placelist->places[index].name);
	  gtk_item_factory_delete_item(item_factory, buffer);	  
	  sprintf(buffer, "/Start Location/%s", placelist->places[index].name);
	  gtk_item_factory_delete_item(item_factory, buffer);	  
	}
      free(placelist->places);
    } 
  else 
    {
      placelist = (carmen_map_placelist_p)
	calloc(1, sizeof(carmen_map_placelist_t));
      carmen_test_alloc(placelist);
    }

  placelist->num_places = new_placelist->num_places;

  if (placelist->num_places > 0) 
    {
      placelist->places = (carmen_place_p)calloc(placelist->num_places, 
						 sizeof(carmen_place_t));
      carmen_test_alloc(placelist->places);
      memcpy(placelist->places, new_placelist->places, 
	     sizeof(carmen_place_t)*placelist->num_places);
    }

  new_entry.path = buffer;
  new_entry.item_type = NULL;
  
  for (index = 0; index < placelist->num_places; index++)
    {
      strcpy(buffer2, placelist->places[index].name);
      buffer2[0] = toupper(buffer2[0]);
      do {
	underscore = strchr(buffer2, '_');
	if (underscore)
	  *underscore = ' ';
      } while (underscore);

      sprintf(buffer, "/Goals/%s", buffer2);
      new_entry.callback = set_goal;
      new_entry.callback_action = index;
      new_entry.accelerator = NULL;
      gtk_item_factory_create_item(item_factory, &new_entry, NULL, 1);

      sprintf(buffer, "/Start Location/%s", buffer2);
      new_entry.callback = set_location;
      new_entry.callback_action = index+1;
      sprintf(accelerator_buffer, "%d", index+1);
      new_entry.accelerator = accelerator_buffer;
      gtk_item_factory_create_item(item_factory, &new_entry, NULL, 1);      
    }
}

void navigator_graphics_update_dynamics(void) 
{
  display_needs_updating = 1;
  do_redraw();
}

void navigator_graphics_initialize_dynamics(carmen_list_t *new_people, 
					    carmen_list_t *new_trash, 
					    carmen_list_t *new_doors)
{
  people = new_people;
  trash = new_trash;
  doors = new_doors;
}

void 
navigator_graphics_update_display(carmen_traj_point_p new_robot, 
				  carmen_world_point_p new_goal, 
				  int autonomous) 
{
  char buffer[255];
  double robot_distance = 0.0, goal_distance = 0.0;
  carmen_world_point_t new_robot_w;
  static int previous_width = 0, previous_height = 0;
  double delta_angle;
  int autonomous_change = 0;
  double adjust_distance;

  if (!autonomous && 
      gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON (autonomous_button))) {
    ignore_click = 1;
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON (autonomous_button), 0);
    label_autonomy_button("Go");
    autonomous_change = 1;
  }

  if (autonomous && 
      !gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON (autonomous_button))) {
    ignore_click = 1;
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON (autonomous_button), 1);
    label_autonomy_button("Stop");
    autonomous_change = 1;
  }

  if (!map_view->internal_map)
    return;

  if (time_of_simulator_update - carmen_get_time() > 30)
    {
      gtk_widget_hide(simulator_box);
      gtk_widget_show(filler_box);
      simulator_hidden = 1;
    }

  adjust_distance = carmen_fmax
    (map_view->internal_map->config.x_size/(double)map_view->port_size_x,
     map_view->internal_map->config.y_size/(double)map_view->port_size_y);

  adjust_distance *= map_view->internal_map->config.resolution * 
    (map_view->zoom/100.0);
  adjust_distance *= 10;
  
  new_robot_w.pose.x = new_robot->x;
  new_robot_w.pose.y = new_robot->y;
  new_robot_w.pose.theta = new_robot->theta;
  new_robot_w.map = map_view->internal_map;
  robot_distance = carmen_distance_world(&new_robot_w, &(map_view->centre));

  if (nav_panel_config->track_robot && 
      (robot_distance > adjust_distance || 
       previous_width != map_view->image_widget->allocation.width ||
       previous_height != map_view->image_widget->allocation.height)) 
    carmen_map_graphics_adjust_scrollbars(map_view, &robot);    

  robot_distance = carmen_distance_world(&new_robot_w, &robot);
  delta_angle = 
    carmen_normalize_theta(new_robot_w.pose.theta-robot.pose.theta);

  robot = new_robot_w;
  robot_traj = *new_robot;

  if (new_goal) 
    {
      goal_distance = carmen_distance_world(new_goal, &goal);
      goal = *new_goal;
    }
  else
    goal_distance = 0.0;

  previous_width = map_view->image_widget->allocation.width;
  previous_height = map_view->image_widget->allocation.height;

  if (autonomous_change || robot_distance > 1.0 || goal_distance > 1.0 ||
      fabs(delta_angle) > carmen_degrees_to_radians(0.01) )
    display_needs_updating = 1;

  sprintf(buffer, "Robot: %5.1f m, %5.1f m, %6.2f", robot.pose.x, 
	  robot.pose.y, carmen_radians_to_degrees(robot.pose.theta));
  gtk_label_set_text(GTK_LABEL(robot_status_label), buffer);
  sprintf(buffer, "Velocity: %5.1f cm/s, %5.1f deg/s", robot_traj.t_vel,
	  carmen_radians_to_degrees(robot_traj.r_vel));
  gtk_label_set_text(GTK_LABEL(robot_speed_label), buffer);
  sprintf(buffer, "Goal: %.1f m, %.1f m", goal.pose.x, goal.pose.y);
  gtk_label_set_text(GTK_LABEL(goal_status_label), buffer);

  last_navigator_update = carmen_get_time();

  do_redraw();
}

void 
navigator_graphics_update_plan(carmen_traj_point_p new_plan, int plan_length)
{
  int index;

  if (map_view->internal_map == NULL)
    return;

  if (path != NULL) 
    {
      free(path);
      path = NULL;
    }

  num_path_points = plan_length;

  if (plan_length > 0) 
    {
      path = (carmen_world_point_t *)
	calloc(plan_length, sizeof(carmen_world_point_t));
      carmen_test_alloc(path);
      carmen_verbose("Got path of length %d\n", plan_length);
      for (index = 0; index < num_path_points; index++) 
	{
	  path[index].pose.x = new_plan[index].x;
	  path[index].pose.y = new_plan[index].y;
	  path[index].pose.theta = new_plan[index].theta;
	  path[index].map = map_view->internal_map;
	  carmen_verbose("%.1f %.1f\n", path[index].pose.x, 
			 path[index].pose.y);
	}
    } 
  else
    num_path_points = 0;

  display_needs_updating = 1;
  do_redraw();
}

carmen_world_point_p 
navigator_graphics_get_current_path(void)
{
  return path;
}

void
navigator_graphics_update_simulator_truepos(carmen_point_t truepose)
{
  time_of_simulator_update = carmen_get_time();
  if (simulator_hidden)
    {
      gtk_widget_show_all(simulator_box);
      gtk_widget_hide(filler_box);
      simulator_hidden = 0;
      if (!GTK_TOGGLE_BUTTON(sync_mode_button)->active)
	gtk_widget_hide(next_tick_button);
    }

  simulator_trueposition.pose = truepose;
  simulator_trueposition.map = map_view->internal_map;
  last_simulator_update = carmen_get_time();
}

void navigator_graphics_update_simulator_objects(int num_objects, 
						 carmen_traj_point_t 
						 *objects_list)
{
  int i;

  if (simulator_objects == NULL) {
    if (num_objects == 0)
      return;
    simulator_objects = carmen_list_create
      (sizeof(carmen_traj_point_t), num_objects);
  }

  simulator_objects->length = 0;

  for (i = 0; i < num_objects; i++) 
    carmen_list_add(simulator_objects, objects_list+i);

  display_needs_updating = 1;
  do_redraw();
}

static void 
sync_mode_change_handler(char *module __attribute__ ((unused)), 
			 char *variable __attribute__ ((unused)), 
			 char *value)
{
  int new_value;

  if (strlen(value) >=2 && strncmp(value, "on", 2) == 0)
    new_value = 1;
  else if (strlen(value) >= 3 && strncmp(value, "off", 3) == 0)
    new_value = 0;
  else
    return;

  GTK_TOGGLE_BUTTON(sync_mode_button)->active = new_value;

  if (simulator_hidden)
    return;

  if (GTK_TOGGLE_BUTTON(sync_mode_button)->active)
    gtk_widget_show(next_tick_button);
  else
    gtk_widget_hide(next_tick_button);
}

void 
navigator_graphics_start(void) 
{
  gtk_main ();
}
