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

/* This example will show you how to list information about the available devices */

#include <gst/gst.h>
#include <stdio.h> /* printf and putchar */


void print_device(GstDevice* device)
{

    GstStructure* struc = gst_device_get_properties(device);

    printf("\tmodel:\t%s\tserial:\t%s\ttype:\t%s\n",
           gst_structure_get_string(struc, "model"),
           gst_structure_get_string(struc, "serial"),
           gst_structure_get_string(struc, "type"));

    gst_structure_free(struc);

}


gboolean bus_function(GstBus* bus __attribute__((unused)), GstMessage* message, gpointer user_data __attribute__((unused)))
{
    GstDevice* device;

    switch (GST_MESSAGE_TYPE(message))
    {
        case GST_MESSAGE_DEVICE_ADDED:
        {
            gst_message_parse_device_added(message, &device);

            printf("NEW device\n");
            print_device(device);
            gst_object_unref(device);
            break;
        }
        case GST_MESSAGE_DEVICE_REMOVED:
        {
            // this can also be used as an alternative to device-lost signals
            gst_message_parse_device_removed(message, &device);
            printf("REMOVED Device\n");
            print_device(device);
            gst_object_unref(device);
            break;
        }
        /*
        // not used by tiscamera
        // requires gstreamer 1.16
        case GST_MESSAGE_DEVICE_CHANGED:
        */
        default:
        {
            break;
        }
    }

    // this means we want to continue
    // to listen to device events
    // to stop listening return G_SOURCE_REMOVE;
    return G_SOURCE_CONTINUE;
}


int main(int argc, char* argv[])
{
    /* this line sets the gstreamer default logging level
       it can be removed in normal applications
       gstreamer logging can contain verry useful information
       when debugging your application
       # see https://gstreamer.freedesktop.org/documentation/tutorials/basic/debugging-tools.html
       for further details
    */
    gst_debug_set_default_threshold(GST_LEVEL_WARNING);

    gst_init(&argc, &argv); // init gstreamer

    // The device monitor listens to device activities for us
    GstDeviceMonitor* monitor = gst_device_monitor_new();
    // We are only interested in devices that are in the categories
    // Video and Source && tcam
    gst_device_monitor_add_filter(monitor, "Video/Source/tcam", NULL);

    //
    // static query
    // list all devices that are available right now
    //

    GList* devices = gst_device_monitor_get_devices(monitor);

    for (GList* elem = devices; elem; elem = elem->next)
    {
        GstDevice* device = (GstDevice*) elem->data;

        print_device(device);
    }

    g_list_free_full(devices, gst_object_unref);

    //
    // dynamic listing
    // notify us on all device changes (add/remove/changed)
    // all devices will appear once as ADDED
    //

    GstBus* bus = gst_device_monitor_get_bus(monitor);
    gst_bus_add_watch(bus, bus_function, NULL);
    gst_object_unref(bus);

    // actually start the dynamic monitoring
    gst_device_monitor_start(monitor);

    printf("Now listening to device changes. Disconnect your camera to see a remove event. Connect it to see a connect event. Press Ctrl-C to end.\n");

    // This is simply used to wait for events or the user to end this script
    GMainLoop* loop = g_main_loop_new(NULL, FALSE);
    g_main_loop_run(loop);
    g_main_loop_unref(loop);

    // has to be called when gst_device_monitor_start has been called
    gst_device_monitor_stop(monitor);

    // cleanup
    gst_object_unref(monitor);

    return 0;
}
