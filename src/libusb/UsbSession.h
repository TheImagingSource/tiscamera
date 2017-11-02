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

#ifndef TCAM_USBSESSION_H
#define TCAM_USBSESSION_H

#include <memory>
#include <libusb-1.0/libusb.h>

namespace tcam
{

class UsbSession
{
private:
    libusb_context* session;

public:
    UsbSession ();
    ~UsbSession ();

    UsbSession (const UsbSession& _session) = delete;
    UsbSession& operator=(const UsbSession&) = delete;

    libusb_context* get_session ();

}; /* class UsbSession */

} /* namespace tcam */

#endif /* TCAM_USBSESSION_H */
