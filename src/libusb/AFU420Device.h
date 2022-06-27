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

#ifndef TCAM_AFU420DEVICE_H
#define TCAM_AFU420DEVICE_H

#include "../DeviceInterface.h"
#include "../FormatHandlerInterface.h"
#include "../VideoFormat.h"
#include "../VideoFormatDescription.h"
#include "LibusbDevice.h"
#include "UsbSession.h"
#include "ep_defines_r42.h"
#include "ep_defines_rx.h"
#include "libusb_utils.h"
#include "struct_defines_rx.h"

#include <atomic>
#include <libusb-1.0/libusb.h>
#include <memory>
#include <mutex> // std::mutex, std::unique_lock

VISIBILITY_INTERNAL

namespace tcam
{

namespace property
{
class AFU420DeviceBackend;
}


class AFU420Device : public DeviceInterface
{
public:
    explicit AFU420Device(const DeviceInfo&);

    AFU420Device() = delete;

    ~AFU420Device();

    AFU420Device(AFU420Device&) = delete;

    AFU420Device& operator=(const AFU420Device&) = delete;

    DeviceInfo get_device_description() const final;

    std::vector<std::shared_ptr<tcam::property::IPropertyBase>> get_properties() final
    {
        return m_properties;
    }

    bool set_video_format(const VideoFormat&) final;

    VideoFormat get_active_video_format() const final;

    std::vector<VideoFormatDescription> get_available_video_formats() final;

    bool set_framerate(double framerate);

    double get_framerate();

    std::shared_ptr<tcam::AllocatorInterface> get_allocator() override
    {
        return get_default_allocator();
    };

    bool initialize_buffers(std::shared_ptr<BufferPool> pool) final;

    bool release_buffers() final;

    void requeue_buffer(const std::shared_ptr<ImageBuffer>&) final;

    bool start_stream(const std::shared_ptr<IImageBufferSink>& sink) final;

    void stop_stream() final;


#pragma pack(push, 1)
    struct strobe_data
    {
        uint8_t mode;
        uint32_t delay_control;
        uint32_t width_high_ctrl;
        uint32_t width_low_ctrl;
        uint32_t width2_high_ctrl;
    };
#pragma pack(pop)

    enum strobe_mode
    {
        off = 0,
        single_strobe = 1,
        double_strobe = 2,
    };

    enum class strobe_parameter
    {
        mode = 0,
        first_strobe_delay = 1,
        first_strobe_duration = 2,
        second_strobe_delay = 3,
        second_strobe_duration = 4,

        polarity = 5,
    };

    enum class color_gain
    {
        ColorGainRed,
        ColorGainGreen1,
        ColorGainGreen2,
        ColorGainBlue,
    };

    enum class trigger_modes
    {
        HardwareFreerun,
        SoftwareContinuous,
        SoftwareSingle
    };

    struct sResolutionConf
    {
        uint16_t x_addr_start; // ROI_Start always +4
        uint16_t y_addr_start; // ROI_Start always +4
        uint16_t x_addr_end; // ROI End
        uint16_t y_addr_end; // ROI End

        uint16_t x_output_size; // pixel to readout
        uint16_t y_output_size; // pixel to readout

        uint16_t digital_crop_x_offset; //
        uint16_t digital_crop_y_offset; //
        uint16_t digital_crop_image_width; // pixel to readout
        uint16_t digital_crop_image_height; // pixel to readout

        uint8_t hor_binning;
        uint8_t ver_binning;

        uint16_t defaultFramerate; // framerate
    };

private:
    enum class resolution_config_mode
    {
        test,
        set
    };

    std::unique_ptr<LibusbDevice> usb_device_;

    const tcam_image_size max_sensor_dim_ = { 7716, 5360 };
    const tcam_image_size min_sensor_dim_ = { 264, 256 };

    tcam_image_size max_sensor_dim_by12 = { 5424, 5360 };
    tcam_image_size step = { 12, 4 };

    static const int m_uPixelMaxX =
        7728 - 12
        + 3; // max horizontal: 7728;	-12: n?chst kleinere aufloesung;	+3: aufwerten f?r +4 pixel start offset
    static const int m_uPixelMaxY =
        5368 - 4
        + 3; // max vertival: 5368;		 -4: n?chst kleinere aufloesung;	+3: aufwerten f?r +4 pixel start offset
    static const int m_uPixelMinX = 0x100;
    static const int m_uPixelMinY = 0x98;

