#include <gst/gst.h>

#include <iostream>
#include <memory>
#include <ctime>
#include <chrono>


#include "unistd.h"

#include "tcamcamera.h"
#include "../../../src/gstreamer-1.0/gstmetatcamstatistics.h"

using namespace gsttcam;
using namespace std::chrono;


int framecount = 0;

GstFlowReturn new_frame_cb(GstAppSink *appsink, gpointer data)
{
    static int64_t last_frame_time_ms = 0;
    static double inter_frame_time_avg = 0;
    static int print_after_x_frames = 30;
    static float max_delay = 0.060;

    GstSample *sample = gst_app_sink_pull_sample(appsink);
    if(sample)
    {
        int64_t cur_time_ms = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
        if (last_frame_time_ms != 0)
        {
            double inter_frame_time = (double)(cur_time_ms - last_frame_time_ms) / 1000;
            inter_frame_time_avg = (inter_frame_time_avg * framecount + inter_frame_time) / (framecount + 1);
            if (framecount % print_after_x_frames == 0) {
                std::cout << 1 / inter_frame_time_avg << std::endl;
            }
            if (inter_frame_time > max_delay) {
                std::cout << "----------------- LATE frame: " << inter_frame_time * 1000 << " ms" << std::endl;
            } else {
                std::cout << "good frame: " << inter_frame_time * 1000 << " ms" << std::endl;
            }
        }
        last_frame_time_ms = cur_time_ms;

        framecount++;
        GstBuffer *buffer = gst_sample_get_buffer(sample);
        GstCaps *caps = gst_sample_get_caps(sample);
        GstClockTime timestamp = GST_BUFFER_PTS(buffer);



        gpointer state = NULL;
        for (GstMeta* m = gst_buffer_iterate_meta(buffer, &state);
             m != NULL;
             m = gst_buffer_iterate_meta(buffer, &state))
        {

            GType tsm = g_type_from_name("TcamStatisticsMeta");

            if (m)
            {
                printf("We have some meta\n");

                if (m->info->type == tsm)
                {
                    TcamStatisticsMeta* mm = (TcamStatisticsMeta*)m;

                    gboolean damaged = FALSE;
                    if (!gst_structure_get_boolean(mm->structure, "is_damaged", &damaged))
                    {
                        printf("Unable to retrieve 'is_damaged'\n");
                    }
                    printf("%s\n", gst_structure_to_string(mm->structure));
                    if (damaged)
                    {
                        printf("Buffer is incomplete\n");
                    }
                    else
                    {
                        printf("Buffer is COMPLETE\n");
                    }

                }

            }
            else
            {
                g_warning("No meta data available ");
            }
        }

//        std::cout << "new frame" << std::endl;
        // g_print("Captured frame %d, Timestamp=%" GST_TIME_FORMAT "            \r",
        //         framecount,
        //         GST_TIME_ARGS(timestamp));
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

    cam.store_device_state("device_state.yaml");

    cam.restore_device_state("device_state.yaml");

    // Get the property with the name "Gain". This only works for cameras
    // which have a Gain property of "double" type. For devices without a
    // "Gain" property, this will throw an exception
    std::shared_ptr<Property> prop = cam.get_property("Gain");
    // Cast to correct type. If the property is of a different type (ie. "integer"), the
    // cast will yield a nullptr
    std::shared_ptr<DoubleProperty> dp = std::dynamic_pointer_cast<DoubleProperty>(prop);
    double gain;
    if (dp)
    {
        std::cout << "Gain Value: " << dp->value << std::endl;
        // Get the current value for gain
        prop->get(cam, gain);
        std::cout << "Gain Value (2): " << gain << std::endl;
        gain /= 2;
        // Set a new gain value
        prop->set(cam, gain);
    }
    else {
        exit(1);
    }

    // Register a callback to be called for each new frame
    cam.set_new_frame_callback(new_frame_cb, NULL);

//    // Create a GStreamer video sink element and attach it to the capture pipeline.
//    // The "ximagesink" Element will open a new window and display the video image
    cam.enable_video_display(gst_element_factory_make("ximagesink", NULL));

    // Start the video capture process
    cam.set_capture_format("GRAY8", {1920,1080}, {30,1});
    cam.start();

    std::time_t start_time = std::time(nullptr);
    const int max_seconds = 60;
    int last_framecount = 0;
    while(std::time(nullptr) < start_time + max_seconds) {
        if (last_framecount < framecount and 0) {
            if (framecount > 90 && framecount <= 120) {
                cam.get_camera_property_list();
                std::cout << "--- read params" << std::endl;
            }

            if (framecount > 180 && framecount <= 210) {
                prop->get(cam, gain);
                std::cout << "--- read gain" << std::endl;
            }

            if (framecount > 270 && framecount <= 300) {
                gain = gain * 0.90;
                prop->set(cam, gain);
                std::cout << "--- set gain: " << gain << std::endl;
            }

            last_framecount = framecount;
        }
    }

//    int a;
//    std::cin >> a;

    // Disable the video display after two seconds. The new frame callback will still be called
    cam.disable_video_display();

    sleep(2);

    // Stop the capture process after two more seconds.
    cam.stop();
    std::cout << std::endl;

    return 0;
}