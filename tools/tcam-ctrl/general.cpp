/*
 * Copyright 2020 The Imaging Source Europe GmbH
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

#include "general.h"

#include "../../src/gobject/tcamprop.h"

bool is_valid_device_serial(GstElement* source, const std::string& serial)
{
    GSList* serials = tcam_prop_get_device_serials(TCAM_PROP(source));

    for (GSList* elem = serials; elem; elem = elem->next)
    {
        const char* device_serial = (gchar*)elem->data;

        if (serial.compare(device_serial) == 0)
        {
            g_slist_free_full(serials, g_free);
            return true;
        }
    }

    g_slist_free_full(serials, g_free);

    GSList* serials_backend = tcam_prop_get_device_serials_backend(TCAM_PROP(source));

    for (GSList* elem = serials_backend; elem; elem = elem->next)
    {
        const char* device_serial = (gchar*)elem->data;

        if (serial.compare(device_serial) == 0)
        {
            g_slist_free_full(serials_backend, g_free);
            return true;
        }
    }

    g_slist_free_full(serials_backend, g_free);

    return false;
}
