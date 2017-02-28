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

#include "Camera.h"
#include "MemoryMap.h"

namespace lib33u
{
Camera::Impl::Impl (std::shared_ptr<driver_interface::IUsbDevice> dev)
    : device_ { dev }
    , gencp_ { dev }
    , flash_ { gencp_ }
	{}

uint16_t Camera::Impl::product_id () const
{
    return device_->product_id();
}

std::string Camera::Impl::model_description () const
{
    return gencp_.read_string( device_interface::MemoryMap::CAMERA_INFO + 0x20, 0x40 );
}

std::string Camera::Impl::fpga_version () const
{
    return gencp_.read_string( device_interface::MemoryMap::CAMERA_INFO + 0x00, 0x20 );
}

std::string Camera::Impl::nios_firmware_version () const
{
    return gencp_.read_string( device_interface::MemoryMap::CAMERA_INFO + 0x60, 0x20 );
}

Camera::Camera (std::shared_ptr<driver_interface::IUsbDevice> dev)
    : impl_ { std::unique_ptr<Impl>( new Impl(dev) ) }
	{}

Camera Camera::attach (std::shared_ptr<driver_interface::IUsbDevice> dev)
{
    dev->open();
    return Camera( dev );
}

}
