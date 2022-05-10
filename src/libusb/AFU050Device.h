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

#include "../DeviceInterface.h"
#include "../VideoFormat.h"
#include "../VideoFormatDescription.h"
#include "LibusbDevice.h"
#include "afu050_definitions.h"

#include <libusb-1.0/libusb.h>
#include <map>
#include <memory>
#include <mutex> // std::mutex, std::unique_lock

VISIBILITY_INTERNAL

namespace tcam
{

namespace property
{
class AFU050DeviceBackend;
}

class AFU050Device : public DeviceInterface
{
public:
    explicit AFU050Device(const DeviceInfo&);

    AFU050Device() = delete;

    ~AFU050Device();

    DeviceInfo get_device_description() const final;

    std::vector<std::shared_ptr<tcam::property::IPropertyBase>> get_properties() final
    {
        return properties_;
    }

    bool set_video_format(const VideoFormat&) final;

    bool validate_video_format(const VideoFormat&) const;

    VideoFormat get_active_video_format() const final;

    std::vector<VideoFormatDescription> get_available_video_formats() final;

    bool set_framerate(double framerate);

    double get_framerate();

    std::shared_ptr<tcam::AllocatorInterface> get_allocator() override
    {
        return tcam::get_default_allocator();
    };

    bool initialize_buffers(std::shared_ptr<BufferPool> pool) final;

    bool release_buffers() final;

    void requeue_buffer(const std::shared_ptr<ImageBuffer>&) final;

    bool start_stream(const std::shared_ptr<IImageBufferSink>&) final;

    void stop_stream() final;

    bool set_control(int unit, int ctrl, int len, unsigned char* value);

    bool get_control(int unit,
                     int ctrl,
                     int len,
                     unsigned char* value,
                     enum CONTROL_CMD cmd = GET_CUR);

private:
    std::unique_ptr<LibusbDevice> usb_device_;

    static const int UVC_SET_CUR = 0x1;
    static const int UVC_GET_CUR = 0x81;
    static const int LEN_IN_BUFFER = 1024 * 32;
    static const int TRANSFER_COUNT = 32;
    static const int JPEGBUF_SIZE = 1024 * 1024 * 5;

    VideoFormat active_video_format_;

    std::vector<VideoFormatDescription> available_videoformats_;

    std::vector<framerate_mapping> framerate_conversions_;

    std::vector<std::shared_ptr<tcam::property::IPropertyBase>> properties_;

    std::shared_ptr<tcam::property::AFU050DeviceBackend> backend_;

    std::atomic_bool device_is_lost_ = false;

    void lost_device();

    void create_formats();

    void add_dependency_tracking();

    void create_properties();


    // streaming related

    struct buffer_info
    {
        std::shared_ptr<ImageBuffer> buffer;
        bool is_queued;
    };

    std::vector<buffer_info> buffer_list_;

    std::mutex buffer_list_mtx_;

    std::atomic_bool is_stream_on_ = false;
    long frames_delivered_ = 0;
    long frames_dropped_ = 0;

    size_t current_jpegsize_ = 0;
    int current_jpegbuf_index_ = 0;
    unsigned char* current_jpegbuf_ptr_ = nullptr;
    std::vector<uint8_t> current_jpegbuf_data_;

    std::weak_ptr<IImageBufferSink> listener_;

    std::vector<libusb_transfer*> transfers_;

    std::shared_ptr<tcam::ImageBuffer> get_free_buffer();

    static void LIBUSB_CALL libusb_bulk_callback(struct libusb_transfer* trans);
    void transfer_callback(libusb_transfer* transfer);

    void init_buffers();

    void add_int(const std::string& name, const VC_UNIT unit, const unsigned char prop);
    void add_double(const std::string& name,
                    const VC_UNIT unit,
                    const unsigned char prop,
                    double modifier = 1.0);
    void add_enum(const std::string& name,
                  const VC_UNIT unit,
                  const unsigned char prop,
                  std::map<int, std::string> entries);

    int set_video_format(uint8_t format_index, uint8_t frame_index, uint32_t frame_interval);

    bool get_bool_value(enum VC_UNIT unit, unsigned char property, enum CONTROL_CMD);

    bool set_bool_value(enum VC_UNIT unit, unsigned char property, bool value);

    int get_int_value(const enum VC_UNIT unit,
                      const unsigned char property,
                      const enum CONTROL_CMD);

    bool set_int_value(enum VC_UNIT unit, unsigned char property, int value);
};

} /* namespace tcam */

VISIBILITY_POP

#endif /* TCAM_AFU050DEVICE_H */
