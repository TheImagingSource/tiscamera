/*
 * Copyright 2014 The Imaging Source Europe GmbH
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

#ifndef TCAM_V4L2DEVICE_H
#define TCAM_V4L2DEVICE_H

#include "DeviceInterface.h"
#include "VideoFormat.h"
#include "VideoFormatDescription.h"
#include "FormatHandlerInterface.h"

#include <linux/videodev2.h>
#include <memory>
#include <thread>
#include <mutex>              // std::mutex, std::unique_lock
#include <condition_variable> // std::condition_variable

VISIBILITY_INTERNAL

namespace tcam
{

class V4l2Device : public DeviceInterface
{

    struct property_description
    {
        int id; // v4l2 identification
        double conversion_factor;
        std::shared_ptr<Property> prop;
    };
    static const int EMULATED_PROPERTY = -1;

    class V4L2PropertyHandler : public PropertyImpl
    {
        friend class V4l2Device;

    public:
        V4L2PropertyHandler (V4l2Device*);


        std::vector<std::shared_ptr<Property>> create_property_vector ();

        bool set_property (const Property&);
        bool get_property (Property&);

    protected:

        static const int EMULATED_PROPERTY = -1;

        std::vector<property_description> properties;
        std::vector<property_description> special_properties;

        V4l2Device* device;
    };

    class V4L2FormatHandler : public FormatHandlerInterface
    {
        friend class V4l2Device;

    public:
        V4L2FormatHandler (V4l2Device*);
        std::vector<double> get_framerates (const struct tcam_image_size&, int pixelformat=0);

    protected:
        V4l2Device* device;
    };

public:

    explicit V4l2Device (const DeviceInfo&);

    V4l2Device () = delete;

    ~V4l2Device ();

    DeviceInfo get_device_description () const;

    // TODO: eval if weak_ptr should be distributed
    std::vector<std::shared_ptr<Property>> getProperties ();

    bool set_property (const Property&);

    bool get_property (Property&);

    bool set_video_format (const VideoFormat&);

    bool validate_video_format (const VideoFormat&) const;

    VideoFormat get_active_video_format () const;

    std::vector<VideoFormatDescription> get_available_video_formats ();

    bool set_framerate (double framerate);

    double get_framerate ();

    bool set_sink (std::shared_ptr<SinkInterface>);

    bool initialize_buffers (std::vector<std::shared_ptr<MemoryBuffer>>);

    bool release_buffers ();

    bool start_stream ();

    bool stop_stream ();

private:

    std::thread work_thread;

    std::thread notification_thread;

    std::condition_variable cv;
    std::mutex mtx;

    int fd;

    VideoFormat active_video_format;

    std::vector<VideoFormatDescription> available_videoformats;
    bool emulate_bayer;
    uint32_t emulated_fourcc;

    // v4l2 uses fractions
    // to assure correct handling we store the values received by v4l2
    // to reuse them when necessary
    struct framerate_conv
    {
        double fps;
        unsigned int numerator;
        unsigned int denominator;
    };

    std::vector<framerate_conv> framerate_conversions;

    std::shared_ptr<V4L2PropertyHandler> property_handler;

    std::shared_ptr<V4L2FormatHandler> format_handler;

    unsigned char lost_countdown;
    bool stop_all;
    bool device_is_lost;
    bool abort_all;

    std::thread udev_monitor;

    void notification_loop ();

    void lost_device ();

    /**
     * @brief iterate over all v4l2 format descriptions and convert them
     *        into the internal representation
     */
    void index_formats ();

    std::vector<double> index_framerates (const struct v4l2_frmsizeenum& frms);

    void determine_active_video_format ();

    void create_emulated_properties ();

    void sort_properties ();

    std::shared_ptr<Property> apply_conversion_factor (std::shared_ptr<Property> prop,
                                                       const double factor);

    void create_conversion_factors ();

    void index_all_controls (std::shared_ptr<PropertyImpl> impl);

    int index_control (struct v4l2_queryctrl* qctrl, std::shared_ptr<PropertyImpl> impl);

    void add_control (struct v4l2_queryctrl* queryctrl,
                      struct v4l2_ext_control* ctrl,
                      std::shared_ptr<PropertyImpl> impl);

    bool propertyChangeEvent (const Property&);

    void updateV4L2Property (V4l2Device::property_description& desc);

    bool changeV4L2Control (const property_description&);

    // streaming related

    bool is_stream_on;
    struct tcam_stream_statistics statistics;
    size_t current_buffer;

    struct buffer_info
    {
        std::shared_ptr<MemoryBuffer> buffer;
        bool is_queued;
    };

    // std::vector<std::shared_ptr<MemoryBuffer>> buffers;
    std::vector<buffer_info> buffers;

    std::weak_ptr<SinkInterface> listener;

    void stream ();

    bool get_frame ();

    bool requeue_mmap_buffer ();

    void init_mmap_buffers ();

    void free_mmap_buffers ();

    tcam_image_size get_sensor_size () const;

    void monitor_v4l2_device ();
};

} /* namespace tcam */

VISIBILITY_POP

#endif /* TCAM_V4L2DEVICE_H */