    struct sResolutionConf active_resolution_conf_;

    struct stream_fmt_data
    {
        int id;

        int src_bpp;

        uint32_t fmt;

        tcam_image_size dim_min;
        tcam_image_size dim_max;
        tcam_image_size dim_step;

        double fps_min;
        double fps_max;
    };

    const std::vector<image_scaling> m_available_scaling = {
        { 1, 1, 1, 1 },
        { 2, 2, 1, 1 },
        { 4, 4, 1, 1 },
        { 8, 8, 1, 1 },
    };

    VideoFormat active_video_format;

    std::vector<VideoFormatDescription> available_videoformats;

    std::vector<framerate_mapping> framerate_conversions;

    std::vector<std::shared_ptr<tcam::property::IPropertyBase>> m_properties;
    std::shared_ptr<tcam::property::AFU420DeviceBackend> m_backend;

    struct AFU420Device::sResolutionConf CreateResolutionConf(const tcam_image_size start,
                                                              const tcam_image_size stream_dim,
                                                              tcam_image_size binning);

    static std::vector<uint8_t> serialize_resolution_config(
        const struct AFU420Device::sResolutionConf& cfg);
    static AFU420Device::sResolutionConf deserialize_resolution_config(
        std::vector<uint8_t> serialized_data) noexcept(true);

    sResolutionConf videoformat_to_resolution_conf(const VideoFormat& format);

    void read_firmware_version();

    tcam_image_size transform_roi_start(tcam_image_size pos, tcam_image_size video_dim);


    std::atomic_int lost_countdown_ = 20;

    void create_formats();

    void create_properties();

    // streaming related

    struct buffer_info
    {
        std::shared_ptr<ImageBuffer> buffer;
        bool is_queued;
    };

    tcam::libusb::deliver_thread deliver_thread_;

    std::vector<buffer_info> buffer_list_;
    std::mutex buffers_mutex_;

    std::shared_ptr<tcam::ImageBuffer> get_next_buffer();

    std::atomic_bool is_stream_on_ = false;
    size_t frames_delivered_ = 0;
    size_t frames_dropped_ = 0;

    int transfer_offset_ = 0;
    bool have_header_ = false;
    std::shared_ptr<ImageBuffer> current_buffer_;   // contains the buffer that image data is copied into

    std::weak_ptr<IImageBufferSink> listener_;

    static void LIBUSB_CALL libusb_bulk_callback(struct libusb_transfer* trans);
    void transfer_callback(struct libusb_transfer* transfer);

    void push_buffer(std::shared_ptr<tcam::ImageBuffer>&&);

    struct header_res
    {
        int frame_id;
        unsigned char* buffer;
        size_t size;
    };

    struct header_res check_and_eat_img_header(unsigned char* data, size_t data_size);

    std::vector<stream_fmt_data> stream_format_list_;

    const std::vector<stream_fmt_data>& get_stream_format_descs() const
    {
        return stream_format_list_;
    }

    int set_resolution_config(sResolutionConf conf, resolution_config_mode mode);

    int setup_bit_depth(int bpp);


    int get_fps_max(double& max,
                    tcam_image_size pos,
                    tcam_image_size dim,
                    tcam_image_size binning,
                    int src_bpp);

    int get_frame_rate_range(uint32_t strm_fmt_id,
                             int scaling_factor_id,
                             tcam_image_size dim,
                             double& min_fps,
                             double& max_fps);

    void query_active_format();

    int read_resolution_config_from_device(sResolutionConf& conf);

    struct bulk_transfer_item
    {
        std::vector<uint8_t> buffer;
        void* transfer = nullptr;

        ~bulk_transfer_item()
        {
            if (transfer != nullptr)
            {
                libusb_free_transfer((libusb_transfer*)transfer);
            }
        }
    };

    std::vector<bulk_transfer_item> transfer_items;

    static const int actual_image_prefix_size_ = 4;
    int usbbulk_chunk_size_ = 0;
    int usbbulk_image_size_ = 0;

