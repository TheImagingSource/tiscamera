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

#include <tcamprop.h>

static GstElement* pipeline;

static gboolean stream_active = FALSE;

GtkWidget* output_area;
GtkWidget* listbox;


static GstElement* get_element (const char* name)
{
    if (pipeline == NULL)
    {
        printf("VVVVVVVVVV\n");
    }
    return gst_bin_get_by_name(GST_BIN(pipeline), name);
}

static void close_valve (gboolean is_closed)
{
    GstElement* valve = get_element("valve");

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
    close_valve(FALSE);
}


static gboolean bus_call (GstBus* bus, GstMessage* msg, gpointer data)
{
    GMainLoop* loop = (GMainLoop*) data;

    switch (GST_MESSAGE_TYPE (msg))
    {
        case GST_MESSAGE_EOS:
        {
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

            break;
        }
		case GST_MESSAGE_ELEMENT:
		{
			const GstStructure *s = gst_message_get_structure (msg);
			const gchar *name = gst_structure_get_name (s);

			if (strcmp(name, "GstMultiFileSink") == 0)
			{
                close_valve(TRUE);
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
    pipeline = gst_parse_launch("tcambin name=src ! tee name=t t. ! queue ! valve name=valve ! videoconvert ! jpegenc ! multifilesink name=filesink post-messages=true location=/tmp/image-%06d.jpg t. ! queue ! videoconvert ! xvimagesink ", &err);

    if (pipeline == NULL)
    {
        printf("Unable to create pipeline.\n");
        return;
    }


    GstBus* bus = gst_pipeline_get_bus(GST_PIPELINE(pipeline));
    gst_bus_add_watch(bus, bus_call, NULL);
    g_object_unref(bus);
}


void button_callback (GtkWidget* button, gpointer user_data)
{
    GstElement* src = get_element("src");

    if (src == NULL)
    {
        printf("Could not find source");
        return;
    }

    GValue val = {};

    g_value_init(&val, G_TYPE_BOOLEAN);
    g_value_set_boolean(&val, TRUE);

    printf("Activating '%s'\n", (char*)user_data);

    if (!tcam_prop_set_tcam_property(TCAM_PROP(src), (gchar*)user_data, &val))
    {
        printf("Could not set tcam property\n");
    }
}


static void checkbox_callback (GtkWidget* widget, gpointer data)
{

    GstElement* src = get_element("src");
    if (src == NULL)
    {
        printf("Could not find source");
        return;
    }

    GValue val = {};

    g_value_init(&val, G_TYPE_BOOLEAN);

    if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget)))
    {
        g_value_set_boolean(&val, TRUE);
    }
    else
    {
        g_value_set_boolean(&val, FALSE);
    }


    if (!tcam_prop_set_tcam_property(TCAM_PROP(src), (gchar*)data, &val))
    {
        printf("Could not set tcam property\n");
    }
}


void combobox_callback (GtkWidget* combo_box, gpointer user_data)
{

    TcamProp* cam = TCAM_PROP(get_element("src"));

    gchar* str = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(combo_box));

    if (str == NULL)
    {
        fprintf(stderr, "Unable to retrieve active entry\n");
        return;
    }

    GValue val = {};

    g_value_init(&val, G_TYPE_STRING);
    g_value_set_string(&val, str);

    if (!tcam_prop_set_tcam_property(cam, (gchar*)user_data, &val))
    {
        printf("Could not set %s\n", user_data);
    }
}


static void slider_callback_int (GtkRange* scale, gpointer user_data)
{
    /* int value = gtk_scale_button_get_value(GTK_SCALE_BUTTON(scale)); */

    int value = gtk_range_get_value(scale);

    printf("Activating '%s' %d\n", (char*)user_data, value);

    GValue val = {};

    g_value_init(&val, G_TYPE_INT);
    g_value_set_int(&val, value);

    tcam_prop_set_tcam_property(TCAM_PROP(get_element("src")), (char*)user_data, &val);
}


static void slider_callback_double (GtkRange* scale, gpointer user_data)
{
    double value = gtk_range_get_value(scale);

    printf("Setting '%s' to %f\n", (char*)user_data, value);

    GValue val = {};

    g_value_init(&val, G_TYPE_DOUBLE);
    g_value_set_double(&val, value);

    tcam_prop_set_tcam_property(TCAM_PROP(get_element("src")), (char*)user_data, &val);
}


