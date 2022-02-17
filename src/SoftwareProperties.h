

#pragma once

#include "PropertyInterfaces.h"
#include "SoftwarePropertiesBase.h"
#include "SoftwarePropertiesImpl.h"
#include "VideoFormat.h"
#include "compiler_defines.h"

#include <dutils_img_pipe/auto_alg_pass.h>
#include <memory>
#include <mutex>
#include <vector>
namespace tcam::property
{

class SoftwareProperties :
    public std::enable_shared_from_this<SoftwareProperties>,
    public emulated::SoftwarePropertyBackend
{
public:
    SoftwareProperties(
        const std::vector<std::shared_ptr<tcam::property::IPropertyBase>>& dev_properties);

public:
    static std::shared_ptr<SoftwareProperties> create(
        const std::vector<std::shared_ptr<tcam::property::IPropertyBase>>& dev_properties,
        bool has_bayer);

    std::vector<std::shared_ptr<tcam::property::IPropertyBase>> get_properties()
    {
        return m_properties;
    }

    void auto_pass(const img::img_descriptor& image);

    outcome::result<int64_t> get_int(emulated::software_prop prop_id) final;
    outcome::result<void> set_int(emulated::software_prop prop_id, int64_t new_val) final;

    outcome::result<double> get_double(emulated::software_prop) final;
    outcome::result<void> set_double(emulated::software_prop, double) final;

    tcam::property::PropertyFlags get_flags(emulated::software_prop) const final;

    void update_to_new_format(const tcam::VideoFormat& new_format);

private:
    // encapsulation for internal property generation
    void generate_public_properties(bool has_bayer);

    void generate_exposure_auto();
    void generate_gain_auto();
    void generate_iris_auto();
    void generate_focus_auto();

    void generate_balance_white_auto();

    outcome::result<double> get_whitebalance_channel(emulated::software_prop prop_id);
    outcome::result<void> set_whitebalance_channel(emulated::software_prop prop_id,
                                                   double new_value);

    void generate_color_transformation();

    outcome::result<double> get_device_color_transform(emulated::software_prop prop_id);

    outcome::result<void> set_device_color_transform(emulated::software_prop prop_id,
                                                     double new_value_tmp);

    using prop_ptr_vec = std::vector<std::shared_ptr<tcam::property::IPropertyBase>>;

    // property-list
    std::vector<std::shared_ptr<tcam::property::IPropertyBase>> m_properties;

    mutable std::mutex m_property_mtx;

    std::shared_ptr<tcam::property::IPropertyFloat> m_dev_exposure = nullptr;

    bool m_exposure_auto_upper_limit_auto = true;
    double m_exposure_auto_upper_limit = 0;

    std::shared_ptr<tcam::property::IPropertyFloat> m_dev_gain = nullptr;
    std::shared_ptr<tcam::property::IPropertyInteger> m_dev_iris = nullptr;
    std::shared_ptr<tcam::property::IPropertyInteger> m_dev_focus = nullptr;

    struct wb_setter
    {
        std::shared_ptr<tcam::property::IPropertyFloat> m_dev_wb_r = nullptr;
        std::shared_ptr<tcam::property::IPropertyFloat> m_dev_wb_g = nullptr;
        std::shared_ptr<tcam::property::IPropertyFloat> m_dev_wb_b = nullptr;

        bool m_wb_is_claimed = false;
        bool m_is_software_auto_wb = false;

        bool is_dev_wb() const
        {
            return m_dev_wb_r != nullptr;
        }
    };
    wb_setter m_wb;

    // color transforms stuff

    std::shared_ptr<tcam::property::IPropertyBool> m_dev_color_transform_enable = nullptr;
    std::shared_ptr<tcam::property::IPropertyFloat> m_dev_color_transform_value = nullptr;
    std::shared_ptr<tcam::property::IPropertyEnum> m_dev_color_transform_value_selector = nullptr;

    // general stuff

    auto_alg::auto_pass_params m_auto_params;
    auto_alg::state_ptr p_state;
    tcam::VideoFormat m_format;

    int64_t m_frame_counter = 0;


    template<class Tprop_info_type, typename... Tparams>
    auto make_prop_entry(emulated::software_prop id,
                         const Tprop_info_type* prop_info,
                         Tparams&&... params) -> std::shared_ptr<IPropertyBase>
    {
        if constexpr (Tprop_info_type::property_type == tcamprop1::prop_type::Boolean)
        {
            return std::make_shared<tcam::property::emulated::SoftwarePropertyBoolImpl>(
                shared_from_this(), id, prop_info, std::forward<Tparams>(params)...);
        }
        else if constexpr (Tprop_info_type::property_type == tcamprop1::prop_type::Integer)
        {
            return std::make_shared<tcam::property::emulated::SoftwarePropertyIntegerImpl>(
                shared_from_this(), id, prop_info, std::forward<Tparams>(params)...);
        }
        else if constexpr (Tprop_info_type::property_type == tcamprop1::prop_type::Float)
        {
            return std::make_shared<tcam::property::emulated::SoftwarePropertyDoubleImpl>(
                shared_from_this(), id, prop_info, std::forward<Tparams>(params)...);
        }
        else if constexpr (Tprop_info_type::property_type == tcamprop1::prop_type::Enumeration)
        {
            return std::make_shared<tcam::property::emulated::SoftwarePropertyEnumImpl>(
                shared_from_this(), id, prop_info, std::forward<Tparams>(params)...);
        }
        else
        {
            static_assert(Tprop_info_type::property_type == tcamprop1::prop_type::Enumeration);
            return nullptr;
        }
    }

    template<class Tprop_info_type, typename... Tparams>
    void add_prop_entry(prop_ptr_vec& v,
                        emulated::software_prop id,
                        const Tprop_info_type* prop_info,
                        Tparams&&... params)
    {
        v.push_back(
            make_prop_entry<Tprop_info_type>(id, prop_info, std::forward<Tparams>(params)...));
    }

    static void add_prop_entry(prop_ptr_vec& v,
                               std::string_view name_to_insert_after,
                               const prop_ptr_vec& vec_to_add);
    static void replace_entry(prop_ptr_vec& v, const std::shared_ptr<IPropertyBase>& prop);
    static void remove_entry(prop_ptr_vec& v, std::string_view name);
}; //class SoftwareProperties

} // namespace tcam::property
