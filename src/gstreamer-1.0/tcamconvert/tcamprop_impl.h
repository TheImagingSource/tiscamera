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

#include <tcamprop_system/tcamprop_provider_base.h>

#include <tcamprop.h>


namespace tcamconvert
{
void gst_tcamconvert_prop_init(TcamPropInterface* iface);
auto get_property_list_interface(TcamProp* iface) -> tcamprop_system::property_list_interface*;
} // namespace tcamconvert
