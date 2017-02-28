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

#include "UploadItem.h"

#include <string>
#include <memory>

namespace lib33u
{
namespace firmware_update
{
	struct IDevicePort;

	struct UploadGroup
	{
		std::string name;
		std::shared_ptr<IDevicePort> port;
		std::vector<UploadItem> items;
		std::string version;
		uint64_t version_check_register;
		bool defines_device_type;
	};
}
}
