#include <carmen/carmen_graphics.h>
#include "canon_interface.h"

GtkWidget *drawing_area;
GdkGC *gc = NULL;

carmen_canon_preview_message preview;

unsigned char *image = NULL;
int image_rows, image_cols;

void redraw(void)
{
  if(image != NULL)
    gdk_draw_rgb_image(drawing_area->window, gc, 0, 0, 320, 240, 
		       GDK_RGB_DITHER_NONE, image, image_cols * 3);
}

void canon_preview_handler(void)
{
  static int image_count = 0;
  static double first_time = 0;

  if(image_count == 0)
    first_time = carmen_get_time_ms();
  image_count++;
  if(image_count % 30 == 0)
    fprintf(stderr, "Running at %.2f fps.\n", image_count / 
	    (carmen_get_time_ms() - first_time));
  if(image != NULL)
    free(image);
  read_jpeg_from_memory(preview.preview, preview.preview_length, 
			&image, &image_cols, &image_rows);
  redraw();
}

static gint expose_event(GtkWidget *widget __attribute__ ((unused)),
                         GdkEventExpose *event __attribute__ ((unused)))
{
  redraw();
  return FALSE;
}

static gint updateIPC(gpointer *data __attribute__ ((unused))) 
{
  sleep_ipc(0.01);
  carmen_graphics_update_ipc_callbacks((GdkInputFunction)updateIPC);
  return 1;
}

GtkWidget *create_drawing_window(char *name, int x, int y,
                                 int width, int height, GtkSignalFunc expose)
{
  GtkWidget *main_window, *drawing_area;
  
  /* Create the top-level window */
  main_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title(GTK_WINDOW(main_window), name);
  gtk_signal_connect(GTK_OBJECT(main_window), "delete_event",
                     GTK_SIGNAL_FUNC(gtk_main_quit), NULL);
  gtk_widget_set_uposition(main_window, x, y);
  /* Create the drawing area */
  drawing_area = gtk_drawing_area_new();
  gtk_signal_connect(GTK_OBJECT(drawing_area), "expose_event", expose, NULL);
  gtk_drawing_area_size(GTK_DRAWING_AREA(drawing_area), width, height);
  gtk_container_add(GTK_CONTAINER(main_window), drawing_area);
  gtk_widget_show(drawing_area);
  /* Show the top-level window */
  gtk_widget_show(main_window);
  return drawing_area;
}

void initialize_graphics(int *argc, char ***argv)
{
  gtk_init(argc, argv);
  gdk_imlib_init();
  gdk_rgb_init();
  carmen_graphics_update_ipc_callbacks((GdkInputFunction)updateIPC);
  carmen_graphics_initialize_screenshot();
  carmen_graphics_setup_colors();
  drawing_area = create_drawing_window("FastSLAM", 50, 50, 320, 240,
                                       (GtkSignalFunc)expose_event);
  gc = gdk_gc_new(drawing_area->window);
  gtk_signal_connect(GTK_OBJECT(drawing_area), "expose_event", 
                     (GtkSignalFunc)expose_event, NULL);
}

void shutdown_module(int x)
{
  if(x == SIGINT) {
    carmen_canon_stop_preview_command();
    close_ipc();
    exit(0);
  }
}

int main(int argc __attribute__ ((unused)), char **argv)
{
  carmen_initialize_ipc(argv[0]);
  carmen_param_check_version(argv[0]);
  
  carmen_canon_subscribe_preview_message(&preview, (carmen_handler_t)
					 canon_preview_handler,
					 CARMEN_SUBSCRIBE_LATEST);
  carmen_canon_start_preview_command();
  signal(SIGINT, shutdown_module);
  initialize_graphics(&argc, &argv);
  gtk_main();
  return 0;
}
