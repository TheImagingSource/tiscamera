/*
 * Copyright 2014 The Imaging Source Europe GmbH
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


#include "UsbSession.h"
#include <stdexcept>

namespace tcam
{

UsbSession::UsbSession ()
    : session(nullptr)
{
    int ret = libusb_init(&this->session);
    if (ret < 0)
    {
        throw std::runtime_error("Unable to initialize libusb. Ret value: " + std::to_string(ret));
    }
    //libusb_set_debug(this->session, 3);
}


UsbSession::~UsbSession ()
{
    libusb_exit(this->session);
}

libusb_context* UsbSession::get_session ()
{
    return this->session;
}

} /* namespace tis */
