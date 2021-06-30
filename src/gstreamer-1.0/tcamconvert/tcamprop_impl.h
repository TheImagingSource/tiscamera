#pragma once

#include "../../gobject/tcamprop.h"
#include "tcamprop_provider_base.h"


namespace tcamconvert
{
void gst_tcamconvert_prop_init(TcamPropInterface* iface);
auto get_property_list_interface(TcamProp* iface) -> tcamprop_system::property_list_interface*;
} // namespace tcamconvert
