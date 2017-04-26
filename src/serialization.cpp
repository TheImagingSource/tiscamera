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

#include "serialization.h"

#include "utils.h"
#include "internal.h"

#include <tinyxml.h>

#include <algorithm>

using namespace tcam;


bool tcam::export_device_list (const std::vector<DeviceInfo>& device_list,
                                      const std::string& filename)
{
    // all allocated tinyxml elements will automatically be cleaned up
    // changing the implementation to raii can cause segmentation faults

    TiXmlDocument doc;

    TiXmlDeclaration* decl = new TiXmlDeclaration ( "1.0", "", "" );
    doc.LinkEndChild( decl );

    TiXmlElement* devices_node = new TiXmlElement ("devices");
    doc.LinkEndChild( devices_node );

    for (const auto& open_device : device_list)
    {
        TiXmlElement* device_node = new TiXmlElement ("device");
        devices_node->LinkEndChild( device_node );

        TiXmlElement* device_name_node = new TiXmlElement( "device_name" );
        device_node->LinkEndChild( device_name_node );

        TiXmlText* name_text = new TiXmlText(open_device.get_name().c_str());
        device_name_node->LinkEndChild( name_text );

        TiXmlElement* device_id_node = new TiXmlElement( "serialnumber" );
        device_node->LinkEndChild( device_id_node );

        TiXmlText* id_text = new TiXmlText(open_device.get_serial().c_str());
        device_id_node->LinkEndChild( id_text );

        TiXmlElement* device_identifier_node = new TiXmlElement( "identifier" );
        device_node->LinkEndChild( device_identifier_node );

        TiXmlText* identifier_text = new TiXmlText(open_device.get_identifier().c_str());
        device_identifier_node->LinkEndChild( identifier_text );


        TiXmlElement* device_type_node = new TiXmlElement( "type" );
        device_node->LinkEndChild( device_type_node );

        TiXmlText* type_text = new TiXmlText(open_device.get_device_type_as_string().c_str());
        device_type_node->LinkEndChild( type_text );
    }

    doc.SaveFile( filename.c_str() );


    return true;
}


static bool load_single_property (TiXmlElement* prop_node,
                                  std::vector<std::shared_ptr<Property>> properties)
{
    std::string p_name;

    auto find_property = [&p_name] (std::shared_ptr<Property> p)
        {
            return p->get_name().compare(p_name) == 0;
        };



    for (TiXmlAttribute* attr = prop_node->FirstAttribute(); attr != nullptr; attr = attr->Next())
    {
        if (strcmp(attr->Name(), "name") == 0)
        {
            p_name = attr->Value();
        }
        else if (strcmp(attr->Name(), "type") == 0)
        {

        }
        else
        {
            return false;
        }
    }

    auto iter = std::find_if(properties.begin(), properties.end(), find_property);

    if (iter == properties.end())
    {
        return false;
    }

    for (TiXmlNode* pChild = prop_node->FirstChild(); pChild != nullptr; pChild = pChild->NextSibling())
    {
        switch (pChild->Type())
        {
            // this means we have a property
            case TiXmlNode::NodeType::TINYXML_ELEMENT:
            {
                break;
            }
            case TiXmlNode::NodeType::TINYXML_TEXT:
            {
                break;
            }
            default:
                break;
        }
    }


    return true;
}


static bool load_property_values (TiXmlNode* properties_node,
                                  std::vector<std::shared_ptr<Property>> properties)
{

    for (TiXmlNode* pChild = properties_node->FirstChild();
         pChild != nullptr;
         pChild = pChild->NextSibling())
    {
        switch (pChild->Type())
        {
            // this means we have a property
            case TiXmlNode::NodeType::TINYXML_ELEMENT:
            {
                bool ret = load_single_property(pChild->ToElement(), properties);
                if (!ret)
                {
                    return false;
                }
                break;
            }
            default:
                break;
        }
    }

    return true;
}


