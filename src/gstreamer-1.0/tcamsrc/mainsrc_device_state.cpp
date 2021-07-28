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

#include "mainsrc_device_state.h"


static void separate_serial_and_type(GstTcamMainSrc* self, const std::string& input)
{
    auto pos = input.find("-");

    if (pos != std::string::npos)
    {
        // assign to tmp variables
        // input could be self->device_serial
        // overwriting it would ivalidate input for
        // device_type retrieval
        std::string tmp1 = input.substr(0, pos);
        std::string tmp2 = input.substr(pos + 1);

        self->device->device_serial = tmp1;
        self->device->device_type = tcam::tcam_device_from_string(tmp2);
    }
    else
    {
        self->device->device_serial = input;
    }
}


bool mainsrc_init_camera(GstTcamMainSrc* self)
{
    GST_DEBUG_OBJECT(self, "Initializing device.");

    self->device->dev = nullptr;

    self->device->all_caps.reset();

    separate_serial_and_type(self, self->device->device_serial);

    GST_DEBUG("Opening device. Serial: '%s Type: '%s'",
              self->device->device_serial.c_str(),
              tcam::tcam_device_type_to_string(self->device->device_type).c_str());

    self->device->dev = tcam::open_device(self->device->device_serial, self->device->device_type);
    if (!self->device->dev)
    {
        GST_ERROR("Unable to open device. No stream possible.");
        mainsrc_close_camera(self);
        return false;
    }
    self->device->device_serial = self->device->dev->get_device().get_serial();
    self->device->device_type = self->device->dev->get_device().get_device_type();

    return true;
}


void mainsrc_close_camera(GstTcamMainSrc* self)
{
    GST_INFO_OBJECT(self, "Closing device");

    std::lock_guard<std::mutex> lck(self->device->mtx);
    if (self->device->dev)
    {
        self->device->close();
    }
}
