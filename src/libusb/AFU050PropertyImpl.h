

#pragma once

#include "../PropertyInterfaces.h"
#include "afu050_definitions.h"
#include <tcamprop1.0_base/tcamprop_property_info.h>

#include "../property_dependencies.h"

#include <map>
#include <memory>

namespace tcam::property
{

class AFU050DeviceBackend;

class AFU050PropertyLockImpl : public PropertyLock
{
protected:
    AFU050PropertyLockImpl(std::string_view name);

    /**
     * Function called when this property wants to know if dependent properties should be locked.
     */
    virtual bool should_set_dependent_locked() const
    {
        return false;
    }
    void set_dependent_properties(std::vector<std::weak_ptr<PropertyLock>>&& controls) override;
    auto get_dependent_names() const -> std::vector<std::string_view> override;

protected:
    void update_dependent_lock_state();

    auto get_dependency_entry() const noexcept
    {
        return dependency_info_;
    }

private:
    std::vector<std::weak_ptr<PropertyLock>> dependent_controls_;

    const tcam::property::dependency_entry* dependency_info_ = nullptr;
};


class AFU050PropertyIntegerImpl : public IPropertyInteger, public AFU050PropertyLockImpl
{
public:
    AFU050PropertyIntegerImpl(const std::string& name, const tcam_value_int& val_def);
    AFU050PropertyIntegerImpl(const std::string& name,
                              control_definition ctrl,
                              std::shared_ptr<tcam::property::AFU050DeviceBackend> backend);

    virtual tcamprop1::prop_static_info get_static_info() const final
    {
        if (p_static_info)
        {
            return *p_static_info;
        }
        return tcamprop1::prop_static_info { /*.name =*/m_name, {}, {}, {} };
    }

    virtual std::string_view get_unit() const final;
    virtual tcamprop1::IntRepresentation_t get_representation() const final;

    virtual PropertyFlags get_flags() const final
    {
        return m_flags;
    };
    virtual void set_flags(PropertyFlags flags) final
    {
        m_flags = flags;
    }

    void set_locked(bool new_locked_state) override
    {
        lock(m_flags, new_locked_state);
    }

    virtual tcamprop1::prop_range_integer get_range() const final
    {
        return { m_min, m_max, m_step };
    }
    virtual outcome::result<int64_t> get_default() const final
    {
        return m_default;
    }

    virtual outcome::result<int64_t> get_value() const final;

    virtual outcome::result<void> set_value(int64_t new_value) final;
    //bool should_set_dependent_locked() const final;

private:
    std::weak_ptr<tcam::property::AFU050DeviceBackend> m_cam;

    std::string m_name;
    tcam::property::PropertyFlags m_flags;
    int64_t m_min;
    int64_t m_max;
    int64_t m_step;
    int64_t m_default;
    int64_t m_value = 0;

    control_definition m_ctrl = {};

    const tcamprop1::prop_static_info_integer* p_static_info = nullptr;
};


class AFU050PropertyDoubleImpl : public IPropertyFloat, public AFU050PropertyLockImpl
{
public:
    AFU050PropertyDoubleImpl(const std::string& name,
                             control_definition ctrl,
                             std::shared_ptr<tcam::property::AFU050DeviceBackend> backend,
                             double modifier = 1.0);

    virtual tcamprop1::prop_static_info get_static_info() const final
    {
        if (p_static_info)
        {
            return *p_static_info;
        }
        return tcamprop1::prop_static_info { /*.name =*/m_name, {}, {}, {} };
    }
    virtual std::string_view get_unit() const final;
    virtual tcamprop1::FloatRepresentation_t get_representation() const final;

    virtual PropertyFlags get_flags() const final
    {
        return m_flags;
    }
    virtual void set_flags(PropertyFlags flags) final
    {
        m_flags = flags;
    }

    void set_locked(bool new_locked_state) override
    {
        lock(m_flags, new_locked_state);
    }

    virtual tcamprop1::prop_range_float get_range() const final
    {
        return { m_min, m_max, m_step };
    }

    virtual outcome::result<double> get_default() const final
    {
        return m_default;
    }

    virtual outcome::result<double> get_value() const final;

    virtual outcome::result<void> set_value(double new_value) final;
    //bool should_set_dependent_locked() const final;

private:
    outcome::result<void> valid_value(double val);

    std::weak_ptr<tcam::property::AFU050DeviceBackend> m_cam;

    std::string m_name;
    tcam::property::PropertyFlags m_flags;
    double m_min;
    double m_max;
    double m_step;
    double m_default;
    // only really used for whitebalance
    // not the cleanest solution
    // due to small scope of implementation
    // deemed acceptable for now
    double m_modifier;

    control_definition m_ctrl;

    const tcamprop1::prop_static_info_float* p_static_info;
};


class AFU050PropertyEnumImpl : public IPropertyEnum, public AFU050PropertyLockImpl
{
public:
    AFU050PropertyEnumImpl(const std::string& name,
                           control_definition ctrl,
                           std::map<int, std::string> m_entries,
                           std::shared_ptr<AFU050DeviceBackend> backend);

    virtual tcamprop1::prop_static_info get_static_info() const final
    {
        if (p_static_info)
        {
            return *p_static_info;
        }
        return tcamprop1::prop_static_info { /*.name =*/m_name, {}, {}, {} };
    }

    virtual PropertyFlags get_flags() const final
    {
        return m_flags;
    }
    virtual void set_flags(PropertyFlags flags) final
    {
        m_flags = flags;
    }

    void set_locked(bool new_locked_state) override
    {
        lock(m_flags, new_locked_state);
    }

    outcome::result<void> set_value(std::string_view new_value) final;
    outcome::result<void> set_value_int(int64_t new_value);

    virtual outcome::result<std::string_view> get_value() const final;
    virtual outcome::result<int64_t> get_value_int() const final;

    outcome::result<std::string_view> get_default() const final
    {
        return m_default;
    }

    virtual std::vector<std::string> get_entries() const final;
    bool should_set_dependent_locked() const final;

private:
    bool valid_value(int value);

    std::map<int, std::string> m_entries;
    std::weak_ptr<AFU050DeviceBackend> m_cam;

    std::string m_name;
    PropertyFlags m_flags;

    std::string m_default;

    control_definition m_ctrl;

    const tcamprop1::prop_static_info_enumeration* p_static_info;
};


} // namespace tcam::property
