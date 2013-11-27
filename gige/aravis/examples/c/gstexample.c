/*
 * Copyright 2013 The Imaging Source Europe GmbH
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 *
 *
 * This is an example on how to use the aravis gstreamer plugin.
 * This program shows how to:
 *     - show a simple color video stream
 *     - change basic settings during the stream
 *
 * This example assumes you use a color camera.
 * If you use a mono camera, remove the bayer gstreamer element and
 * set the wished format for your camera appropriately.
 * How a format string can look like is described below. 
 *
 */

#include <string.h>
#include <stdlib.h>
#include <glib.h>
#include <gtk/gtk.h>
#include <gst/gst.h>
#include <gst/interfaces/xoverlay.h> 
#include <gdk/gdkx.h>
#include <math.h>

#include <arv.h>

/* our camera variables */
#define CAMERA "The Imaging Source Europe GmbH-46210199"
#define WIDTH  2592
#define HEIGHT 1944
#define PATTERN "rggb"
#define INIT_FPS 15

/* the format description gbrg may vary between camera types.
   Other are rggb, gbrg and bggr */
#define FORMAT_COLOR "video/x-raw-bayer"

/* does not need format description */
#define FORMAT_MONO "video/x-raw-gray"


const int ABSVAL_SLIDER_TICKS = 100;

/* container holding all widgets needed for gtk window */
typedef struct
{
    GtkWidget* main_window;           /* The uppermost window, containing all other windows */
    GtkWidget* video_window;          /* The drawing area where the video will be shown */
    GtkWidget* main_box;              /* VBox to hold main_hbox and the controls */
    GtkWidget* main_hbox;             /* HBox to hold the video_window and the stream info text widget */
    GtkWidget* controls;              /* HBox to hold the buttons and sliders */
 
    GtkWidget* framerate_field;       /* textbox for framerate manipulation */
    GtkWidget* framerate_send_button; /* button to send new framerate */
 
    GtkWidget* exposure_label;        /* description for exposure scale */
    GtkWidget* exposure_value_label;  /* value for exposure scale */
    GtkWidget* exposure_hscale;       /* horizontal scale for exposure manipulation */
    GtkWidget* gain_label;            /* description for gain scale */
    GtkWidget* gain_value_label;      /* value for gain scale */
    GtkWidget* gain_hscale;           /* horizontal scale for gain manipulation */

} Win;

static Win* w;

/* container holding all elements needed for gstreamer pipeline */
typedef struct
{
    GMainLoop* loop;
    GstBus* bus;
    guint bus_watch_id;
    GstElement* pipeline;       /* the actual pipeline */
    GstElement* source;         /* aravissrc - retrieves data from camera and sends it to pipeline */
    GstElement* capsfilter;     /* capability filter used for easier caps definition */
    GstElement* bayer;          /* bayer2rgb - responsible for coloring our image */
    GstElement* queue;          /* buffering within our pipeline */
    GstElement* colorspace;     /* colorspace to assure everything we receive is interpretable */
    GstElement* sink;           /* output element - used for displaying the stream */
    ArvCamera* camera;          /* our camera object */

} Gstreamer_Pipeline;

static Gstreamer_Pipeline p;


/*
  callback to adjust the framerate
 */
void framerate_value_changed (Win* we)
{
    GstPad *pad = NULL;
    GstCaps *caps = NULL;

    if (w == NULL)
    {
        return;
    }

    /* retrieve user input */
    const char* s = gtk_entry_get_text(GTK_ENTRY(w->framerate_field));
    gdouble frame_rate = strtod(s, NULL);

    int num;
    int denom;

    /* gst caps want framerates as fraction, so we convert it */
    gst_util_double_to_fraction (frame_rate, &num, &denom);
    
    /*
      When setting caps, width and height currently always have to be defined or
      there will be an undefined behaviour that can crash your application
      Currently width and height have to be mentioned, else undefined behaviour will be experienced.
    */
    caps = gst_caps_new_simple (FORMAT_COLOR,
                                "format", G_TYPE_STRING, PATTERN,
                                "framerate", GST_TYPE_FRACTION, num, denom,
                                "width", G_TYPE_INT, WIDTH,
                                "height",G_TYPE_INT, HEIGHT,
                                NULL);

    /* pause pipeline to allow re-negotiation */
    gst_element_set_state (p.pipeline, GST_STATE_READY);
   
    /* set changed values */
    g_object_set (p.capsfilter, "caps", caps, NULL);

    /* resume playing */
    gst_element_set_state (p.pipeline, GST_STATE_PLAYING);
}


