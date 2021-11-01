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

#include "../../../libs/gst-helper/include/tcamprop1.0_base/tcamprop_property_interface.h"
#include "../../../libs/tcam-property/src/tcam-property-1.0.h"
#include "../../PropertyInterfaces.h"

#include <memory>

namespace tcam::mainsrc
{
auto make_wrapper_instance(const std::shared_ptr<tcam::property::IPropertyBase>& prop)
    -> std::unique_ptr<tcamprop1::property_interface>;
void gst_tcam_mainsrc_tcamprop_init(TcamPropertyProviderInterface* iface);

} // namespace tcam::mainsrc
