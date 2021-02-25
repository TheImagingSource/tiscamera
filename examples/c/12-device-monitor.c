
#include <gst/gst.h>

#include <stdio.h> /* printf and putchar */

#include <unistd.h> /* usleep */


static gboolean
my_bus_func (GstBus * bus, GstMessage * message, gpointer user_data)
{
    GstDevice *device;
    gchar *name;

    switch (GST_MESSAGE_TYPE (message))
    {
        case GST_MESSAGE_DEVICE_ADDED:
            gst_message_parse_device_added (message, &device);
            name = gst_device_get_display_name (device);
            g_print("Device added: %s\n", name);
            g_free (name);
            GstCaps* caps = gst_device_get_caps(device);
            g_print("\t caps: %s\n", gst_caps_to_string(caps));
            gst_object_unref (device);
            break;
        case GST_MESSAGE_DEVICE_REMOVED:
            gst_message_parse_device_removed (message, &device);
            name = gst_device_get_display_name (device);
            g_print("Device removed: %s\n", name);
            g_free (name);
            gst_object_unref (device);
            break;
        default:
            break;
    }

    return G_SOURCE_CONTINUE;
}


GstDeviceMonitor *
setup_raw_video_source_device_monitor (void)
{
    GstDeviceMonitor *monitor;
    GstBus *bus;
    GstCaps *caps;

    monitor = gst_device_monitor_new ();

    bus = gst_device_monitor_get_bus (monitor);
    gst_bus_add_watch (bus, my_bus_func, NULL);
    gst_object_unref (bus);

//    caps = gst_caps_new_empty_simple ("ANY");
    gst_device_monitor_add_filter (monitor, "Video/Source/tcam", NULL);
    //gst_caps_unref (caps);

    gst_device_monitor_start (monitor);

    return monitor;
}
static GMainLoop* loop;


int main (int argc, char *argv[])
{
    gst_init(&argc, &argv); // init gstreamer

    const char* serial = NULL; // set this if you do not want the first found device

    GError* err = NULL;

    //GstElement* pipeline = gst_parse_launch("tcambin name=source ! videoconvert ! ximagesink", &err);
    GstDeviceMonitor* monitor = setup_raw_video_source_device_monitor();

    loop = g_main_loop_new (NULL, FALSE);
    g_main_loop_run (loop);

    g_main_loop_unref(loop);

    /* while (1) */
    /* { */
    /*     sleep(1); */
    /* } */

    return 0;
}
