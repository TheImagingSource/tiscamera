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

#include "../property_dependencies.h"
#include "../PropertyInterfaces.h"
#include "../error.h"
#include "tcamprop1.0_base/tcamprop_base.h"
#include "virtcam_device.h"
#include <memory>
#include <vector>

using namespace tcam::property;

namespace tcam::virtcam
{

enum class VirtcamProperty
{
    TriggerMode,
    TriggerSoftware,
};


class VirtcamPropertyBackend
{
public:
    explicit VirtcamPropertyBackend(VirtcamDevice*);

    outcome::result<void> set_bool(tcam::virtcam::VirtcamProperty, bool new_value);
    outcome::result<bool> get_bool(tcam::virtcam::VirtcamProperty);

    outcome::result<void> execute(tcam::virtcam::VirtcamProperty);
private:

    VirtcamDevice* device_;
};

class VirtcamPropertyLockImpl : public PropertyLock
{

protected:

    VirtcamPropertyLockImpl(std::string_view name);

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



class prop_impl_trigger_mode : public IPropertyEnum, public VirtcamPropertyLockImpl
{
public:
    explicit prop_impl_trigger_mode(std::shared_ptr<VirtcamPropertyBackend> back)
        : VirtcamPropertyLockImpl("TriggerMode"), backend_(back)
    {}

    tcamprop1::prop_static_info get_static_info() const;

    PropertyFlags get_flags() const final
    {
        return PropertyFlags::Implemented | PropertyFlags::Available;
    }
    outcome::result<void> set_value(std::string_view new_value);

    outcome::result<std::string_view> get_value() const final
    {
        return backend_->get_bool(tcam::virtcam::VirtcamProperty::TriggerMode).value() ? "On" : "Off";
    }

    outcome::result<std::string_view> get_default() const final
    {
        return "On";
    }

    std::vector<std::string> get_entries() const final
    {
        return { "Off", "On" };
    }

    bool should_set_dependent_locked() const final
    {
        return !backend_->get_bool(tcam::virtcam::VirtcamProperty::TriggerMode).value();
    }

    void set_locked(bool /*new_locked_state*/) final
    {
        // we don't really need to do anything here
    }
private:
    std::shared_ptr<VirtcamPropertyBackend> backend_;
};



class prop_impl_trigger_software : public IPropertyCommand, public VirtcamPropertyLockImpl
{
public:
    explicit prop_impl_trigger_software(std::shared_ptr<VirtcamPropertyBackend> back)
        : VirtcamPropertyLockImpl("TriggerSoftware"), backend_(back)
    {}

    tcamprop1::prop_static_info get_static_info() const;

    PropertyFlags get_flags() const final
    {
        return flags_;
    }
    outcome::result<void> execute() final;


    void set_locked(bool new_locked_state) final;

private:
    std::shared_ptr<VirtcamPropertyBackend> backend_;

    tcam::property::PropertyFlags flags_ = PropertyFlags::Implemented | PropertyFlags::Available;
};

} // namespace tcam::virtcam
