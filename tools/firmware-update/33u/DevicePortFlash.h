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

#include "IDevicePort.h"

#include "lib/PugiXml/pugixml.hpp"

#include <string>

namespace lib33u
{
namespace firmware_update
{
	class DevicPortFlash : public IDevicePort
	{
	public:
		DevicPortFlash( const std::string& name, const pugi::xml_node& port_config );

	public:
		virtual std::string name() override;
		virtual void upload( Camera & dev, const std::vector<UploadItem>& items, util::progress::IReportProgress& progress ) override;

	private:
		std::string name_;

	private:
		void upload_internal( Camera& dev, const std::vector<UploadItem>& items, util::progress::IReportProgress& progress );
	};

} /* namespace device_interface */
} /* namespace lib33u */
