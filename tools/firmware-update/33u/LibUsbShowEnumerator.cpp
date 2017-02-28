/*
 * Copyright 2017 The Imaging Source Europe GmbH
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

#include "LibUsbShowEnumerator.h"
#include "LibUsbShowDevice.h"
#include "UsbHandler.h"

#include <algorithm>
#include <iterator>

namespace lib33u
{
namespace driver_interface
{
namespace libusb
{
	std::vector<std::shared_ptr<IUsbDevice>> ShowDeviceEnumerator::enum_all ()
	{
        tis::UsbHandler usb;

        auto dev = usb.get_device_list();

		std::vector<std::shared_ptr<IUsbDevice>> devices;

		for (auto& d : dev)
        {
            auto new_dev = std::make_shared<ShowDevice>(usb.get_session(), d);

            devices.push_back(new_dev);
        }

		return devices;
	}
} /* namespace libusb */
} /* namespace driver_interface */
} /* namespace lib33u */
