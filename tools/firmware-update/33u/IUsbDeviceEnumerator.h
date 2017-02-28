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

#pragma once

#include "IUsbDevice.h"

#include <vector>
#include <memory>

namespace lib33u
{
namespace driver_interface
{
	struct IUsbDeviceEnumerator
	{
		virtual ~IUsbDeviceEnumerator ()
		{}

		virtual std::vector<std::shared_ptr<IUsbDevice>> enum_all () = 0;
	};
}
}
