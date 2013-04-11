#include <gtk/gtk.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <libio.h>
#include <gdk/gdkkeysyms.h>

#include "tools.h"

#define JOYPAD_IMG_PATH "./famicon-controller.jpg"

static int button_up;
static int button_down;
static int button_left;
static int button_right;
static int button_A;
static int button_B;
static int button_select;
static int button_start;

int get_button(unsigned int code) {
    code &= 0x07;
    switch (code) {
        case 0:
            return button_A;
        case 1:
            return button_B;
        case 2:
            return button_select;
        case 3:
            return button_start;
        case 4:
            return button_up;
        case 5:
            return button_down;
        case 6:
            return button_left;
        case 7:
        default:
            return button_right;
    }
    return FALSE;
}

static gboolean expose_event(GtkWidget *widget, GdkEventExpose *event, gpointer data){
    GdkPixbuf *pixbuf = (GdkPixbuf *)data;

    gdk_pixbuf_render_to_drawable (pixbuf, widget->window,
            widget->style->fg_gc[GTK_STATE_NORMAL],
            0, 0, 0, 0,
            gdk_pixbuf_get_width(pixbuf),
            gdk_pixbuf_get_height(pixbuf),
            GDK_RGB_DITHER_NORMAL, 0, 0);
    return TRUE;
}

static void key_press(GtkWidget *widget,
        GdkEventKey *event,
        gpointer data){

    int valid_key = FALSE;
    const char *key;

    switch (event->keyval) {
        case 'q':
        case 'w':
        case 'e':
        case 'r':
            button_up = TRUE;
            valid_key = TRUE;
            key = "up";
            break;
        case 'z':
        case 'x':
        case 'c':
        case 'v':
        case 'b':
            button_down = TRUE;
            valid_key = TRUE;
            key = "down";
            break;
        case 'a':
        case 's':
            button_left = TRUE;
            valid_key = TRUE;
            key = "left";
            break;
        case 'd':
        case 'f':
            button_right = TRUE;
            valid_key = TRUE;
            key = "right";
            break;
        case 'l':
        case ';':
            button_B = TRUE;
            valid_key = TRUE;
            key = "B";
            break;
        case ':':
        case ']':
            button_A = TRUE;
            valid_key = TRUE;
            key = "A";
            break;
        case GDK_Tab: //tab key
            button_select = TRUE;
            valid_key = TRUE;
            key = "select";
            break;
        case GDK_Return: //enter key
            button_start = TRUE;
            valid_key = TRUE;
            key = "start";
            break;
    }
    if (valid_key)
        dprint("%s key pressed\n", key);
    //dprint("press, key code: %d key: %s\n", event->keyval, event->string);
}

static void key_release(GtkWidget *widget,
        GdkEventKey *event,
        gpointer data){

    int valid_key = FALSE;
    const char *key;

    switch (event->keyval) {
        case 'q':
        case 'w':
        case 'e':
        case 'r':
            button_up = FALSE;
            valid_key = TRUE;
            key = "up";
            break;
        case 'z':
        case 'x':
        case 'c':
        case 'v':
        case 'b':
            button_down = FALSE;
            valid_key = TRUE;
            key = "down";
            break;
        case 'a':
        case 's':
            button_left = FALSE;
            valid_key = TRUE;
            key = "left";
            break;
        case 'd':
        case 'f':
            button_right = FALSE;
            valid_key = TRUE;
            key = "right";
            break;
        case 'l':
        case ';':
            button_B = FALSE;
            valid_key = TRUE;
            key = "B";
            break;
        case ':':
        case ']':
            button_A = FALSE;
            valid_key = TRUE;
            key = "A";
            break;
        case 65289: //tab key
            button_select = FALSE;
            valid_key = TRUE;
            key = "select";
            break;
        case 65293: //enter key
            button_start = FALSE;
            valid_key = TRUE;
            key = "start";
            break;
    }
    if (valid_key)
        dprint("%s key released\n", key);
    //dprint("release, key code: %d key: %s\n", event->keyval, event->string);
}


void close_joypad_wnd(void) {
    gtk_main_quit();
}

void* window_start(void* arg)
{
    GtkWidget *window;
    GtkWidget *drawing_area;
    GdkPixbuf *pixbuf;
    GError *err = NULL;
    int width, height;
    int argc;
    char *argv0 = "./motonesemu";
    char *argv[] = {(char*)&argv0, NULL};

    //get thread lock
    gdk_threads_enter();

    //init.
    argc = 1;
    gtk_init(&argc, (char***)&argv);

    pixbuf = gdk_pixbuf_new_from_file(JOYPAD_IMG_PATH, &err);
    if (pixbuf == NULL) {
        fprintf(stderr, "error reading image file[%s].\n", JOYPAD_IMG_PATH);
        g_error_free (err);
        return NULL;
    }
    width = gdk_pixbuf_get_width (pixbuf); 
    height = gdk_pixbuf_get_height (pixbuf);
    //dprint("w:%d, h:%d\n", width, height);
    //

    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_signal_connect (GTK_OBJECT (window), "destroy",
            GTK_SIGNAL_FUNC (gtk_main_quit), NULL);

    drawing_area = gtk_drawing_area_new();
    gtk_drawing_area_size(GTK_DRAWING_AREA(drawing_area), width, height);
    gtk_window_set_position (GTK_WINDOW(window), GTK_WIN_POS_CENTER);
    gtk_container_add (GTK_CONTAINER (window), drawing_area);

    //draw event
    gtk_signal_connect (GTK_OBJECT (drawing_area), "expose_event",
            GTK_SIGNAL_FUNC(expose_event), pixbuf);
    //key board event
    gtk_signal_connect(GTK_OBJECT(window), "key_press_event",
            GTK_SIGNAL_FUNC(key_press), NULL);
    gtk_signal_connect(GTK_OBJECT(window), "key_release_event",
            GTK_SIGNAL_FUNC(key_release), NULL);
/*
    gtk_signal_connect (GTK_OBJECT (drawing_area), "configure_event",
            GTK_SIGNAL_FUNC(configure_event), NULL);
*/

    gtk_widget_show_all(window);
    gtk_main();
    gdk_threads_leave();

    return NULL;
}

int init_joypad_wnd(void) {
    //init thread
    g_thread_init (NULL);
    gdk_threads_init ();

    printf("joypad key entry:\n");
    printf("    select button:  <Tab>\n");
    printf("    start button:   <Enter>\n");
    printf("    up key:         Q, W, E, R\n");
    printf("    down key:       Z, X, C, V, B\n");
    printf("    left key:       A, S\n");
    printf("    right key:      D, F\n");
    printf("    A button:       <:>, <]>\n");
    printf("    B button:       L, <;>\n");

    button_up = FALSE;
    button_down = FALSE;
    button_left = FALSE;
    button_right = FALSE;
    button_A = FALSE;
    button_B = FALSE;
    button_select = FALSE;
    button_start = FALSE;

    return TRUE;
}

