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

#include <gst/gst.h>

namespace tcammainsrc
{
struct provider_state;
}

G_BEGIN_DECLS

#define TCAM_TYPE_MAINSRC_DEVICE_PROVIDER tcam_mainsrc_device_provider_get_type()
#define TCAM_MAINSRC_DEVICE_PROVIDER(obj) \
    (G_TYPE_CHECK_INSTANCE_CAST(          \
        (obj), TCAM_TYPE_MAINSRC_DEVICE_PROVIDER, TcamMainSrcDeviceProvider))

typedef struct _TcamMainSrcDeviceProvider TcamMainSrcDeviceProvider;
typedef struct _TcamMainSrcDeviceProviderClass TcamMainSrcDeviceProviderClass;

struct _TcamMainSrcDeviceProviderClass
{
    GstDeviceProviderClass parent_class;
};

struct _TcamMainSrcDeviceProvider
{
    GstDeviceProvider parent;

    tcammainsrc::provider_state* state;
};


GType tcam_mainsrc_device_provider_get_type(void);

G_END_DECLS
