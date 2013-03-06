#include <gtk/gtk.h>
#include <unistd.h>

#include "tools.h"
#include "vga.h"

static GtkWidget *main_window;
static int wnd_ready;

int window_ready(void) {
    return wnd_ready;
}

void draw_point(int x, int y, char r, char g, char b) {
    GtkWidget *widget = main_window;
    cairo_t *cr;

    if (!wnd_ready)
        return;

    //aquire thread lock
    gdk_threads_enter ();
    cr = gdk_cairo_create(widget->window);

    cairo_set_source_rgb(cr, r, g, b);
    //cairo_set_line_width (cr, 0.5);
    /*
    cairo_rectangle(cr, x, y, 3, 3);
    cairo_fill(cr);
    */
    cairo_move_to(cr, x, y);
    cairo_line_to(cr, x + 1, y);
    cairo_stroke(cr);

    cairo_destroy(cr);
    //leave thread.
    gdk_threads_leave ();

}

int window_start(int argc, char** argv) 
{
    //init thread and gtk lib.
    g_thread_init (NULL);
    gdk_threads_init ();
    gtk_init (&argc, &argv);

    //create window
    main_window = gtk_window_new (GTK_WINDOW_TOPLEVEL);

    //set pos
    gtk_widget_set_size_request(main_window, VGA_WIDTH, VGA_HEIGHT);
    gtk_window_set_position (GTK_WINDOW(main_window), GTK_WIN_POS_CENTER);

    //connect destructor
    g_signal_connect (G_OBJECT(main_window), "destroy", G_CALLBACK (gtk_main_quit), NULL);

    //show
    gtk_widget_show (main_window);
    wnd_ready = TRUE;

    //get thread lock
    gdk_threads_enter();
    gtk_main ();
    gdk_threads_leave();

    return 0;
}

int window_init(void) {
    main_window = NULL;
    wnd_ready = FALSE;
    return TRUE;
}

