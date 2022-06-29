/*
 * Copyright 2022 The Imaging Source Europe GmbH
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

#include "virtcam_properties_impl.h"

#include "../logging.h"
#include "../property_dependencies.h"
#include "tcamprop1.0_base/tcamprop_property_info_list.h"
#include "virtcam_device.h"


//
// Backend
//

tcam::virtcam::VirtcamPropertyBackend::VirtcamPropertyBackend(tcam::virtcam::VirtcamDevice* dev)
    : device_(dev)
{}


outcome::result<void> tcam::virtcam::VirtcamPropertyBackend::set_bool(
    tcam::virtcam::VirtcamProperty id,
    bool new_value)
{
    switch (id)
    {
        case tcam::virtcam::VirtcamProperty::TriggerMode:
        {
            device_->trigger_mode_ = new_value;
            return outcome::success();
        }
        case tcam::virtcam::VirtcamProperty::TriggerSoftware:
        {
            return tcam::status::PropertyNotImplemented;
        }
    }
    return tcam::status::PropertyNotImplemented;
}

outcome::result<bool> tcam::virtcam::VirtcamPropertyBackend::get_bool(
    tcam::virtcam::VirtcamProperty id)
{
    switch (id)
    {
        case tcam::virtcam::VirtcamProperty::TriggerMode:
        {
            return device_->trigger_mode_;
        }
        case tcam::virtcam::VirtcamProperty::TriggerSoftware:
        {
            return tcam::status::PropertyNotImplemented;
        }
    }
    return tcam::status::PropertyNotImplemented;
}


outcome::result<void> tcam::virtcam::VirtcamPropertyBackend::execute(tcam::virtcam::VirtcamProperty id)
{
    switch (id)
    {
        case tcam::virtcam::VirtcamProperty::TriggerMode:
        {
            return tcam::status::InvalidParameter;
        }
        case tcam::virtcam::VirtcamProperty::TriggerSoftware:
        {
            device_->trigger_next_image_ = true;
            return outcome::success();
        }
    }
    return tcam::status::PropertyNotImplemented;
}

//
// LockImpl
//


tcam::virtcam::VirtcamPropertyLockImpl::VirtcamPropertyLockImpl(std::string_view name)
{
    dependency_info_ = tcam::property::find_dependency_entry(name);
}

std::vector<std::string_view> tcam::virtcam::VirtcamPropertyLockImpl::get_dependent_names() const
{
    if (dependency_info_)
    {
        return dependency_info_->dependent_property_names;
    }
    return {};
}

void tcam::virtcam::VirtcamPropertyLockImpl::update_dependent_lock_state()
{
    if (dependent_controls_.empty())
        return;

    bool new_locked_state = should_set_dependent_locked();
    for (auto& dep : dependent_controls_)
    {
        if (auto d = dep.lock())
        {
            d->set_locked(new_locked_state);
        }
    }
}

void tcam::virtcam::VirtcamPropertyLockImpl::set_dependent_properties(
    std::vector<std::weak_ptr<PropertyLock>>&& controls)
{
    dependent_controls_ = std::move(controls);

    update_dependent_lock_state();
}


//
// TriggerMode
//

tcamprop1::prop_static_info tcam::virtcam::prop_impl_trigger_mode::get_static_info() const
{
    return tcamprop1::prop_list::TriggerMode;
}


outcome::result<void> tcam::virtcam::prop_impl_trigger_mode::set_value(std::string_view new_value)
{
    if (new_value == "Off")
    {
        auto ret = backend_->set_bool(tcam::virtcam::VirtcamProperty::TriggerMode, false);

        if (!ret)
        {
            return ret.as_failure();
        }

        update_dependent_lock_state();

        return outcome::success();
    }
    else if (new_value == "On")
    {
        auto ret = backend_->set_bool(tcam::virtcam::VirtcamProperty::TriggerMode, true);

        if (!ret)
        {
            return ret.as_failure();
        }
        update_dependent_lock_state();

        return outcome::success();
    }
    return tcam::status::PropertyValueOutOfBounds;
}

//
// TriggerSoftware
//

tcamprop1::prop_static_info tcam::virtcam::prop_impl_trigger_software::get_static_info() const
{
    return tcamprop1::prop_list::TriggerSoftware;
}


outcome::result<void> tcam::virtcam::prop_impl_trigger_software::execute()
{
    return backend_->execute(tcam::virtcam::VirtcamProperty::TriggerSoftware);
}

void tcam::virtcam::prop_impl_trigger_software::set_locked(bool new_locked_state)
{
    lock(flags_, new_locked_state);
}
