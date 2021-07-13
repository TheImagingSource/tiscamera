#pragma once

#include "../tcamprop_system/tcamprop_provider_base.h"

#include <tcamprop.h>


namespace tcamconvert
{
void gst_tcamconvert_prop_init(TcamPropInterface* iface);
auto get_property_list_interface(TcamProp* iface) -> tcamprop_system::property_list_interface*;
} // namespace tcamconvert