static gboolean bus_call (GstBus* bus, GstMessage* msg, gpointer data)
{
    GMainLoop* loop = (GMainLoop*) data;

    switch (GST_MESSAGE_TYPE (msg))
    {
        case GST_MESSAGE_EOS:
        {
            g_print ("End of stream\n");
            g_main_loop_quit (loop);
            break;
        }
        case GST_MESSAGE_ERROR:
        {
            gchar*  debug;
            GError* error;

            gst_message_parse_error (msg, &error, &debug);
            g_free (debug);

            g_printerr ("Error: %s\n", error->message);
            g_error_free (error);

            g_main_loop_quit (loop);
            break;
        }
        default:
            break;
    }

    return TRUE;
}


/**
 * updates given label with given double value to display
 */
void set_slider_label (GtkLabel* label, double pos)
{
    char output[20];

    sprintf(output, "%7.2f", pos);

    gtk_label_set_text (GTK_LABEL(label),output);
}


/**
 * Sends update to source for camera adjustment
 */
void exposure_value_changed (GtkAdjustment *adj, Win* w)
{
    /* exposure should be handled logarithmically */

    double rmin;
    double rmax;

    /* retrieve exposure boundaries */
    arv_camera_get_exposure_time_bounds (p.camera, &rmin, &rmax);

    /* get new value */
    double pos = adj->value;

    if (isnan(pos))
    {
        pos = 1.0;
    }

    /* since exposure is a logarithmic scale we do the math */
    
    double min = log(rmin);
    double rangelen = log(rmax) - min;
    double val = exp( min + rangelen / ABSVAL_SLIDER_TICKS * pos );

    if( val > rmax ) val = rmax;
    if( val < rmin ) val = rmin;

    /* always set exposure and gain via aravisrc to assure everybody gets the new values */
    g_object_set (G_OBJECT (p.source), "exposure", val, NULL);

    /* update our slides label */
    set_slider_label (GTK_LABEL(w->exposure_value_label), val);
}


/**
 * Sends update to source for camera adjustment
 */
void gain_value_changed (GtkAdjustment *adj, Win* w)
{
    g_object_set (G_OBJECT (p.source), "gain", adj->value, NULL);

    set_slider_label (GTK_LABEL(w->gain_value_label), adj->value);
}


/**
 * Set up the gstreamer pipeline
 */
void create_pipeline()
{
    GstCaps* caps;
    GstElement* filter;

    p.loop = g_main_loop_new (NULL, FALSE);

    /* Create gstreamer elements */
    p.pipeline   = gst_pipeline_new ("player");
    p.source     = gst_element_factory_make ("aravissrc",        "source");
    p.capsfilter = gst_element_factory_make ("capsfilter",       "filter");
    p.bayer      = gst_element_factory_make ("bayer2rgb",        "bayer");
    p.queue      = gst_element_factory_make ("queue",            "queue");
    p.colorspace = gst_element_factory_make ("ffmpegcolorspace", "colorspace");
    p.sink       = gst_element_factory_make ("ximagesink",       "output");

    /* define our conditions and set them for the capsfilter */
    caps = gst_caps_new_simple (FORMAT_COLOR,
                                "format",    G_TYPE_STRING, PATTERN,
                                "bpp",       G_TYPE_INT,    8,
                                "depth",     G_TYPE_INT,    8,
                                "width",     G_TYPE_INT,    WIDTH,
                                "height",    G_TYPE_INT,    HEIGHT,
                                "framerate", GST_TYPE_FRACTION, (guint) INIT_FPS, 1,
                                NULL);

    g_object_set(p.capsfilter, "caps", caps, NULL);

    /* check if factories messed up and could not create some elements */
    if (!p.pipeline)
    {
        g_printerr ("The pipeline element could not be created.\n");
        exit (-1);
    }

    if (!p.source)
    {
        g_printerr ("gstaravis could not be created.\n");
        exit (-1);
    }

    if (!p.bayer)
    {
        g_printerr ("bayer element could not be created.\n");
        exit (-1);
    }

    if (!p.queue)
    {
        g_printerr ("queue element could not be created.\n");
        exit (-1);
    }

    if (!p.colorspace)
    {
        g_printerr ("colorspace element could not be created.\n");
        exit (-1);
    }

    if (!p.sink)
    {
        g_printerr ("sink element could not be created.\n");
        exit (-1);
    }

    /* set the input camera that shall be used by the source element */
    g_object_set (G_OBJECT (p.source), "camera-name", CAMERA, NULL);

    /* retrieve the camera from aravissrc */
    p.camera = malloc(sizeof(ArvCamera));
    g_object_get (G_OBJECT (p.source), "camera", &p.camera, NULL);
    
    if (!ARV_IS_CAMERA(p.camera))
    {
        printf ("Unable to retrieve camera\n");
        exit(1);
    }

    /* add a message handler */
    p.bus = gst_pipeline_get_bus (GST_PIPELINE (p.pipeline));
    p.bus_watch_id = gst_bus_add_watch (p.bus, bus_call, p.loop);
    gst_object_unref (p.bus);

    /* add all elements into the pipeline */
    gst_bin_add_many (GST_BIN (p.pipeline),
                      p.source,
                      p.capsfilter,
                      p.bayer,
                      p.queue,
                      p.colorspace,
                      p.sink,
                      NULL);

    /* link the elements together */
    gst_element_link_many (p.source,
                           p.capsfilter,
                           p.bayer,
                           p.queue,
                           p.colorspace,
                           p.sink,
                           NULL);
    
    /* ready to play */
    gst_element_set_state (p.pipeline, GST_STATE_READY);
}


