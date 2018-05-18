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

#include "DevicePortI2CMachXO3.h"
#include "ReportProgress.h"

#include "MachXO3.h"
#include "JedecFile.h"

#include <set>
#include <algorithm>


namespace lib33u
{
namespace firmware_update
{
	DevicePortI2CMachXO3::DevicePortI2CMachXO3( const std::string& name, const pugi::xml_node& /*port_config*/ )
		: name_ { name }
	{
	}

	std::string DevicePortI2CMachXO3::name()
	{
		return name_;
	}

	namespace
	{
		I2C::WriteAction forwardI2CWrite (Camera& dev)
		{
			return [&](uint8_t i2cDev, const I2C::DataArray& command, bool combine_with_read)
			{
				dev.i2c_write( i2cDev, command, combine_with_read );				
			};
		}


		I2C::ReadAction forwardI2CRead (Camera& dev)
		{
			return [&](uint8_t i2cDev, uint16_t request_length, bool combine_with_write) -> I2C::DataArray
			{
				return dev.i2c_read( i2cDev, request_length, combine_with_write );
			};
		}		
	}

	void DevicePortI2CMachXO3::upload( Camera& dev, const std::vector<UploadItem>& items, util::progress::IReportProgress& progress )
	{
        auto&& item = items.front();

        auto jedec = MachXO3::JedecFile::Parse( item.data );

        I2C::I2CDevice i2c( 0x80, forwardI2CWrite(dev), forwardI2CRead(dev), 1024 );
		MachXO3::MachXO3Device machXO3( i2c );

		auto progress_func = [&progress]( const char* /* msg */, int pct )
		{
			progress.report_percentage( pct );
		};

		machXO3.UpdateConfiguration( jedec, progress_func, false );
	}

} /* namespace firmware_update */
} /* namespace lib33u */
