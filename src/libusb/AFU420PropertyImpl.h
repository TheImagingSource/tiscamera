

#pragma once

#include "../PropertyInterfaces.h"
#include "../compiler_defines.h"
#include "ep_defines_rx.h"

#include <map>
#include <memory>

VISIBILITY_INTERNAL

namespace tcam::property
{

class AFU420DeviceBackend;

class AFU420PropertyIntegerImpl : public IPropertyInteger
{
public:
    AFU420PropertyIntegerImpl(const std::string& name,
                              tcam_value_int i,
                              tcam::afu420::AFU420Property id,
                              std::shared_ptr<tcam::property::AFU420DeviceBackend> backend);

    virtual std::string get_name() const final
    {
        return m_name;
    };
    virtual PropertyFlags get_flags() const final
    {
        return m_flags;
    };
    virtual void set_flags(PropertyFlags flags) final
    {
        m_flags = flags;
    };
    virtual int64_t get_min() const final
    {
        return m_min;
    };
    virtual int64_t get_max() const final
    {
        return m_max;
    };
    virtual int64_t get_step() const final
    {
        return m_step;
    };
    virtual int64_t get_default() const final
    {
        return m_default;
    };

    virtual outcome::result<int64_t> get_value() const final;

    virtual outcome::result<void> set_value(int64_t new_value) final;

private:
    outcome::result<void> valid_value(int64_t val);

    std::weak_ptr<tcam::property::AFU420DeviceBackend> m_cam;

    std::string m_name;
    tcam::property::PropertyFlags m_flags;
    int64_t m_min;
    int64_t m_max;
    int64_t m_step;
    int64_t m_default;

    tcam::afu420::AFU420Property m_id;
};


class AFU420PropertyDoubleImpl : public IPropertyFloat
{
public:
    AFU420PropertyDoubleImpl(const std::string& name,
                             tcam_value_double d,
                             tcam::afu420::AFU420Property id,
                             std::shared_ptr<tcam::property::AFU420DeviceBackend> backend);

    virtual std::string get_name() const final
    {
        return m_name;
    };
    virtual PropertyFlags get_flags() const final
    {
        return m_flags;
    };
    virtual void set_flags(PropertyFlags flags) final
    {
        m_flags = flags;
    };
    virtual double get_min() const final
    {
        return m_min;
    };
    virtual double get_max() const final
    {
        return m_max;
    };
    virtual double get_step() const final
    {
        return m_step;
    };
    virtual double get_default() const final
    {
        return m_default;
    };

    virtual outcome::result<double> get_value() const final;

    virtual outcome::result<void> set_value(double new_value) final;

private:
    outcome::result<void> valid_value(double val);

    std::weak_ptr<tcam::property::AFU420DeviceBackend> m_cam;

    std::string m_name;
    tcam::property::PropertyFlags m_flags;
    double m_min;
    double m_max;
    double m_step;
    double m_default;

    tcam::afu420::AFU420Property m_id;
};

class AFU420PropertyBoolImpl : public IPropertyBool
{
public:
    AFU420PropertyBoolImpl(const std::string& name,
                           bool default_value,
                           tcam::afu420::AFU420Property id,
                           std::shared_ptr<tcam::property::AFU420DeviceBackend> backend);

    virtual std::string get_name() const final
    {
        return m_name;
    };
    virtual PropertyFlags get_flags() const final
    {
        return m_flags;
    };
    virtual void set_flags(PropertyFlags flags) final
    {
        m_flags = flags;
    };

    virtual bool get_default() const final
    {
        return m_default;
    };

    virtual outcome::result<bool> get_value() const final;

    virtual outcome::result<void> set_value(bool new_value) final;

private:
    std::string m_name;
    std::weak_ptr<tcam::property::AFU420DeviceBackend> m_cam;

    tcam::property::PropertyFlags m_flags;
    bool m_default;
    // needed only for write only properties
    bool m_value;
    tcam::afu420::AFU420Property m_id;
};


class AFU420PropertyEnumImpl : public IPropertyEnum
{
public:
    AFU420PropertyEnumImpl(const std::string& name,
                           tcam::afu420::AFU420Property id,
                           std::map<int, std::string> m_entries,
                           std::shared_ptr<AFU420DeviceBackend> backend);

    virtual std::string get_name() const final
    {
        return m_name;
    };
    virtual PropertyFlags get_flags() const final
    {
        return m_flags;
    };
    virtual void set_flags(PropertyFlags flags) final
    {
        m_flags = flags;
    };

    virtual outcome::result<void> set_value_str(const std::string& new_value) final;
    virtual outcome::result<void> set_value(int64_t new_value) final;

    virtual outcome::result<std::string> get_value() const final;
    virtual outcome::result<int64_t> get_value_int() const final;

    virtual std::string get_default() const final
    {
        return m_default;
    };

    virtual std::vector<std::string> get_entries() const final;

private:
    bool valid_value(int value);

    std::map<int, std::string> m_entries;
    std::weak_ptr<AFU420DeviceBackend> m_cam;

    std::string m_name;
    PropertyFlags m_flags;

    std::string m_default;

    tcam::afu420::AFU420Property m_id;
};


} // namespace tcam::property

VISIBILITY_POP