void togglePipelineState ()
{
    GstState* stat;

    if (GST_STATE(p.pipeline) == GST_STATE_PLAYING)
        gst_element_set_state (p.pipeline, GST_STATE_READY);
    else
        gst_element_set_state (p.pipeline, GST_STATE_PLAYING);
}


/**
 * Initialize components for gtk window
 */
static Win* create_ui () 
{
    Win* w = malloc(sizeof(Win));

    /* create a new window */
    w->main_window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
   
    /* area that will display our stream */
    w->video_window = gtk_drawing_area_new ();

    /* ===== framerate settings ===== */
    w->framerate_field = gtk_entry_new();

    /* connect button event to callback function */
    w->framerate_send_button = gtk_button_new_with_label("Set FrameRate");
    g_signal_connect (G_OBJECT (w->framerate_send_button), "clicked", G_CALLBACK (framerate_value_changed), w);

    /* ===== exposure scale ===== */
    w->exposure_label = gtk_label_new("Exposure:");

    gdouble value;
    g_object_get (G_OBJECT (p.source), "exposure", &value, NULL);

    w->exposure_value_label = gtk_label_new("0.0");
    /* the label should give enough space to not rezise */
    gtk_label_set_width_chars (GTK_LABEL(w->exposure_value_label), 10);
 
    /* the scale will have a range from 0 to 100 to use the percentage to implement a log scale */
    double rmin = 0;
    double rmax = 100;

    double rangelen2 = log(rmax) - log(rmin);
    double exposure_value = (ABSVAL_SLIDER_TICKS) / rangelen2 * ( log(value) - log(rmin) );
    
    gdouble exposure_lower          = 0.0;       /* min value */
    gdouble exposure_upper          = 100.0;     /* max value */
    gdouble exposure_step_increment = 1.0;       /* step size */
    gdouble exposure_page_increment = -10.0;     /* negatic value, to make page up increase value */
    gdouble exposure_page_size      = 0.0;

    GtkObject* wat = gtk_adjustment_new ( exposure_value,
                                          exposure_lower,
                                          exposure_upper,
                                          exposure_step_increment,
                                          exposure_page_increment,
                                          exposure_page_size);

    w->exposure_hscale = gtk_hscale_new (GTK_ADJUSTMENT(wat));
    gtk_scale_set_draw_value(GTK_SCALE (w->exposure_hscale), FALSE);
    gtk_widget_set_usize (GTK_WIDGET (w->exposure_hscale), 400, 30);
    g_signal_connect (G_OBJECT(wat), "value-changed", G_CALLBACK (exposure_value_changed), w);
    set_slider_label (GTK_LABEL(w->exposure_value_label), value);


    /* ===== gain part ===== */

    w->gain_label = gtk_label_new("Gain:");

    double gain_value;
    gdouble gain_lower; /* min value */
    gdouble gain_upper; /* max value */    

    w->gain_value_label = gtk_label_new("0.0");

    g_object_get (G_OBJECT (p.source), "gain", &gain_value, NULL);
    arv_camera_get_gain_bounds (p.camera, &gain_lower, &gain_upper);

    set_slider_label (GTK_LABEL(w->gain_value_label), gain_value);

    gdouble gain_step_increment = 0.1;       /* step size */
    gdouble gain_page_increment = -1.0;
    gdouble gain_page_size      = 0.0;

    GtkObject* range_gain = gtk_adjustment_new (gain_value,
                                                gain_lower,
                                                gain_upper,
                                                gain_step_increment,
                                                gain_page_increment,
                                                gain_page_size);

    w->gain_hscale = gtk_hscale_new (GTK_ADJUSTMENT(range_gain));
    gtk_scale_set_draw_value(GTK_SCALE (w->gain_hscale), FALSE);
    gtk_widget_set_usize (GTK_WIDGET (w->gain_hscale), 400, 50);
    g_signal_connect (G_OBJECT(range_gain), "value-changed", G_CALLBACK (gain_value_changed), w);

    
    GtkWidget* toggle;
    toggle = gtk_button_new_with_label("Start/Stop");
    g_signal_connect (G_OBJECT (toggle), "clicked", G_CALLBACK (togglePipelineState), NULL);


    /* ===== organize everything and place in window ===== */

    w->controls = gtk_hbox_new (FALSE, 0);

    gtk_box_pack_start (GTK_BOX (w->controls), toggle, FALSE, FALSE, 2);
    gtk_box_pack_start (GTK_BOX (w->controls), w->framerate_field, FALSE, FALSE, 2);
    gtk_box_pack_start (GTK_BOX (w->controls), w->framerate_send_button, FALSE, FALSE, 2);
    gtk_box_pack_start (GTK_BOX (w->controls), w->exposure_label, FALSE, FALSE, 2);
    gtk_box_pack_start (GTK_BOX (w->controls), w->exposure_value_label, FALSE, FALSE, 2);
    gtk_box_pack_start (GTK_BOX (w->controls), w->exposure_hscale, FALSE, FALSE, 2);
    gtk_box_pack_start (GTK_BOX (w->controls), w->gain_label, FALSE, FALSE, 2);
    gtk_box_pack_start (GTK_BOX (w->controls), w->gain_value_label, FALSE, FALSE, 2);
    gtk_box_pack_start (GTK_BOX (w->controls), w->gain_hscale, FALSE, FALSE, 2);
   
    w->main_hbox = gtk_hbox_new (FALSE, 0);
    gtk_box_pack_start (GTK_BOX (w->main_hbox), w->video_window, TRUE, TRUE, 0);
   
    w->main_box = gtk_vbox_new (FALSE, 0);
    gtk_box_pack_start (GTK_BOX (w->main_box), w->main_hbox, TRUE, TRUE, 0);
    gtk_box_pack_start (GTK_BOX (w->main_box), w->controls, FALSE, FALSE, 0);
    gtk_container_add (GTK_CONTAINER (w->main_window), w->main_box);

    gtk_window_set_default_size (GTK_WINDOW (w->main_window), HEIGHT, WIDTH);
   
    /* show all widgets we just added to out main window */
    gtk_widget_show_all (w->main_window);

    return w;
}