    size_t get_packet_header_size() const
    {
        return (actual_image_prefix_size_ * active_video_format.get_size().width * image_bit_depth_)
               / 8;
    };


    struct frame_rate_cache_item
    {
        uint32_t strm_fmt_id;
        int scaling_factor_id;
        tcam_image_size dim;

        double min_fps;
        double max_fps;
    };

    std::vector<frame_rate_cache_item> frame_rate_cache_;
    int image_bit_depth_ = 8;

    int get_stream_bitdepth() const
    {
        return image_bit_depth_;
    };

    tcam_image_size get_stream_dim()
    {
        return active_video_format.get_size();
    };

    bool has_optics_;
    void check_for_optics();

    bool create_exposure();
    bool create_gain();
    bool create_focus();
    bool create_hdr();
    bool create_iris();
    bool create_color_gain();
    bool create_strobe();

    bool create_offsets();
    bool create_ois();
    void create_sensor_dimensions();

    friend class property::AFU420DeviceBackend;

protected:
    bool has_optics()
    {
        return has_optics_;
    }
    bool has_ois_unit()
    {
        return has_optics_;
    };

    int64_t get_exposure();
    bool set_exposure(int64_t exposure_in_us);
    int64_t get_gain();
    bool set_gain(int64_t gain);
    int64_t get_focus();
    bool set_focus(int64_t focus);

    // can not be read from device
    bool m_iris = true;

    bool get_iris()
    {
        return m_iris;
    };
    bool set_iris(bool open);

    int64_t get_hdr();
    bool set_hdr(int64_t);

    bool get_color_gain_factor(color_gain eColor, double& dValue);
    bool set_color_gain_factor(color_gain eColor, double dValue);
    int read_strobe(strobe_data& strobe);
    int64_t get_strobe(strobe_parameter param);
    bool set_strobe(strobe_parameter param, int64_t);

    int m_ois_pos_x = 0;
    int m_ois_pos_y = 0;

    int64_t get_ois_mode();
    bool set_ois_mode(int64_t mode);

    bool get_ois_pos(int64_t& x_pos, int64_t& y_pos);
    bool set_ois_pos(const int64_t& x_pos, const int64_t& y_pos);

    tcam_image_size m_binning = { 1, 1 };
    tcam_image_size m_offset = { 0, 0 };
    bool m_offset_auto = true;

    tcam_image_size calculate_auto_offset(uint32_t fourcc,
                                          const tcam_image_size& size,
                                          const tcam_image_size& binning) const;

private:
    int control_write(unsigned char ucRequest, uint16_t ushValue, uint16_t ushIndex = 0);
    int control_write(unsigned char ucRequest, uint16_t ushValue, uint16_t ushIndex, uint8_t data);
    int control_write(unsigned char ucRequest, uint16_t ushValue, uint16_t ushIndex, uint16_t data);
    int control_write(unsigned char ucRequest, uint16_t ushValue, uint16_t ushIndex, uint32_t data);
    int control_write(unsigned char ucRequest,
                      uint16_t ushValue,
                      uint16_t ushIndex,
                      std::vector<unsigned char>& data);

    template<class T>
    int read_reg(T& value, unsigned char ucRequest, uint16_t ushValue, uint16_t ushIndex);

    int control_read(uint8_t& value,
                     unsigned char ucRequest,
                     uint16_t ushValue = 0,
                     uint16_t ushIndex = 0);
    int control_read(uint16_t& value,
                     unsigned char ucRequest,
                     uint16_t ushValue = 0,
                     uint16_t ushIndex = 0);
    int control_read(uint32_t& value,
                     unsigned char ucRequest,
                     uint16_t ushValue = 0,
                     uint16_t ushIndex = 0);
    int control_read(uint64_t& value,
                     unsigned char ucRequest,
                     uint16_t ushValue = 0,
                     uint16_t ushIndex = 0);
    int control_read(std::vector<uint8_t>& value,
                     unsigned char ucRequest,
                     uint16_t ushValue = 0,
                     uint16_t ushIndex = 0);
};

} /* namespace tcam */

VISIBILITY_POP

#endif /* TCAM_AFU420DEVICE_H */