bool tcam::load_xml_description (const std::string& filename,
                                        const DeviceInfo& device,
                                        VideoFormat& format,
                                        std::vector<std::shared_ptr<Property>>& properties)
{

    TiXmlDocument doc( filename.c_str() );
    bool ret = doc.LoadFile();

    if (!ret)
    {
        return false;
    }

    // TODO finish implementation

    if (device.get_serial().compare("") == 0)
    {
        return false;
    }


    TiXmlNode* pChild = nullptr;

    load_property_values(pChild, properties);



    return false;
}


bool tcam::save_xml_description (const std::string& filename,
                                        const DeviceInfo& device,
                                        const VideoFormat& format,
                                        const std::vector<std::shared_ptr<Property>>& properties)
{

    // all allocated tinyxml elements will automatically be cleaned up
    // changing the implementation to raii can cause segmentation faults

    DeviceInfo open_device = device;

    TiXmlDocument doc;

    TiXmlDeclaration* decl = new TiXmlDeclaration ( "1.0", "", "" );
    doc.LinkEndChild( decl );

    TiXmlElement* device_node = new TiXmlElement ("device");
    doc.LinkEndChild( device_node );

    TiXmlComment* comment = new TiXmlComment();
    comment->SetValue(" device identification " );
    device_node->LinkEndChild( comment );

    TiXmlElement* device_id_node = new TiXmlElement( "serialnumber" );
    device_node->LinkEndChild( device_id_node );

    TiXmlText* id_text = new TiXmlText(open_device.get_serial().c_str());
    device_id_node->LinkEndChild( id_text );

    TiXmlComment* comment2 = new TiXmlComment();
    comment2->SetValue(" active format settings " );
    device_node->LinkEndChild( comment2 );

    // format section

    TiXmlElement* format_node = new TiXmlElement( "image_settings" );
    device_node->LinkEndChild( format_node );

    TiXmlElement* format_fourcc = new TiXmlElement ( "format" );
    format_node->LinkEndChild( format_fourcc );

    TiXmlText* fourcc_text = new TiXmlText( fourcc2description(format.get_fourcc()) );
    format_fourcc->LinkEndChild( fourcc_text );

    TiXmlElement* format_size  = new TiXmlElement( "resolution" );
    format_node->LinkEndChild( format_size );

    TiXmlElement* size_width = new TiXmlElement( "width" );
    format_size->LinkEndChild( size_width );

    TiXmlText* width_text = new TiXmlText ( std::to_string(format.get_size().width) );
    size_width->LinkEndChild( width_text );

    TiXmlElement* size_height = new TiXmlElement ( "height" );
    format_size->LinkEndChild( size_height );

    TiXmlText* height_text = new TiXmlText( std::to_string(format.get_size().height) );
    size_height->LinkEndChild( height_text );

    TiXmlElement* format_framerate = new TiXmlElement( "fps" );
    format_node->LinkEndChild( format_framerate );

    TiXmlText* framerate_text = new TiXmlText( std::to_string(format.get_framerate()) );
    format_framerate->LinkEndChild( framerate_text );

    // property section

    TiXmlElement* properties_node = new TiXmlElement( "properties" );
    device_node->LinkEndChild( properties_node );

    for (const auto& dp : properties)
    {
        TiXmlElement* property_node = new TiXmlElement ( "property" );
        properties_node->LinkEndChild( property_node );

        property_node->SetAttribute("name", dp->get_name().c_str());
        property_node->SetAttribute("type", propertyType2String(dp->get_type()).c_str());

        if (is_bit_set(dp->get_flags(), TCAM_PROPERTY_FLAG_EXTERNAL))
        {
            TiXmlElement* emulated_node = new TiXmlElement( "emulated" );
            property_node->LinkEndChild( emulated_node );

            TiXmlText* emulated_text = new TiXmlText( "true" );
            emulated_node->LinkEndChild( emulated_text );
        }

        TiXmlElement* value_node = new TiXmlElement( "value" );
        property_node->LinkEndChild( value_node );

        TiXmlText* text = new TiXmlText(dp->to_string() );
        value_node->LinkEndChild( text );
    }

    doc.SaveFile( filename.c_str() );

    return true;
}
