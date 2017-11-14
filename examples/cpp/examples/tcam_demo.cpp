#include <gst/gst.h>

#include <iostream>
#include <memory>

#include "unistd.h"

#include "tcamcamera.h"

using namespace gsttcam;

GstFlowReturn new_frame_cb(GstAppSink *appsink, gpointer data)
{
    static int framecount = 0;
    GstSample *sample = gst_app_sink_pull_sample(appsink);
    if(sample)
    {
        framecount++;
        GstBuffer *buffer = gst_sample_get_buffer(sample);
        GstCaps *caps = gst_sample_get_caps(sample);
        GstClockTime timestamp = GST_BUFFER_PTS(buffer);
        g_print("Captured frame %d, Timestamp=%" GST_TIME_FORMAT "            \r",
                framecount,
                GST_TIME_ARGS(timestamp));
        GstMapInfo info;
        if (gst_buffer_map(buffer, &info, GST_MAP_READ))
        {
            unsigned char* data = info.data;
            // 'data' now contains a pointer to readable image data

            // unmap after use
            gst_buffer_unmap(buffer, &info);
        }
    }

    gst_sample_unref(sample);

    return GST_FLOW_OK;
}

int main(int argc, char **argv)
{
    // Call to gst_init is required before using any gsttcam functions
    gst_init(&argc, &argv);

    // List all connected devices
    auto devices = get_device_list();
    if (devices.empty())
    {
        std::cout << "No cameras found." << std::endl;
        exit(0);
    }
    std::cout << "Connected cameras:" << std::endl;
    for(auto &device : devices)
    {
        std::cout << "'" << device.name << "' " << "Serial No.: " << device.serial << std::endl;
    }

    // If a serial number is given on the command line, use this for the device to work with,
    // otherwise use the first device found
    std::string serial;
    if (argc > 1)
    {
        serial = argv[1];
    }
    else
    {
        serial = devices[0].serial;
    }

    // Create a new camera instance for a given serial number
    TcamCamera cam(serial);

    // Get a list of all suported video formats and print it out
    auto formats = cam.get_format_list();
    std::cout << "Video Formats:" << std::endl;
    for(VideoFormatCaps &fmt : formats)
    {
        std::cout << fmt.to_string() << std::endl;
    }

    // Get a list of all supported properties and print it out
    auto properties = cam.get_camera_property_list();
    std::cout << "Properties:" << std::endl;
    for(auto &prop : properties)
    {
        std::cout << prop->to_string() << std::endl;
    }

    // Get the property with the name "Gain". This only works for cameras
    // which have a Gain property of "double" type. For devices without a
    // "Gain" property, this will throw an exception
    std::shared_ptr<Property> prop = cam.get_property("Gain");
    // Cast to correct type. If the property is of a different type (ie. "integer"), the
    // cast will yield a nullptr
    std::shared_ptr<DoubleProperty> dp = std::dynamic_pointer_cast<DoubleProperty>(prop);
    if (dp)
    {
        std::cout << "Gain Value: " << dp->value << std::endl;
        // Get the current value for gain
        double gain;
        prop->get(cam, gain);
        std::cout << "Gain Value (2): " << gain << std::endl;
        gain /= 2;
        // Set a new gain value
        prop->set(cam, gain);
    }

    // Register a callback to be called for each new frame
    cam.set_new_frame_callback(new_frame_cb, NULL);

    // Create a GStreamer video sink element and attach it to the capture pipeline.
    // The "ximagesink" Element will open a new window and display the video image
    cam.enable_video_display(gst_element_factory_make("ximagesink", NULL));

    // Start the video capture process
    cam.start();

    // Disable the video display after two seconds. The new frame callback will still be called
    sleep(2);
    cam.disable_video_display();

    // Stop the capture process after two more seconds.
    sleep(2);
    cam.stop();
    std::cout << std::endl;

    return 0;
}