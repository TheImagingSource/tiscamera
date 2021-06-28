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


#define TCAM_TYPE_DEVICE_PROVIDER tcam_device_provider_get_type()
#define TCAM_DEVICE_PROVIDER(obj) \
    (G_TYPE_CHECK_INSTANCE_CAST((obj), TCAM_TYPE_DEVICE_PROVIDER, TcamDeviceProvider))

typedef struct _TcamDeviceProvider TcamDeviceProvider;
typedef struct _TcamDeviceProviderClass TcamDeviceProviderClass;

struct _TcamDeviceProviderClass
{
    GstDeviceProviderClass parent_class;
};

struct provider_state;

/**
 * Our device provider instance.
 *
 * @factory: the videotestsrc factory
 * @patterns: When started, the list of videotestsrc pattern
 *            (as strings) to iterate through when adding new devices,
 *            eg "smpte", "snow", ...
 * @timeout_id: When started, we will add a new device every
 *            %NEW_DEVICE_INTERVAL seconds
 */
struct _TcamDeviceProvider
{
    GstDeviceProvider parent;
    GstElementFactory* factory;

    //GList* devices;
    struct provider_state* state;
    //guint timeout_id;
};


GType tcam_device_provider_get_type(void);