static void add_properties ()
{
    TcamProp* prop = TCAM_PROP(get_element("src"));

    if (prop == NULL)
    {
        printf("Could not find src.\n");
    }

    GSList* prop_names = tcam_prop_get_tcam_property_names(prop);

    if (prop_names == NULL)
    {
        printf ("No properties found.\n");
        return;
    }

    GSList* p = prop_names;
    do
    {
        GValue value = {};
        GValue min = {};
        GValue max = {};
        GValue default_value = {};
        GValue step = {};
        GValue type = {};
        GValue flags = {};
        GValue category = {};
        GValue group = {};

        if (tcam_prop_get_tcam_property(TCAM_PROP(prop), p->data,
                                        &value,
                                        &min, &max,
                                        &default_value, &step, &type, &flags,
                                        &category, &group) == FALSE)
        {
            printf ("Unable to retrieve tcam property '%s'\n", p->data);
            goto end;
        }

        GtkWidget* row = gtk_list_box_row_new();

        if (g_strcmp0(g_value_get_string(&type), "button") == 0)
        {
            GtkWidget* button = gtk_button_new();
            gtk_button_set_label(GTK_BUTTON(button), p->data);

            g_signal_connect(G_OBJECT(button), "clicked", G_CALLBACK(button_callback), p->data);

            gtk_container_add(GTK_CONTAINER(row), button);
        }
        else if (g_strcmp0(g_value_get_string(&type), "boolean") == 0)
        {
            GtkWidget* check = gtk_check_button_new();

            gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check), g_value_get_boolean(&value));

            GtkWidget* label = gtk_label_new(p->data);

            GtkWidget* hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
            g_signal_connect(G_OBJECT(check), "toggled", G_CALLBACK(checkbox_callback), p->data);
            /* gtk_container_add(GTK_CONTAINER(hbox), label); */
            /* gtk_container_add(GTK_CONTAINER(hbox), check); */

            gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, TRUE, 0);
            gtk_box_pack_start(GTK_BOX(hbox), check, TRUE, TRUE, 0);

            gtk_container_add(GTK_CONTAINER(row), hbox);
        }
        else if (g_strcmp0(g_value_get_string(&type), "integer") == 0)
        {
            if (g_value_get_int(&min) >= g_value_get_int(&max))
            {
                fprintf(stderr, "%s has weird range. ignoring. min: %lld max: %lld\n",
                       p->data,
                       g_value_get_int(&min), g_value_get_int(&max));
                goto end;
            }

            GtkWidget* slider = gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL,
                                                         g_value_get_int(&min),
                                                         g_value_get_int(&max),
                                                         g_value_get_int(&step));

            gtk_scale_set_draw_value(GTK_SCALE(slider), TRUE);
            gtk_range_set_value(GTK_RANGE(slider), g_value_get_int(&value));

            g_signal_connect(G_OBJECT(GTK_RANGE(slider)), "value-changed", G_CALLBACK(slider_callback_int), p->data);

            GtkWidget* label = gtk_label_new(p->data);

            GtkWidget* hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);

            gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, TRUE, 0);
            gtk_box_pack_start(GTK_BOX(hbox), slider, TRUE, TRUE, 0);

            gtk_container_add(GTK_CONTAINER(row), hbox);
        }
        else if (g_strcmp0(g_value_get_string(&type), "double") == 0)
        {
            GtkWidget* slider = gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL,
                                                         g_value_get_double(&min),
                                                         g_value_get_double(&max),
                                                         g_value_get_double(&step));

            gtk_scale_set_draw_value(GTK_SCALE(slider), TRUE);
            gtk_range_set_value(GTK_RANGE(slider), g_value_get_double(&value));

            g_signal_connect(G_OBJECT(GTK_RANGE(slider)),
                             "value-changed", G_CALLBACK(slider_callback_double),
                             p->data);

            GtkWidget* label = gtk_label_new(p->data);

            GtkWidget* hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);

            gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, TRUE, 0);
            gtk_box_pack_start(GTK_BOX(hbox), slider, TRUE, TRUE, 0);

            gtk_container_add(GTK_CONTAINER(row), hbox);
        }
        else if (g_strcmp0(g_value_get_string(&type), "string") == 0)
        {
            // currently not used
        }
        else if (g_strcmp0(g_value_get_string(&type), "enum") == 0)
        {
            GSList* entry_list = tcam_prop_get_tcam_menu_entries(prop, (const char*)p->data);

            if (entry_list == NULL)
            {
                printf ("enum '%s' has no entries.\n", p->data);
                goto end;
            }

            GtkWidget* combo_box = gtk_combo_box_text_new();
            GSList* e = entry_list;
            gint index = 0;
            do
            {
                gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(combo_box), e->data);

                if (g_strcmp0(e->data, g_value_get_string(&value)) == 0)
                {
                    gtk_combo_box_set_active(GTK_COMBO_BOX(combo_box), index);
                }

                e = g_slist_next(e);
                index++;
            }
            while (e != NULL);

            g_signal_connect(G_OBJECT(combo_box), "changed", G_CALLBACK(combobox_callback), p->data);

            GtkWidget* label = gtk_label_new(p->data);

            GtkWidget* hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);

            gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, TRUE, 0);
            gtk_box_pack_start(GTK_BOX(hbox), combo_box, TRUE, TRUE, 0);

            gtk_container_add(GTK_CONTAINER(row), hbox);

        }
        else
        {
            printf ("Unkown property type '%s' for '%s'\n", g_value_get_string(&type), p->data);
        }

        gtk_container_add(GTK_CONTAINER(listbox), row);

    end:
        p = g_slist_next(p);
    }
    while (p != NULL);

    gtk_widget_show_all(listbox);

}


