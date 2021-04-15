/*
 * Copyright 2021 The Imaging Source Europe GmbH
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

#pragma once

#include "PropertyInterfaces.h"

#include <linux/videodev2.h>
#include <map>
#include <memory>
#include <string>


namespace tcam::v4l2
{
struct v4l2_genicam_mapping;
}

namespace tcam::property
{


class V4L2PropertyBackend;

class V4L2PropertyIntegerImpl : public IPropertyInteger
{
public:
    V4L2PropertyIntegerImpl(const std::string& name, int id);
    V4L2PropertyIntegerImpl(struct v4l2_queryctrl* queryctrl,
                            struct v4l2_ext_control* ctrl,
                            std::shared_ptr<V4L2PropertyBackend> backend);

    virtual std::string get_name() const final
    {
        return p_name;
    };
    virtual PropertyFlags get_flags() const final
    {
        return p_flags;
    };

    virtual int64_t get_min() const final
    {
        return p_min;
    };
    virtual int64_t get_max() const final
    {
        return p_max;
    };
    virtual int64_t get_step() const final
    {
        return p_step;
    };
    virtual int64_t get_default() const final
    {
        return p_default;
    };
    virtual int64_t get_value() const final;

    virtual bool set_value(int64_t new_value) final;

private:
    bool valid_value(int64_t val);

    std::weak_ptr<V4L2PropertyBackend> p_cam;

    std::string p_name;
    PropertyFlags p_flags;

    int64_t p_min;
    int64_t p_max;
    int64_t p_step;
    int64_t p_default;

    int p_v4l2_id;
};


class V4L2PropertyDoubleImpl : public IPropertyFloat
{
public:
    V4L2PropertyDoubleImpl(const std::string& name, int id);
    V4L2PropertyDoubleImpl(struct v4l2_queryctrl* queryctrl,
                           struct v4l2_ext_control* ctrl,
                           std::shared_ptr<V4L2PropertyBackend> backend);

    virtual std::string get_name() const final
    {
        return p_name;
    };
    virtual PropertyFlags get_flags() const final
    {
        return p_flags;
    };

    virtual double get_min() const final
    {
        return p_min;
    };
    virtual double get_max() const final
    {
        return p_max;
    };
    virtual double get_step() const final
    {
        return p_step;
    };
    virtual double get_default() const final
    {
        return p_default;
    };
    virtual double get_value() const final;

    virtual bool set_value(double new_value) final;

private:
    bool valid_value(double val);

    std::weak_ptr<V4L2PropertyBackend> p_cam;

    std::string p_name;
    PropertyFlags p_flags;

    double p_min;
    double p_max;
    double p_step;
    double p_default;

    int p_v4l2_id;
};


class V4L2PropertyBoolImpl : public IPropertyBool
{
public:
    V4L2PropertyBoolImpl(struct v4l2_queryctrl* queryctrl,
                         struct v4l2_ext_control* ctrl,
                         std::shared_ptr<V4L2PropertyBackend> backend);

    virtual std::string get_name() const final
    {
        return p_name;
    };
    virtual PropertyFlags get_flags() const final
    {
        return p_flags;
    };

    virtual bool get_default() const final;
    virtual bool get_value() const final;

    virtual bool set_value(bool new_value) final;

private:
    std::weak_ptr<V4L2PropertyBackend> p_cam;

    std::string p_name;
    PropertyFlags p_flags;

    bool p_default;

    int p_v4l2_id;
};


class V4L2PropertyCommandImpl : public IPropertyCommand
{
public:
    V4L2PropertyCommandImpl(struct v4l2_queryctrl* queryctrl,
                            struct v4l2_ext_control* ctrl,
                            std::shared_ptr<V4L2PropertyBackend> backend);

    virtual std::string get_name() const final
    {
        return p_name;
    };
    virtual PropertyFlags get_flags() const final
    {
        return p_flags;
    };

    virtual bool execute() final;

private:
    std::weak_ptr<V4L2PropertyBackend> p_cam;

    std::string p_name;
    PropertyFlags p_flags;

    int p_v4l2_id;
};


class V4L2PropertyEnumImpl : public IPropertyEnum
{
public:
    V4L2PropertyEnumImpl(struct v4l2_queryctrl* queryctrl,
                         struct v4l2_ext_control* ctrl,
                         std::shared_ptr<V4L2PropertyBackend> backend,
                         const tcam::v4l2::v4l2_genicam_mapping* mapping = nullptr);

    virtual std::string get_name() const final
    {
        return p_name;
    };
    virtual PropertyFlags get_flags() const final
    {
        return p_flags;
    };

    virtual bool set_value_str(const std::string& new_value);
    virtual bool set_value(int new_value);

    virtual std::string get_value_str() const;
    virtual int get_value() const;

    virtual std::vector<std::string> get_entries() const final;

private:

    bool valid_value(int value);

    std::map<int, std::string> p_entries;
    std::weak_ptr<V4L2PropertyBackend> p_cam;

    std::string p_name;
    PropertyFlags p_flags;

    int p_v4l2_id;
};


} // namespace tcam::property
