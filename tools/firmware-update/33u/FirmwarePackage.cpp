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

#include "FirmwarePackage.h"
#include "IDevicePort.h"
#include "DevicePortFlash.h"
#include "ReportProgress.h"

#include "Archive.h"
#include "lib/PugiXml/pugixml.hpp"

#include <stdexcept>
#include <cctype>
#include <cstdlib>

namespace lib33u
{
namespace firmware_update
{
	int FirmwarePackage::version() const
	{
		return version_;
	}

	std::vector<DeviceTypeDesc> FirmwarePackage::device_types() const
	{
		std::vector<DeviceTypeDesc> result;

		for( auto dt : device_types_ )
		{
			result.push_back( dt.second.desc );
		}

		return result;
	}

	namespace
	{
		std::map<std::string, std::shared_ptr<IDevicePort>> parse_device_ports( pugi::xpath_node_set port_nodes )
		{
			std::map<std::string, std::shared_ptr<IDevicePort>> result;

			for( auto port_node : port_nodes )
			{
				std::string port_type = port_node.node().attribute( "Type" ).as_string();
				std::string port_name = port_node.node().attribute( "Name" ).as_string();
				auto port_config = port_node.node().select_node( "PortConfiguration" );

				if( port_type == "Flash" )
				{
					result.emplace( port_name, std::make_shared<DevicPortFlash>( port_name, port_config.node() ) );
				}
				else
				{
					throw std::runtime_error( "Unexpected port_type" );
				}
			}

			return result;
		}

		std::vector<UploadItem> parse_upload_items( pugi::xpath_node_set upload_item_nodes, util::sz::Archive& ar )
		{
			std::vector<UploadItem> result;

			for( auto upload_item_node : upload_item_nodes )
			{
				auto file_attr = upload_item_node.node().attribute( "File" );
				auto string_attr = upload_item_node.node().attribute( "String" );
				auto u32_attr = upload_item_node.node().attribute( "U32" );
				auto offset_attr = upload_item_node.node().attribute( "Offset" );
				auto length_attr = upload_item_node.node().attribute( "Length" );

				UploadItem ui = {};

				if( !file_attr.empty() )
				{
					auto file = ar.read( file_attr.as_string() );
					ui.data.assign( file.data(), file.data() + file.size() );
				}
				else if( !string_attr.empty() )
				{
					std::string s = string_attr.as_string();
					ui.data.assign( s.begin(), s.end() );
				}
				else if( !u32_attr.empty() )
				{
					uint32_t u32 = u32_attr.as_uint();
					uint8_t* pu32 = reinterpret_cast<uint8_t*>(&u32);
					ui.data.assign( pu32, pu32 + sizeof( u32 ) );
				}
				else
				{
					throw std::runtime_error( "Did not find a valid data element in <Upload/> item" );
				}

				if( !offset_attr.empty() )
				{
					ui.offset = offset_attr.as_uint();
				}

				if( !length_attr.empty() )
				{
					uint32_t len = length_attr.as_uint();
					if( ui.data.size() > len )
					{
						throw std::runtime_error( "Upload data item is larger than the specified upload length" );
					}

					ui.data.resize( len );
				}

				while( ui.data.size() % 4 )
					ui.data.push_back( 0 );

				result.push_back( std::move(ui) );
			}

			return result;
		}

		std::vector<UploadGroup> parse_upload_groups( pugi::xpath_node_set upload_group_nodes, util::sz::Archive& ar, const std::map<std::string, std::shared_ptr<IDevicePort>>& port_lookup )
		{
			std::vector<UploadGroup> result;

			for( auto upload_group_node : upload_group_nodes )
			{
				auto group_name = upload_group_node.node().attribute( "Name" ).as_string();
				std::string version = upload_group_node.node().attribute( "Version" ).as_string();
				if( version.empty() )
				{
					throw std::runtime_error( "Upload group does not specify version" );
				}

				auto port_name = upload_group_node.node().attribute( "Destination" ).as_string();
				auto it = port_lookup.find( port_name );
				if( it == port_lookup.end() )
				{
					throw std::runtime_error( "DevicePort not found" );
				}
				auto port = it->second;

				auto upload_item_nodes = upload_group_node.node().select_nodes( "Upload" );
				auto upload_items = parse_upload_items( upload_item_nodes, ar );

				uint64_t version_check_register = upload_group_node.node().attribute( "VersionCheckRegister" ).as_ullong();
				bool defines_device_type = upload_group_node.node().attribute( "DefinesDeviceType" ).as_bool();

				result.emplace_back( UploadGroup { std::string( group_name ), port, upload_items, version, version_check_register, defines_device_type } );
			}

			return result;
		}

