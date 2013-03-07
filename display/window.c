#include <gtk/gtk.h>
#include <unistd.h>
#include <string.h>

#include "tools.h"
#include "vga.h"

int pix_buf[VGA_WIDTH][VGA_HEIGHT][3];

void set_pixel_color(int x, int y, int r, int g, int b) {
    //g_print ("set pix...\n");
    pix_buf[x][y][0] = r;
    pix_buf[x][y][1] = g;
    pix_buf[x][y][2] = b;
}

GdkGC *gc = NULL;
GdkGC *set_color(gushort r, gushort g, gushort b)
{
    GdkColor color;

    color.red = r;
    color.green = g;
    color.blue = b;
    gdk_color_alloc(gdk_colormap_get_system(), &color);
    gdk_gc_set_foreground(gc, &color);
    return gc;
}

#if 0
void expose_event(GtkWidget *widget, GdkEventExpose *event, gpointer data){
    GdkDrawable *drawable = widget->window;

    int x, y;

    //g_print ("draw...\n");
    x = y = 0;
    for (y = 0; y < VGA_HEIGHT; y++) {
        for (x = 0; x < VGA_WIDTH; x++) {
            set_color(pix_buf[x][y][0],
                    pix_buf[x][y][1],
                    pix_buf[x][y][2]); 
            gdk_draw_point (drawable, gc, x, y);
        }

    }
}

static gboolean wnd_timer_func(GtkWidget *widget)
{
    if (widget->window == NULL)
    {
        return FALSE;
    }

    gdk_threads_enter ();
    //g_print ("timer...\n");
    //repaint buffer.
    gtk_widget_queue_draw(widget);
    //cairo_stroke(wnd_cairo);
    gdk_threads_leave ();

    return (TRUE);
}


//int main(int argc, char *argv[]){
int window_start(int argc, char** argv) 
{
    GtkWidget *window;
    GtkWidget *drawing_area;

    //init thread 
    g_thread_init (NULL);
    gdk_threads_init ();

    //get thread lock
    gdk_threads_enter();

    //init.
    gtk_init(&argc, &argv);

    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_signal_connect (GTK_OBJECT (window), "delete_event",
            GTK_SIGNAL_FUNC (gtk_main_quit), NULL);

    //get GDK drawing area.
    drawing_area = gtk_drawing_area_new();
    gtk_drawing_area_size(GTK_DRAWING_AREA(drawing_area), VGA_WIDTH, VGA_HEIGHT);
    gtk_window_set_position (GTK_WINDOW(window), GTK_WIN_POS_CENTER);

    gtk_container_add (GTK_CONTAINER (window), drawing_area);
    gtk_signal_connect (GTK_OBJECT (drawing_area), "expose_event",
            GTK_SIGNAL_FUNC(expose_event), NULL);

    //connect timer.
    g_timeout_add(1, (GSourceFunc)wnd_timer_func, window);

    gtk_widget_show_all(window);

    gc = gdk_gc_new(window->window);

    gtk_main();
    gdk_threads_leave();

    return 0;
}

#else

GdkPixmap *pixmap = NULL;
gint repaint(gpointer data){
    GtkWidget *drawing_area = GTK_WIDGET (data);

    int x, y;

    //g_print ("draw...\n");
    x = y = 0;
    for (y = 0; y < VGA_HEIGHT; y++) {
        for (x = 0; x < VGA_WIDTH; x++) {

            if (x%2 || y%2)
                continue;

            set_color(pix_buf[x][y][0],
                    pix_buf[x][y][1],
                    pix_buf[x][y][2]); 
            gdk_draw_point (pixmap, gc, x, y);
        }
    }
    //copy pixmap to window
    gtk_widget_draw(drawing_area, NULL);

    return TRUE;
}

void configure_event(GtkWidget *widget, GdkEventConfigure *event, gpointer data){
    if (pixmap)
        gdk_pixmap_unref(pixmap);

    pixmap = gdk_pixmap_new(widget->window,
            widget->allocation.width,
            widget->allocation.height,
            -1);
}

void expose_event(GtkWidget *widget, GdkEventExpose *event, gpointer data){
    gdk_draw_pixmap(widget->window,
            widget->style->fg_gc[GTK_WIDGET_STATE (widget)],
            pixmap,
            event->area.x, event->area.y,
            event->area.x, event->area.y,
            event->area.width, event->area.height);
}

int window_start(int argc, char** argv) 
{
    GtkWidget *window;
    GtkWidget *drawing_area;

    //init thread 
    g_thread_init (NULL);
    gdk_threads_init ();

    //get thread lock
    gdk_threads_enter();

    //init.
    gtk_init(&argc, &argv);

    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_signal_connect (GTK_OBJECT (window), "delete_event",
            GTK_SIGNAL_FUNC (gtk_main_quit), NULL);

    drawing_area = gtk_drawing_area_new();
    gtk_drawing_area_size(GTK_DRAWING_AREA(drawing_area), VGA_WIDTH, VGA_HEIGHT);
    gtk_window_set_position (GTK_WINDOW(window), GTK_WIN_POS_CENTER);
    gtk_container_add (GTK_CONTAINER (window), drawing_area);

    gtk_signal_connect (GTK_OBJECT (drawing_area), "expose_event",
            GTK_SIGNAL_FUNC(expose_event), NULL);
    gtk_signal_connect (GTK_OBJECT (drawing_area), "configure_event",
            GTK_SIGNAL_FUNC(configure_event), NULL);

    gtk_widget_show_all(window);

    gc = gdk_gc_new(window->window);

    gtk_timeout_add(1, repaint, (gpointer) drawing_area);

    gtk_main();
    gdk_threads_leave();

    return 0;
}
#endif

int window_init(void) {
    memset(pix_buf, 0, sizeof(pix_buf));
    return TRUE;
}

