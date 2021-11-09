

#pragma once

#include "PropertyInterfaces.h"
#include "SoftwarePropertiesBase.h"
#include "SoftwarePropertyBackend.h"
#include "VideoFormat.h"
#include "compiler_defines.h"

#include <dutils_img_pipe/auto_alg_pass.h>
#include <memory>
#include <mutex>
#include <vector>

//VISIBILITY_INTERNAL

namespace tcam::property
{

namespace emulated
{
class SoftwarePropertiesBackend;
}

class SoftwareProperties
{
public:
    explicit SoftwareProperties(
        const std::vector<std::shared_ptr<tcam::property::IPropertyBase>>& dev_properties,
        bool has_bayer);

    std::vector<std::shared_ptr<tcam::property::IPropertyBase>> get_properties()
    {
        return m_properties;
    }

    void auto_pass(const img::img_descriptor& image);


    outcome::result<int64_t> get_int(emulated::software_prop prop_id);
    outcome::result<void> set_int(emulated::software_prop prop_id, int64_t new_val);

    outcome::result<double> get_double(emulated::software_prop);
    outcome::result<void> set_double(emulated::software_prop, double);

    tcam::property::PropertyFlags get_flags(emulated::software_prop);

    void update_to_new_format(const tcam::VideoFormat& new_format);

private:
    // encapsulation for internal property generation
    void generate_public_properties(bool has_bayer);

    void generate_exposure();
    void generate_gain();
    void generate_iris();
    void generate_focus();
    void convert_whitebalance();
    void generate_whitebalance();

    outcome::result<double> get_device_wb(emulated::software_prop prop_id);

    outcome::result<void> set_device_wb(emulated::software_prop prop_id,
                                        double new_value);


    void set_locked(emulated::software_prop prop_id, bool is_locked);

    void enable_property(emulated::software_prop prop_name, bool device_flags=false);

    void enable_property_double(emulated::software_prop prop_name,
                                std::shared_ptr<IPropertyFloat> prop);
    void enable_property_double(emulated::software_prop prop_name,
                                std::shared_ptr<IPropertyInteger> prop);

    void enable_property_int(emulated::software_prop prop_id,
                             std::shared_ptr<IPropertyInteger> prop);

    // properties the actual camera has
    std::vector<std::shared_ptr<tcam::property::IPropertyBase>> m_device_properties;

    // properties the user has
    std::vector<std::shared_ptr<tcam::property::IPropertyBase>> m_properties;

    auto_alg::property_cont_exposure m_brightness_reference;

    std::mutex m_property_mtx;

    std::shared_ptr<tcam::property::IPropertyFloat> m_dev_exposure = nullptr;
    bool m_exposure_upper_auto = true;
    double m_exposure_auto_upper_limit = 0;

    std::shared_ptr<tcam::property::IPropertyFloat> m_dev_gain = nullptr;
    std::shared_ptr<tcam::property::IPropertyInteger> m_dev_iris = nullptr;
    std::shared_ptr<tcam::property::IPropertyInteger> m_dev_focus = nullptr;

    std::shared_ptr<tcam::property::IPropertyInteger> m_dev_wb_r = nullptr;
    std::shared_ptr<tcam::property::IPropertyInteger> m_dev_wb_g = nullptr;
    std::shared_ptr<tcam::property::IPropertyInteger> m_dev_wb_b = nullptr;

    bool m_wb_is_claimed = false;

    enum class wb_type
    {
        None,
        DevChannel,
        DevSelector,
        Software
    };

    enum class wb_channel
    {
        Red,
        Green,
        Blue,
    };

    struct wb_setter
    {
        wb_type type = wb_type::None;


        std::shared_ptr<tcam::property::IPropertyInteger> m_dev_wb_r = nullptr;
        std::shared_ptr<tcam::property::IPropertyInteger> m_dev_wb_g = nullptr;
        std::shared_ptr<tcam::property::IPropertyInteger> m_dev_wb_b = nullptr;

        std::shared_ptr<tcam::property::IPropertyEnum> m_dev_wb_selector = nullptr;
        std::shared_ptr<tcam::property::IPropertyFloat> m_dev_wb_ratio = nullptr;

        bool is_dev_wb () const
        {
            if (type == wb_type::DevSelector
                || type == wb_type::DevChannel)
            {
                return true;
            }
            return false;
        }

    };
    wb_setter m_wb;

    std::shared_ptr<emulated::SoftwarePropertyBackend> m_backend = nullptr;

    auto_alg::auto_pass_params m_auto_params;
    auto_alg::state_ptr p_state;
    tcam::VideoFormat m_format;

    int64_t     m_frame_counter = 0;
};

} // namespace tcam::property

//VISIBILITY_POP