		std::map<uint16_t, DeviceType> parse_device_types( pugi::xpath_node_set device_type_nodes, util::sz::Archive& ar, const std::map<std::string, std::shared_ptr<IDevicePort>>& port_lookup )
		{
			std::map<uint16_t, DeviceType> result;

			for( auto device_type_node : device_type_nodes )
			{
				auto desc = device_type_node.node().attribute( "Description" ).as_string();
				auto pid = device_type_node.node().attribute( "ProductID" ).as_uint();

				auto upload_group_nodes = device_type_node.node().select_nodes( "UploadGroup" );
				auto groups = parse_upload_groups( upload_group_nodes, ar, port_lookup );

				DeviceType dt = { { desc, static_cast<uint16_t>(pid) }, groups };

				result.emplace( pid, dt );
			}

			return result;
		}
	}

	FirmwarePackage::FirmwarePackage( const std::string& fn )
	{
		auto ar = util::sz::Archive::open( fn );
		auto manifest = ar.read( "manifest.xml" );

		pugi::xml_document doc;
		auto load_res = doc.load_buffer( manifest.data(), manifest.size() );
		if( load_res.status != pugi::status_ok )
		{
			throw std::runtime_error( "Failed to parse manifest.xml" );
		}

		auto ver_attr = doc.select_node( "//FirmwarePackage/@FirmwareVersion" ).attribute();
		if( ver_attr.empty() )
		{
			throw std::runtime_error( "manifest.xml missing version info" );
		}
		version_ = ver_attr.as_int();

		auto port_nodes = doc.select_nodes( "//FirmwarePackage/DevicePorts/DevicePort" );
		ports_ = parse_device_ports( port_nodes );

		auto device_type_nodes = doc.select_nodes( "//FirmwarePackage/DeviceTypes/DeviceType" );
		device_types_ = parse_device_types( device_type_nodes, ar, ports_ );
	}

	namespace
	{
		bool match_version_number( Camera& cam, uint32_t version_number, uint64_t version_check_register )
		{
			try
			{
				auto cam_version = cam.gencp().read_u32( version_check_register );

				return cam_version == version_number;
			}
			catch( std::exception& )
			{
				return false;
			}
		}

		bool match_version_string( Camera& cam, const std::string& version_string, uint64_t version_check_register )
		{
			try
			{
				auto cam_version = cam.gencp().read_string( version_check_register, static_cast<uint16_t>(version_string.size()) );

				return cam_version == version_string;
			}
			catch( std::exception& )
			{
				return false;
			}
		}

		bool check_group_update_required( Camera& cam, const UploadGroup& group )
		{
			if( std::isdigit( group.version[0] ) )
			{
				uint32_t number = std::atoi( group.version.c_str() );

				return !match_version_number( cam, number, group.version_check_register );
			}
			else
			{
				return !match_version_string( cam, group.version, group.version_check_register );
			}
		}
	}

	void FirmwarePackage::upload( Camera& cam, util::progress::IReportProgress& progress, DeviceTypeDesc overrideDeviceType )
	{
		auto type_pid = overrideDeviceType.product_id ? overrideDeviceType.product_id : cam.product_id();
		auto it = device_types_.find( type_pid );
		if( it == device_types_.end() )
		{
			throw std::runtime_error( "Device type not found in package" );
		}

		progress.report_group_format( "Upgrade to version %d", version() );
		progress.report_group_format( "DeviceType = %s [0x%04X]", it->second.desc.description.c_str(), it->second.desc.product_id );

		auto upload_groups = it->second.upload_groups;

		auto group_progress = util::progress::MapItemProgress( progress, upload_groups.size() );

		for( auto upload_group : upload_groups )
		{
			group_progress.report_group( upload_group.name );

			if( !check_group_update_required( cam, upload_group ) )
			{
				group_progress.report_step( "Skipping, up to date" );
				group_progress.report_item();
				continue;
			}

			upload_group.port->upload( cam, upload_group.items, group_progress );

			group_progress.report_item();
		}
	}

} /* namespace firmware_update */
} /* namespace lib33u */