void toggle_play_stop (GtkWidget* button,
                       gpointer data)
{
    if (stream_active == TRUE)
    {
        close_valve(FALSE);
        gst_element_set_state(pipeline, GST_STATE_READY);
        printf("Stopping stream...\n");
        gtk_button_set_label(GTK_BUTTON(button), "Play");
        stream_active = FALSE;
    }
    else
    {
        gboolean create_properties = FALSE;
        if (pipeline == NULL)
        {
            create_pipeline();
            create_properties = TRUE;
        }
        close_valve(FALSE);
        gst_element_set_state(pipeline, GST_STATE_PLAYING);

        printf("Starting stream...\n");
        gtk_button_set_label(GTK_BUTTON(button), "Stop");
        stream_active = TRUE;
        if (create_properties == TRUE)
        {
            sleep(1);
            add_properties();
        }
    }
}


static void activate (GtkApplication* app,
                      gpointer user_data)
{
    GtkWidget* window;

    window = gtk_application_window_new (app);
    gtk_window_set_title (GTK_WINDOW (window), "TCam Properties Example");
    gtk_window_set_default_size (GTK_WINDOW (window), 800, 600);

    GtkWidget* hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    GtkWidget* vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);

    gtk_widget_set_size_request(vbox, 300, 600);
    gtk_container_add(GTK_CONTAINER(hbox), vbox);

    gtk_container_add(GTK_CONTAINER(window), hbox);

    GtkWidget* draw_area = gtk_drawing_area_new();
    gtk_widget_set_size_request(draw_area, 640, 480);

    output_area = draw_area;

    gtk_container_add(GTK_CONTAINER(hbox), draw_area);

    GtkWidget* start_button = gtk_button_new_with_label("Start");
    g_signal_connect (start_button, "clicked", G_CALLBACK (toggle_play_stop), NULL);

    GtkWidget* save_button = gtk_button_new_with_label("Save");
    g_signal_connect (save_button, "clicked", G_CALLBACK (save_image), NULL);

    gtk_container_add(GTK_CONTAINER(vbox), start_button);
    gtk_container_add(GTK_CONTAINER(vbox), save_button);

    listbox = gtk_list_box_new();
    gtk_list_box_set_selection_mode(GTK_LIST_BOX(listbox), GTK_SELECTION_NONE);

    GtkWidget* scroll_window = gtk_scrolled_window_new(NULL, NULL);

    gtk_container_add(GTK_CONTAINER(scroll_window), listbox);
    gtk_scrolled_window_set_overlay_scrolling(GTK_SCROLLED_WINDOW(scroll_window), FALSE);

    gtk_widget_set_size_request(scroll_window, 300, 600);

    gtk_box_pack_start(GTK_BOX(vbox), scroll_window, TRUE, TRUE, 0);

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
    /* g_object_unref (app); */

    return status;
}
