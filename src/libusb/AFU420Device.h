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

#include "DeviceInterface.h"
#include "VideoFormat.h"
#include "VideoFormatDescription.h"
#include "FormatHandlerInterface.h"
#include "UsbSession.h"
#include "LibusbDevice.h"
//#include "afu420_definitions.h"

#include "ep_defines_r42.h"
#include "ep_defines_rx.h"
#include "struct_defines_rx.h"

#include <libusb-1.0/libusb.h>
#include <memory>
#include <thread>
#include <mutex>              // std::mutex, std::unique_lock
#include <condition_variable> // std::condition_variable
#include <atomic>


VISIBILITY_INTERNAL

namespace tcam
{

class AFU420Device : public DeviceInterface
{


    struct property_description
    {
        std::shared_ptr<Property> property;
    };

    class AFU420PropertyHandler : public PropertyImpl
    {
        friend class AFU420Device;

    public:
        AFU420PropertyHandler (AFU420Device*);


        std::vector<std::shared_ptr<Property>> create_property_vector ();

        bool set_property (const Property&);
        bool get_property (Property&);

    protected:

        std::vector<struct property_description> properties;

        AFU420Device* device;
    };

    class AFU420FormatHandler : public FormatHandlerInterface
    {
        friend class AFU420Device;

    public:
        AFU420FormatHandler (AFU420Device*);
        std::vector<double> get_framerates (const struct tcam_image_size&, int pixelformat=0);

    protected:
        AFU420Device* device;
    };

public:

    explicit AFU420Device (const DeviceInfo&);

    AFU420Device () = delete;

    ~AFU420Device ();

    AFU420Device (AFU420Device&) = delete;

    AFU420Device& operator= (const AFU420Device&) = delete;

    DeviceInfo get_device_description () const;

    std::vector<std::shared_ptr<Property>> getProperties ();

    bool set_property (const Property&);

    bool get_property (Property&);

    bool set_video_format (const VideoFormat&);

    VideoFormat get_active_video_format () const;

    std::vector<VideoFormatDescription> get_available_video_formats ();

    bool set_framerate (double framerate);

    double get_framerate ();

    bool set_sink (std::shared_ptr<SinkInterface>);

    bool initialize_buffers (std::vector<std::shared_ptr<MemoryBuffer>>);

    bool release_buffers ();

    void requeue_buffer (std::shared_ptr<MemoryBuffer>);

    bool start_stream ();

    bool stop_stream ();


#pragma pack( push, 1)
    struct strobe_data
    {
        uint8_t		mode;
        uint32_t	delay_control;
        uint32_t	width_high_ctrl;
        uint32_t	width_low_ctrl;
        uint32_t	width2_high_ctrl;
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
        uint16_t    x_addr_start;				// ROI_Start always +4
        uint16_t    y_addr_start;				// ROI_Start always +4
        uint16_t    x_addr_end;					// ROI End
        uint16_t    y_addr_end;					// ROI End

        uint16_t    x_output_size;				// pixel to readout
        uint16_t    y_output_size;				// pixel to readout

        uint16_t    digital_crop_x_offset;		//
        uint16_t    digital_crop_y_offset;		//
        uint16_t    digital_crop_image_width;	// pixel to readout
        uint16_t    digital_crop_image_height;	// pixel to readout

        uint8_t     hor_binning;
        uint8_t     ver_binning;

        uint16_t    defaultFramerate;				// framerate
    };

private:


    enum class resolution_config_mode
    {
        test,
        set
    };

    std::unique_ptr<LibusbDevice> usb_device_;

    static constexpr tcam_image_size max_sensor_dim_ = { 7716, 5360 };
    static constexpr tcam_image_size min_sensor_dim_ = { 264, 256 };

    static constexpr int    m_uPixelMaxX = 7728 - 12 + 3;			// max horizontal: 7728;	-12: n?chst kleinere aufloesung;	+3: aufwerten f?r +4 pixel start offset
    static constexpr int    m_uPixelMaxY = 5368 - 4 + 3;			// max vertival: 5368;		 -4: n?chst kleinere aufloesung;	+3: aufwerten f?r +4 pixel start offset
    static constexpr int    m_uPixelMinX = 0x100;
    static constexpr int    m_uPixelMinY = 0x98;

