/*
 * Copyright 2016 The Imaging Source Europe GmbH
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
 */

#include <gst/gst.h>
#include <gst/video/video.h>

#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <gdk/gdkx.h>

#include <string.h>

GstElement* pipeline;

gboolean stream_active = FALSE;

GtkWidget* output_area;
GtkWidget* start_button;


static void close_valve (gboolean is_closed)
{
    GstElement* valve = gst_bin_get_by_name(GST_BIN(pipeline), "valve");

    if (valve == NULL)
    {
        printf("No valve found. Aborting\n");
        return;
    }

    GValue val = G_VALUE_INIT;
    g_value_init(&val, G_TYPE_BOOLEAN);
    g_value_set_boolean(&val, is_closed);

    g_object_set_property(G_OBJECT(valve), "drop", &val);
}


void save_image (GtkWidget* button,
                 gpointer data)
{
    printf("saving image\n");
    close_valve(FALSE);
}


static gboolean bus_call (GstBus* bus, GstMessage* msg, gpointer data)
{
    GMainLoop* loop = (GMainLoop*) data;

    switch (GST_MESSAGE_TYPE (msg))
    {
        case GST_MESSAGE_EOS:
        {
            /* g_main_loop_quit (Camera.loop); */
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

            /* g_main_loop_quit (Camera.loop); */
            break;
        }
		case GST_MESSAGE_ELEMENT:
		{
			const GstStructure *s = gst_message_get_structure (msg);
			const gchar *name = gst_structure_get_name (s);

			if (strcmp(name, "GstMultiFileSink") == 0)
			{
                close_valve(TRUE);
				/* g_object_set(Camera.Valve,"drop",1,NULL); */
				/* Camera.SaveanImage = 0; */
			}
		}
        default:
            break;
    }

    if (gst_is_video_overlay_prepare_window_handle_message(msg))
    {
        gst_video_overlay_set_window_handle(GST_VIDEO_OVERLAY(GST_MESSAGE_SRC(msg)),
                                            GDK_WINDOW_XID(gtk_widget_get_window(output_area)));
    }

    return TRUE;
}


static void create_pipeline ()
{
    GError* err = NULL;
    pipeline = gst_parse_launch("tcambin name=src \
                                ! tee name=t t. \
                                ! queue \
                                ! valve name=valve \
                                ! videoconvert \
                                ! jpegenc \
                                ! multifilesink \
                                 name=filesink post-messages=true \
                                 location=/tmp/image-%06d.jpg t. \
                                ! queue \
                                ! videoconvert \
                                ! xvimagesink ", &err);

    if (pipeline == NULL)
    {
        printf("Unable to create pipeline.\n");
        return;
    }

    GstBus* bus = gst_pipeline_get_bus(GST_PIPELINE(pipeline));
    gst_bus_add_watch(bus, bus_call, NULL);
    g_object_unref(bus);
}


void toggle_play_stop (GtkWidget* button,
                       gpointer data)
{
    if (stream_active == TRUE)
    {
        close_valve(FALSE);
        gst_element_set_state(pipeline, GST_STATE_READY);
        printf("Stopping stream...\n");
        stream_active = FALSE;
        gtk_button_set_label(GTK_BUTTON(start_button), "Start");
    }
    else
    {
        if (pipeline == NULL)
        {
            create_pipeline();
        }

        gst_element_set_state(pipeline, GST_STATE_READY);

        close_valve(FALSE);

        gst_element_set_state(pipeline, GST_STATE_PLAYING);

        printf("Starting stream...\n");
        stream_active = TRUE;
        gtk_button_set_label(GTK_BUTTON(start_button), "Stop");
    }
}


static void activate (GtkApplication* app,
                      gpointer        user_data)
{
    GtkWidget* window;
    window = gtk_application_window_new (app);
    gtk_window_set_title (GTK_WINDOW (window), "TCam Pipeline Example");
    gtk_window_set_default_size (GTK_WINDOW (window), 800, 600);

    GtkWidget* vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 50);
    gtk_container_add(GTK_CONTAINER(window), vbox);

    GtkWidget* draw_area = gtk_drawing_area_new();
    gtk_widget_set_size_request(draw_area, 640, 480);

    output_area = draw_area;

    gtk_container_add(GTK_CONTAINER(vbox), draw_area);

    GtkWidget* hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 50);
    gtk_container_add(GTK_CONTAINER(vbox), hbox);

    start_button = gtk_button_new_with_label("Start");
    g_signal_connect (start_button, "clicked", G_CALLBACK (toggle_play_stop), NULL);

    GtkWidget* save_button = gtk_button_new_with_label("Save");
    g_signal_connect (save_button, "clicked", G_CALLBACK (save_image), NULL);

    gtk_container_add(GTK_CONTAINER(hbox), start_button);
    gtk_container_add(GTK_CONTAINER(hbox), save_button);


    gtk_widget_show_all (window);
}


int main (int argc, char *argv[])
{
    /* Initialisation */
    gst_init(&argc, &argv);

    GtkApplication* app;
    int status;

    app = gtk_application_new ("org.gtk.example", G_APPLICATION_FLAGS_NONE);
    g_signal_connect (app, "activate", G_CALLBACK (activate), NULL);


    status = g_application_run (G_APPLICATION (app), argc, argv);
    g_object_unref (app);

    return status;
}
