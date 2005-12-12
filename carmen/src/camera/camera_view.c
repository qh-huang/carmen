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
#include <carmen/camera_interface.h>

static GtkWidget *drawing_area;
static GdkPixmap *image_pixmap;
static int received_image = 0;
static void redraw(void);

static void image_handler(carmen_camera_image_message *image)
{
  GdkImlibImage *imlib_image;
  int i, j;
  unsigned char *data;

  if (!received_image) {
    gtk_widget_set_usize (drawing_area, image->width, image->height);
  }

  data = (unsigned char *)calloc
    (image->width*image->height*3, sizeof(unsigned char));
  carmen_test_alloc(data);

  for (i = 0; i < image->width*image->height; i++) {
    for (j = 0; j < 3; j++) {
      data[3*i+j] = image->image[3*i+j];
    }
  }

  imlib_image = 
   gdk_imlib_create_image_from_data(data, (unsigned char *)NULL, 
				    image->width, image->height);
 
  gdk_imlib_render(imlib_image, image->width, image->height);
  image_pixmap = gdk_imlib_move_image(imlib_image);
  gdk_imlib_destroy_image(imlib_image);

  received_image = 1;
  redraw();
}

static void 
shutdown_camera_view(int x)
{
  if(x == SIGINT) {
    close_ipc();	
    printf("Disconnected from robot.\n");
    exit(1);
  }
}

static gint 
updateIPC(gpointer *data __attribute__ ((unused))) 
{
  sleep_ipc(0.01);
  carmen_graphics_update_ipc_callbacks((GdkInputFunction)updateIPC);
  return 1;
}

static gint 
expose_event(GtkWidget *widget __attribute__ ((unused)), 
	     GdkEventExpose *event __attribute__ ((unused))) 
{
  redraw();
  return 1;
}

static gint
key_press_event(GtkWidget *widget __attribute__ ((unused)), 
		GdkEventKey *key)
{
  if (toupper(key->keyval) == 'C' && (key->state & GDK_CONTROL_MASK))
    shutdown_camera_view(SIGINT);

  if (toupper(key->keyval) == 'Q' && (key->state & GDK_CONTROL_MASK))
    shutdown_camera_view(SIGINT);
  
  if (key->state || key->keyval > 255)
    return 1;

  return 1;
}

static gint 
key_release_event(GtkWidget *widget __attribute__ ((unused)), 
		  GdkEventButton *key __attribute__ ((unused)))
{
  return 1;
}

static void redraw(void)
{
  /* Make sure data structures are all the right size. */

  if (!received_image)
    return;

  gdk_draw_pixmap(drawing_area->window, 
		  drawing_area->style->fg_gc[GTK_WIDGET_STATE (drawing_area)],
                  image_pixmap, 0, 0, 0, 0, 
		  drawing_area->allocation.width, 
		  drawing_area->allocation.height);
}

static void 
start_graphics(int argc, char *argv[]) 
{
  GtkWidget *main_window;

  gtk_init(&argc, &argv);

  carmen_graphics_initialize_screenshot();

  main_window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title (GTK_WINDOW (main_window), "Robot Graph");
  
  drawing_area = gtk_drawing_area_new ();
  gtk_widget_set_usize (drawing_area, 100, 100);

  gtk_container_add(GTK_CONTAINER(main_window), drawing_area);

  gtk_signal_connect(GTK_OBJECT(drawing_area), "expose_event",
		     (GtkSignalFunc)expose_event, NULL);
  gtk_signal_connect(GTK_OBJECT(main_window), "key_press_event",
		     (GtkSignalFunc)key_press_event, NULL);
  gtk_signal_connect(GTK_OBJECT(main_window), "key_release_event",
		     (GtkSignalFunc)key_release_event, NULL);
  
  gtk_widget_add_events(drawing_area,  GDK_EXPOSURE_MASK 
			| GDK_BUTTON_PRESS_MASK 
			| GDK_BUTTON_RELEASE_MASK 
			| GDK_POINTER_MOTION_MASK
			| GDK_POINTER_MOTION_HINT_MASK
			| GDK_KEY_PRESS_MASK
			| GDK_KEY_RELEASE_MASK);
  
  carmen_graphics_update_ipc_callbacks((GdkInputFunction)updateIPC);
  gtk_widget_show(drawing_area);
  gtk_widget_show(main_window);

  gtk_main();
}

int 
main(int argc, char **argv)
{  

  carmen_initialize_ipc(argv[0]);
  carmen_param_check_version(argv[0]);

  carmen_camera_subscribe_images
    (NULL, (carmen_handler_t)image_handler, CARMEN_SUBSCRIBE_LATEST);

  signal(SIGINT, shutdown_camera_view);  

  start_graphics(argc, argv);

  return 0;
}