    struct sResolutionConf active_resolution_conf_;

    struct stream_fmt_data
    {
        int         id;

        int         src_bpp;

        uint32_t    fmt_in;
        uint32_t    fmt_out;

        tcam_image_size dim_min;
        tcam_image_size dim_max;
        tcam_image_size dim_step;

        double      fps_min;
        double      fps_max;
    };

    std::thread work_thread;

    std::thread notification_thread;

    std::condition_variable cv;
    std::mutex mtx;

    VideoFormat active_video_format;

    std::vector<VideoFormatDescription> available_videoformats;

    std::vector<framerate_mapping> framerate_conversions;

    std::shared_ptr<AFU420PropertyHandler> property_handler;

    std::shared_ptr<AFU420FormatHandler> format_handler;

    bool stop_all;
    bool device_is_lost;

    std::thread udev_monitor;


    struct AFU420Device::sResolutionConf CreateResolutionConf (const tcam_image_size start,
                                                               const tcam_image_size stream_dim,
                                                               tcam_image_size binning);

    static std::vector<uint8_t> serialize_resolution_config (const struct AFU420Device::sResolutionConf& cfg);
    static AFU420Device::sResolutionConf deserialize_resolution_config (std::vector<uint8_t> serialized_data) noexcept( true );

    sResolutionConf videoformat_to_resolution_conf (const VideoFormat& format,
                                                    const std::shared_ptr<Property> binning_hor,
                                                    const std::shared_ptr<Property> binning_ver,
                                                    const std::shared_ptr<Property> offset_hor,
                                                    const std::shared_ptr<Property> offset_ver);

    void read_firmware_version ();

    tcam_image_size transform_roi_start (tcam_image_size pos, tcam_image_size video_dim);


    void notification_loop ();

    std::atomic_int lost_countdown;

    //void lost_device ();

    void determine_active_video_format ();

    void create_formats ();

    void create_property (struct property_description);

    void create_properties ();

    // streaming related

    struct buffer_info
    {
        std::shared_ptr<MemoryBuffer> buffer;
        bool is_queued;
    };

    std::vector<buffer_info> buffers;

    std::shared_ptr<tcam::MemoryBuffer> get_next_buffer ();

    volatile std::atomic_bool is_stream_on;
    struct tcam_stream_statistics statistics;

    size_t transfered_size_;
    unsigned int offset_;
    std::shared_ptr<MemoryBuffer> current_buffer_;

    std::weak_ptr<SinkInterface> listener;

    static void LIBUSB_CALL libusb_bulk_callback (struct libusb_transfer* trans);
    void transfer_callback (struct libusb_transfer* transfer);

    void push_buffer ();

    struct header_res
    {
        int frame_id;
        unsigned char* buffer;
        size_t size;
    };

    struct header_res check_and_eat_img_header (unsigned char* data,
                                                size_t data_size);

    bool get_frame ();

    void init_buffers ();

    void monitor_device ();

    bool update_property (property_description& desc);


    std::vector<stream_fmt_data>        stream_format_list_;

    const std::vector<stream_fmt_data>& get_stream_format_descs() const
    {
        return stream_format_list_;
    }

    int set_resolution_config (sResolutionConf conf, resolution_config_mode mode);

    int setup_bit_depth (int bpp);


    int get_fps_max (double& max,
                     tcam_image_size pos,
                     tcam_image_size dim,
                     tcam_image_size binning,
                     int src_bpp);

    int get_frame_rate_range (uint32_t strm_fmt_id,
                              int scaling_factor_id,
                              tcam_image_size dim,
                              double& min_fps, double& max_fps);

    void query_active_format ();

    int read_resolution_config_from_device (sResolutionConf& conf);

    struct bulk_transfer_item
    {
        //uint8_t buffer[512];
        std::vector<uint8_t> buffer;
        //uint8_t buffer[2048];
        void* transfer;