int main (int argc, char *argv[])
{
    /* Initialisation */
    gst_init (&argc, &argv);

    /* Initialize GTK */
    gtk_init (&argc, &argv);

    /* initialize out gstreamer pipeline */
    create_pipeline();

    /* create our window that shall display everything */
    w = create_ui();
    
    /* do not display the video in own frame but integrate it into our window */
    gst_x_overlay_set_window_handle(GST_X_OVERLAY(p.sink), GDK_WINDOW_XID(w->video_window->window));

    /* Set the pipeline to "playing" state */
    g_print ("Now playing stream from: %s\n", CAMERA);

    /*
       IMPORTANT after state changed to playing the capabilities are fixed.
     */
    gst_element_set_state (p.pipeline, GST_STATE_PLAYING);

    g_print ("Running...");

    /* finished construction and now run the main loop while waiting for user interaction */
    g_main_loop_run (p.loop);

    
    /* ===== clean up ===== */

    /* Out of the main loop, clean up nicely */
    g_print ("Returned, stopping playback\n");
    gst_element_set_state (p.pipeline, GST_STATE_PAUSED);
    gst_element_set_state (p.pipeline, GST_STATE_NULL);

    g_print("Deleting pipeline\n");
    gst_object_unref (GST_OBJECT (p.pipeline));
    g_source_remove (p.bus_watch_id);
    g_main_loop_unref (p.loop);

    free(w);
    w = NULL;

    return 0;
}
