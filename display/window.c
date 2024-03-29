#include <gtk/gtk.h>
#include <unistd.h>
#include <string.h>

#include "tools.h"
#include "vga.h"

void *vga_shm_get(void);

static struct rgb15 *disp_data;
static GdkPixmap *pixmap = NULL;
static GdkGC *gc = NULL;
static int first_draw;

static GdkGC *set_color(gushort r, gushort g, gushort b)
{
    GdkColor color;

    color.red = r;
    color.green = g;
    color.blue = b;
    gdk_color_alloc(gdk_colormap_get_system(), &color);
    gdk_gc_set_foreground(gc, &color);
    return gc;
}

static gint repaint(gpointer data){
    GtkWidget *drawing_area = GTK_WIDGET (data);

    int x, y;

    if (first_draw) {
        //on first paint event, fill bgcolor with black.
        first_draw = FALSE;
        set_color(0, 0, 0);
        gdk_draw_rectangle (pixmap, gc, TRUE, 0, 0, VGA_WIDTH, VGA_HEIGHT);
    }

    //g_print ("draw...\n");
    x = y = 0;
    for (y = 0; y < VGA_HEIGHT; y++) {
        for (x = 0; x < VGA_WIDTH; x++) {

            int pos = x + VGA_WIDTH * y;

            if (x%2 || y%2)
                continue;

            set_color( to16bit(disp_data[pos].r), 
                    to16bit(disp_data[pos].g),
                    to16bit(disp_data[pos].b)); 
            gdk_draw_point (pixmap, gc, x, y);
        }
    }
    //copy pixmap to window
    gtk_widget_draw(drawing_area, NULL);

    return TRUE;
}

static void configure_event(GtkWidget *widget, GdkEventConfigure *event, gpointer data){
    if (pixmap)
        gdk_pixmap_unref(pixmap);

    pixmap = gdk_pixmap_new(widget->window,
            widget->allocation.width,
            widget->allocation.height,
            -1);
}

static void expose_event(GtkWidget *widget, GdkEventExpose *event, gpointer data){
    //copy pixmap to the window
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

    gtk_timeout_add(1000 / VGA_REFRESH_RATE, repaint, (gpointer) drawing_area);

    gtk_main();
    gdk_threads_leave();

    return 0;
}

static int shm_init(void) {

    /* get vga shared memory */
    if((disp_data = (struct rgb15 *)vga_shm_get()) == NULL)
    {
        fprintf(stderr, "error attaching shared memory.\n");
        return FALSE;
    }
    return TRUE;
}

int window_init(void) {
    int ret;
    disp_data = NULL;
    first_draw = TRUE;

    ret = shm_init();
    if (!ret)
        return FALSE;
   
    memset(disp_data, 0, sizeof(VGA_SHM_SIZE));

    //init thread 
    g_thread_init (NULL);
    gdk_threads_init ();

    return TRUE;
}