        ~bulk_transfer_item ()
        {
            if (transfer != nullptr)
            {
                libusb_free_transfer((libusb_transfer*)transfer);
            }
        }
    };

    std::vector<bulk_transfer_item> transfer_items;


    int no_buffer_counter = 0;
    static constexpr int no_buffer_counter_max = 10;
    unsigned int usbbulk_chunk_size_ = 0;
    unsigned int usbbulk_image_size_ = 0;
    static constexpr int actual_image_prefix_size_ = 4;

    int get_packet_header_size () const { return (actual_image_prefix_size_ * active_video_format.get_size().width * image_bit_depth_) / 8;};


    struct frame_rate_cache_item {
        uint32_t        strm_fmt_id;
        int             scaling_factor_id;
        tcam_image_size dim;

        double min_fps;
        double max_fps;
    };

    std::vector<frame_rate_cache_item>  frame_rate_cache_;
    int                     image_bit_depth_ = 8;

    int get_stream_bitdepth() const {return image_bit_depth_;};

    tcam_image_size get_stream_dim() {return active_video_format.get_size();};

    std::mutex control_transfer_mtx_;

    bool has_optics_;
    void check_for_optics ();
    bool has_optics () {return has_optics_;}
    bool has_ois_unit () {return has_optics_;};

    bool create_exposure ();
    bool create_gain ();
    bool create_focus ();
    bool create_hdr ();
    bool create_shutter ();
    bool create_color_gain ();
    bool create_strobe ();

    bool create_offsets ();
    bool create_binning ();
    bool create_ois ();

    int64_t get_exposure ();
    bool set_exposure(int64_t exposure_in_us);
    int64_t get_gain ();
    bool set_gain (int64_t gain);
    int64_t get_focus ();
    bool set_focus (int64_t focus);
    bool get_shutter ();
    bool set_shutter (bool open);

    int64_t get_hdr ();
    bool set_hdr (int64_t);

    bool get_color_gain_factor (color_gain eColor, double& dValue);
    bool set_color_gain_factor (color_gain eColor, int dValue);
    int read_strobe (strobe_data& strobe);
    int64_t get_strobe (strobe_parameter param);
    bool set_strobe (strobe_parameter param, int64_t);

    int64_t get_ois_mode ();
    bool set_ois_mode (int64_t mode);

    bool get_ois_pos (int64_t& x_pos,
                      int64_t& y_pos);
    bool set_ois_pos (const int64_t& x_pos,
                      const int64_t& y_pos);

    int control_write (unsigned char ucRequest, uint16_t ushValue, uint16_t ushIndex = 0);
    int control_write (unsigned char ucRequest, uint16_t ushValue, uint16_t ushIndex, uint8_t data);
    int control_write (unsigned char ucRequest, uint16_t ushValue, uint16_t ushIndex, uint16_t data);
    int control_write (unsigned char ucRequest, uint16_t ushValue, uint16_t ushIndex, uint32_t data);
    int control_write (unsigned char ucRequest, uint16_t ushValue, uint16_t ushIndex, std::vector<unsigned char>& data);

    template<class T>
    int read_reg (T& value, unsigned char ucRequest, uint16_t ushValue, uint16_t ushIndex);

    int control_read (uint8_t& value, unsigned char ucRequest,
                      uint16_t ushValue = 0, uint16_t ushIndex = 0);
    int control_read (uint16_t& value, unsigned char ucRequest,
                      uint16_t ushValue = 0, uint16_t ushIndex = 0);
    int control_read (uint32_t& value, unsigned char ucRequest,
                      uint16_t ushValue = 0, uint16_t ushIndex = 0);
    int control_read (uint64_t& value, unsigned char ucRequest,
                      uint16_t ushValue = 0, uint16_t ushIndex = 0);
    int control_read (std::vector<uint8_t>& value, unsigned char ucRequest,
                      uint16_t ushValue = 0, uint16_t ushIndex = 0);
};

} /* namespace tcam */

VISIBILITY_POP

#endif /* TCAM_AFU420DEVICE_H */
