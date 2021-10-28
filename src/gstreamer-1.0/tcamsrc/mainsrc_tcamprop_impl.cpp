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

#include "mainsrc_tcamprop_impl.h"

#include "../../PropertyCategory.h"
#include "mainsrc_device_state.h"
#include "gsttcammainsrc.h"

using namespace tcam::property;

using namespace tcam;

// static TcamError error_code_from_outcome(const std::error_code& res)
// {
//     if (res.category() == tcam::error_category())
//     {
//         switch (static_cast<tcam::status>(res.value()))
//         {
//             case status::DeviceCouldNotBeOpened:
//             case status::DeviceDoesNotExist:
//             case status::DeviceBlocked:
//             case status::DeviceLost:
//             {
//                 return TCAM_ERROR_DEVICE_LOST;
//             }
//             case status::PropertyDoesNotExist:
//             case status::PropertyOutOfBounds:
//             case status::PropertyValueDoesNotExist:
//             case status::PropertyIsLocked:
//             {
//                 return TCAM_ERROR_PROPERTY_NOT_SETTABLE;
//             }
//             case status::FormatInvalid:
//             case status::ResourceNotLockable:
//             case status::Timeout:
//             case status::NotImplemented:
//             case status::NotSupported:
//             case status::UndefinedError:
//             default:
//             {
//                 return TCAM_ERROR_UNKNOWN;
//             }
//         }
//     }
//     return TCAM_ERROR_UNKNOWN;
// }

// // fmt is not a string literal, waranting this pragma
// #pragma GCC diagnostic ignored "-Wformat-nonliteral"
// inline void fill_gerror_from_outcome(GError** gerr,
//                                      const std::string& fmt,
//                                      const std::error_code& err)
// {
//     if (gerr)
//     {
//         //*gerr =  g_error_new(TCAM_ERROR, error_code_from_outcome(err), fmt.c_str(), err.message());
//     }
// }
// #pragma GCC diagnostic warning "-Wformat-nonliteral"


static device_state& get_device_reference (GstTcamMainSrc* iface)
{
    GstTcamMainSrc* self = GST_TCAM_MAINSRC(iface);

    assert( self != nullptr );
	assert( self->device != nullptr );

    return *self->device;
}


static auto tcammainsrc_get_provider_impl_from_interface( TcamPropertyProvider* self ) -> tcamprop1_gobj::tcam_property_provider*
{
    auto& device = get_device_reference( GST_TCAM_MAINSRC( self ) );
    return &device.tcamprop_container_;
}

void tcam::mainsrc::gst_tcam_mainsrc_tcamprop_init(TcamPropertyProviderInterface* iface)
{
    tcamprop1_gobj::init_provider_interface<&tcammainsrc_get_provider_impl_from_interface>( iface );
}
