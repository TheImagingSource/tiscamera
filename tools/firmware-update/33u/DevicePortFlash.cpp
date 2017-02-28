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

#include "DevicePortFlash.h"
#include "ReportProgress.h"

#include <set>
#include <algorithm>


namespace lib33u
{
namespace firmware_update
{
	DevicPortFlash::DevicPortFlash( const std::string& name, const pugi::xml_node& /*port_config*/ )
		: name_ { name }
	{
	}

	std::string DevicPortFlash::name()
	{
		return name_;
	}

	void DevicPortFlash::upload( Camera & dev, const std::vector<UploadItem>& items, util::progress::IReportProgress& progress )
	{
		try
		{
			upload_internal( dev, items, progress );
		}
		catch( std::exception& )
		{
			// Some error occurred... Re-try once
			upload_internal( dev, items, progress );
		}
	}

	namespace
	{
		std::set<uint32_t> calc_erase_requests( const UploadItem& item )
		{
			std::set<uint32_t> result;

			uint32_t block_size = device_interface::Flash::FLASH_PAGE_SIZE;
			uint32_t item_size = static_cast<uint32_t>(item.data.size());

			uint32_t startBlockOffset = item.offset % block_size;

			result.insert( item.offset - startBlockOffset );

			uint32_t spaceToEndOfBlock = block_size - startBlockOffset;
			if( item.data.size() > spaceToEndOfBlock )
			{
				uint32_t remainingLength = item_size - spaceToEndOfBlock;
				uint32_t offset = item.offset + spaceToEndOfBlock;

				while( remainingLength > 0 )
				{
					result.insert( offset );

					uint32_t step = std::min( block_size, remainingLength );
					remainingLength -= step;
					offset += step;
				}
			}

			return result;
		}
	}

	void DevicPortFlash::upload_internal( Camera& cam, const std::vector<UploadItem>& items, util::progress::IReportProgress& progress )
	{
		std::set<uint32_t> erase_requests;

		for( auto& item : items )
		{
			auto req = calc_erase_requests( item );

			erase_requests.insert( req.begin(), req.end() );
		}

		auto erase_progress = util::progress::MapPercentage( progress, 0, 33 );
		auto erase_items_progress = util::progress::MapItemProgress( erase_progress, static_cast<int>(erase_requests.size()) );

		erase_items_progress.report_step( "Erase Flash" );

		for( auto address : erase_requests )
		{
			cam.flash().erase( address, 1 );

			erase_items_progress.report_items( 1, "pages/s", 1 );
		}

		auto items_progress = util::progress::MapPercentage( progress, 33, 100 );
		auto items_items_progress = util::progress::MapItemProgress( items_progress, static_cast<int>(items.size()) );

		for( auto item : items )
		{
			cam.flash().write_verify( item.offset, item.data.data(), static_cast<uint32_t>(item.data.size()), items_items_progress );

			items_items_progress.report_item();
		}
	}
} /* namespace firmware_update */
} /* namespace lib33u */
