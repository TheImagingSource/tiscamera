/*
 * Copyright 2017 The Imaging Source Europe GmbH
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

#ifndef TCAM_AFU050DEVICE_H
#define TCAM_AFU050DEVICE_H

#include "DeviceInterface.h"
#include "VideoFormat.h"
#include "VideoFormatDescription.h"
#include "FormatHandlerInterface.h"
#include "UsbSession.h"
#include "afu050_definitions.h"

#include <libusb-1.0/libusb.h>
#include <memory>
#include <thread>
#include <mutex>              // std::mutex, std::unique_lock
#include <condition_variable> // std::condition_variable

VISIBILITY_INTERNAL

namespace tcam
{

class AFU050Device : public DeviceInterface
{
    typedef enum
    {
        FMT2592x1944 = 1,
        FMT1920x1080 = 2,
        FMT1280x960 = 3
    }AFU050_VIDEO_FORMAT;


    struct property_description
    {
        VC_UNIT unit;
        int id; // v4l2 identification
        std::shared_ptr<Property> prop;
    };

    class AFU050PropertyHandler : public PropertyImpl
    {
        friend class AFU050Device;

    public:
        AFU050PropertyHandler (AFU050Device*);


        std::vector<std::shared_ptr<Property>> create_property_vector ();

        bool set_property (const Property&);
        bool get_property (Property&);

    protected:

        std::vector<property_description> properties;

        AFU050Device* device;
    };

    class AFU050FormatHandler : public FormatHandlerInterface
    {
        friend class AFU050Device;

    public:
        AFU050FormatHandler (AFU050Device*);
        std::vector<double> get_framerates (const struct tcam_image_size&, int pixelformat=0);

    protected:
        AFU050Device* device;
    };

public:

    explicit AFU050Device (const DeviceInfo&,
                           std::shared_ptr<UsbSession> session);

    AFU050Device () = delete;

    ~AFU050Device ();

    DeviceInfo get_device_description () const;

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

    std::shared_ptr<UsbSession> session;
    struct libusb_device_handle* device_handle;

    static const int UVC_SET_CUR = 0x1;
    static const int UVC_GET_CUR = 0x81;
    static const int LEN_IN_BUFFER = 1024*32;
    static const int TRANSFER_COUNT = 32;
    static const int JPEGBUF_SIZE = 1024*1024*5;

    std::thread work_thread;

    std::thread notification_thread;

    std::condition_variable cv;
    std::mutex mtx;

    VideoFormat active_video_format;

    std::vector<VideoFormatDescription> available_videoformats;

    std::vector<framerate_mapping> framerate_conversions;

    std::shared_ptr<AFU050PropertyHandler> property_handler;

    std::shared_ptr<AFU050FormatHandler> format_handler;

    unsigned char lost_countdown;
    bool stop_all;
    bool device_is_lost;
    bool abort_all;

    std::thread udev_monitor;

    void notification_loop ();

    void lost_device ();

    void determine_active_video_format ();

    void create_formats ();

    void create_properties ();

    // streaming related

    struct buffer_info
    {
        std::shared_ptr<MemoryBuffer> buffer;
        bool is_queued;
    };

    std::vector<buffer_info> buffers;

    std::shared_ptr<tcam::MemoryBuffer> get_next_buffer ();

    bool is_stream_on;
    struct tcam_stream_statistics statistics;

    size_t current_buffer;
    size_t jpegsize;
    int jpegptr;
    unsigned char* jpegbuf;

    std::weak_ptr<SinkInterface> listener;

    std::vector<struct libusb_transfer*> transfers;

    static void LIBUSB_CALL libusb_bulk_callback (struct libusb_transfer* trans);
    void transfer_callback (struct libusb_transfer* transfer);

    void stream ();

    bool get_frame ();

    void init_buffers ();

    void monitor_device ();

    void add_int (const TCAM_PROPERTY_ID id, const VC_UNIT unit, const unsigned char prop);

    void add_bool (TCAM_PROPERTY_ID id, VC_UNIT unit, unsigned char prop);

    bool update_property (property_description& desc);

    bool set_control (int unit, int ctrl, int len, unsigned char *value);

    bool get_control (int unit, int ctrl, int len, unsigned char *value, enum CONTROL_CMD cmd=GET_CUR);

    int set_video_format (uint8_t format_index ,uint8_t frame_index, uint32_t frame_interval);

    bool get_bool_value (enum VC_UNIT unit, unsigned char property, enum CONTROL_CMD);

    bool set_bool_value (enum VC_UNIT unit, unsigned char property, bool value);

    int get_int_value (const enum VC_UNIT unit, const unsigned char property, const enum CONTROL_CMD);

    bool set_int_value (enum VC_UNIT unit, unsigned char property, int value);

};

} /* namespace tcam */

VISIBILITY_POP

#endif /* TCAM_AFU050DEVICE_H */
