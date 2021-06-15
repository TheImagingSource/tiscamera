
#include <gst/gst.h>

#include <stdio.h> /* printf and putchar */

#include <unistd.h> /* usleep */

static gboolean g_print_device_caps = FALSE;

static gboolean
print_caps_field (GQuark field, const GValue * value, gpointer unused )
{
    gchar *str = gst_value_serialize (value);

    g_print (", %s=%s", g_quark_to_string (field), str);
    g_free (str);
    return TRUE;
}

static void
print_caps( GstCaps * caps )
{
    if( caps == NULL )
        return;
    
    int size = gst_caps_get_size (caps);

    g_print( "\t caps: %d\n", size);
    for (int i = 0; i < size; ++i) {
        GstStructure *s = gst_caps_get_structure (caps, i);

        g_print ("\t       %s", gst_structure_get_name (s));
        gst_structure_foreach (s, print_caps_field, NULL);
        g_print ("\n");
    }
}

static void 
print_device (GstDevice * device, gboolean add_message )
{
    gchar *name = gst_device_get_display_name (device);
    if( add_message )
        g_print("Device added:\n\t name: %s\n", name);
    else
        g_print("Device removed:\n\t name: %s\n", name);
    g_free (name);

    if( !add_message ) {
        return;
    }

    GstStructure* props = gst_device_get_properties( device );

    const char* serial = gst_structure_get_string ( props, "serial" );
    if( serial ) {
        g_print ("\t serial: %s\n", serial );
    }
    const char* model = gst_structure_get_string ( props, "model" );
    if( model ) {
        g_print ("\t model: %s\n", model );
    }

    gst_structure_free( props );

    if( g_print_device_caps ) {
        GstCaps *caps = gst_device_get_caps(device);
        print_caps ( caps );
        gst_caps_unref ( caps );
    }

    g_print ("\n");
}


static gboolean
my_bus_func (GstBus * bus, GstMessage * message, gpointer user_data)
{
    GstDevice *device;

    switch (GST_MESSAGE_TYPE (message))
    {
        case GST_MESSAGE_DEVICE_ADDED:
            gst_message_parse_device_added (message, &device);

            print_device (device, TRUE);

            gst_object_unref (device);
            break;
        case GST_MESSAGE_DEVICE_REMOVED:
            gst_message_parse_device_removed (message, &device);

            print_device (device, FALSE);
            
            gst_object_unref (device);
            break;
        default:
            break;
    }

    return TRUE;
}


static GstDeviceMonitor *
setup_raw_video_source_device_monitor (void)
{
    GstDeviceMonitor *monitor = gst_device_monitor_new ();

    GstBus *bus = gst_device_monitor_get_bus (monitor);
    gst_bus_add_watch (bus, my_bus_func, NULL);
    gst_object_unref (bus);

    gst_device_monitor_add_filter (monitor, "Video/Source/tcam", NULL);

    gst_device_monitor_start (monitor);

    return monitor;
}

static GMainLoop* loop;


int main (int argc, char *argv[])
{
    gst_init(&argc, &argv); // init gstreamer

    loop = g_main_loop_new (NULL, FALSE);

    GstDeviceMonitor* monitor = setup_raw_video_source_device_monitor();

    g_main_loop_run (loop);

    gst_device_monitor_stop (monitor);

    g_main_loop_unref (loop);

    gst_object_unref (monitor);

    return 0;
}
