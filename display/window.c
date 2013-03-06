#include <gtk/gtk.h>
#include <unistd.h>

#include "tools.h"
#include "vga.h"

static GtkWidget *main_window;
//static GtkWidget *draw_area;
static int wnd_ready;

int window_ready(void) {
    return wnd_ready;
}

void draw_point(int x, int y, char r, char g, char b) {
    cairo_t *cr;

    GtkWidget *widget = main_window;

           gdk_threads_enter ();
    cr = gdk_cairo_create(widget->window);

    GdkColor color ;

    if (!wnd_ready)
        return;

    color.red = r;
    color.green = g;
    color.blue = b ;

    //cairo_translate(cr, 0, 7);

    cairo_set_source_rgb(cr, r, g, b);
    //cairo_paint(cr);

    cairo_rectangle(cr, x, y, 3, 3);
    //cairo_rectangle(cr, 42, 40, 30, 3);

    cairo_fill(cr);

    cairo_destroy(cr);
           gdk_threads_leave ();

}

#if 0
void draw_point(int x, int y, char r, char g, char b) {
    GdkGC *gc;
    GdkColor color ;

    if (!wnd_ready)
        return;

    color.red = r;
    color.green = g;
    color.blue = b ;

    gc = gdk_gc_new(draw_area->window);
    gdk_gc_set_rgb_fg_color(gc, &color);

    gdk_draw_point(draw_area->window , gc, x, y);

    g_object_unref(gc);

}
#endif

int window_start(int argc, char** argv) 
{
    g_thread_init (NULL);
    gdk_threads_init ();

    gtk_init (&argc, &argv);

    main_window = gtk_window_new (GTK_WINDOW_TOPLEVEL);

    gtk_widget_set_size_request(main_window, VGA_WIDTH, VGA_HEIGHT);
    gtk_window_set_position (GTK_WINDOW(main_window), GTK_WIN_POS_CENTER);

    /*
    draw_area = gtk_drawing_area_new();
    gtk_container_add(GTK_CONTAINER(main_window), draw_area);
    g_signal_connect(G_OBJECT(draw_area), "expose-event", 
            G_CALLBACK(expose_event), NULL);
*/
    g_signal_connect (G_OBJECT(main_window), "destroy", G_CALLBACK (gtk_main_quit), NULL);

    gtk_widget_show (main_window);
    wnd_ready = TRUE;
      gdk_threads_enter();
    gtk_main ();
      gdk_threads_leave();

    return 0;
}

int window_init(void) {
    main_window = NULL;
    //draw_area = NULL;
    wnd_ready = FALSE;
    return TRUE;
}

#if 0
int window_start(int argc, char** argv) 
    ;

#include <gtk/gtk.h>
void enter_button(GtkWidget *widget, gpointer data) 
{ 
    GdkColor color;
    color.red = 27000;
    color.green = 30325;
    color.blue = 34181;
    gtk_widget_modify_bg(widget, GTK_STATE_PRELIGHT, &color);

    int x, y;
    char r, g, b;

    x = y = 0;
    r = g = b = 0;

    while (x < 1000) {
        draw_point(x++ % (VGA_WIDTH - 10) + 5, y++ % (VGA_HEIGHT - 10) + 5, 
                r++, g++, b++);
    }
}


int window_start(int argc, char** argv) 
{

    GtkWidget *window;
    GtkWidget *fixed;
    GtkWidget *button;

    gtk_init(&argc, &argv);

    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
    gtk_window_set_default_size(GTK_WINDOW(window), 230, 150);
    gtk_window_set_title(GTK_WINDOW(window), "enter signal");

    fixed = gtk_fixed_new();
    //gtk_container_add(GTK_CONTAINER(window), fixed);
    //draw_area = gtk_drawing_area_new();
    draw_area = fixed;
    gtk_container_add(GTK_CONTAINER(window), draw_area);

    button = gtk_button_new_with_label("Button");
    gtk_widget_set_size_request(button, 80, 35);
    gtk_fixed_put(GTK_FIXED(fixed), button, 50, 50);


    g_signal_connect(G_OBJECT(button), "enter", 
            G_CALLBACK(enter_button), NULL);

    g_signal_connect_swapped(G_OBJECT(window), "destroy",
            G_CALLBACK(gtk_main_quit), NULL);

    gtk_widget_show_all(window);

    gtk_main();

    return 0;
}

#endif

