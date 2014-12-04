



#include "AravisDevice.h"

#include "aravis_utils.h"

#include "internal.h"
#include "utils.h"

#include <algorithm>
#include <cstring>


using namespace tcam;


AravisDevice::AravisPropertyHandler::AravisPropertyHandler (AravisDevice* dev)
    : device(dev)
{}


bool AravisDevice::AravisPropertyHandler::getProperty (Property& p)
{
    return device->getProperty(p);

    return false;
}


bool AravisDevice::AravisPropertyHandler::setProperty (const Property& p)
{
    return device->setProperty(p);
    return false;
}


AravisDevice::AravisDevice (const DeviceInfo& device_desc)
    : device(device_desc), handler(nullptr), current_buffer(0), stream(NULL)
{
    this->arv_camera = arv_camera_new (this->device.getInfo().identifier);

    if (this->arv_camera == NULL)
    {
        throw std::runtime_error("Error while creating ArvCamera");
    }

    arv_options.auto_socket_buffer = false;
    arv_options.no_packet_resend = true;
    arv_options.packet_timeout = 40;
    arv_options.frame_retention = 200;

    handler = std::make_shared<AravisPropertyHandler>(this);

    index_genicam();
    determine_active_video_format();
}


AravisDevice::~AravisDevice ()
{
    if (arv_camera != NULL)
    {
        g_object_unref (arv_camera);
        arv_camera = NULL;
    }
}


DeviceInfo AravisDevice::getDeviceDescription () const
{
    return device;
}


std::vector<std::shared_ptr<Property>> AravisDevice::getProperties ()
{
    std::vector<std::shared_ptr<Property>> vec;

    for (auto& p : handler->properties)
    {
        vec.push_back(p.prop);
    }
    tcam_log(TCAM_LOG_DEBUG, "Returning %d properties", vec.size());

    return vec;
}


bool AravisDevice::setProperty (const Property& p)
{
    auto f = [p] (const property_mapping& m)
        {
            return p.getName().compare(m.prop->getName()) == 0;
        };

    auto pm = std::find_if(handler->properties.begin(), handler->properties.end(), f);

    if (pm == handler->properties.end())
    {
        return false;
    }

    auto device = arv_camera_get_device(arv_camera);

    Property::VALUE_TYPE value_type = pm->prop->getValueType();

    TCAM_PROPERTY_TYPE type = pm->prop->getType();

    if (type == TCAM_PROPERTY_TYPE_INTEGER)
    {
        auto prop_impl = (PropertyInteger&) (*pm->prop);
    }
    else if (type == TCAM_PROPERTY_TYPE_DOUBLE)
    {

    }
    else
    {
        //return false;
    }

    switch (value_type)
    {
        case Property::INTEGER:
        {
            tcam_log(TCAM_LOG_ERROR, "Integer %s: %d", pm->arv_ident.c_str(), ((PropertyInteger&) p).getValue());
            // PropertyInteger&
            arv_device_set_integer_feature_value(device, pm->arv_ident.c_str(), ((PropertyInteger&) p).getValue());
            pm->prop->setStruct(p.getStruct());
            break;
        }
        case Property::INTSWISSKNIFE:
        {
            tcam_log(TCAM_LOG_ERROR, "Swissknife");
            arv_device_set_integer_feature_value(device, pm->arv_ident.c_str(), ((PropertyInteger&) (p)).getValue());
            break;
        }
        case Property::FLOAT:
        {
            tcam_log(TCAM_LOG_ERROR, "FLOAT");
            arv_device_set_float_feature_value(device, pm->arv_ident.c_str(), ((PropertyInteger&) (p)).getValue());
            pm->prop->setStruct(p.getStruct());

            break;
        }
        case Property::BUTTON:
        case Property::COMMAND:
        {
            //arv_device_
            arv_device_execute_command(device, pm->arv_ident.c_str());
            break;
        }
        case Property::BOOLEAN:
        {
            tcam_log(TCAM_LOG_DEBUG, "Bool %s", pm->arv_ident.c_str());
            if (((PropertyBoolean&)p).getValue())
                arv_device_set_integer_feature_value(device, pm->arv_ident.c_str(), 1);
            else
                arv_device_set_integer_feature_value(device, pm->arv_ident.c_str(), 0);

            pm->prop->setStruct(p.getStruct());
            break;
        }
        case Property::STRING:
        case Property::ENUM:
        {
            tcam_log(TCAM_LOG_DEBUG, "====ENUMERATION %s", pm->arv_ident.c_str());
            if (p.getType() == TCAM_PROPERTY_TYPE_BOOLEAN)
            {
                if (((PropertyBoolean&)p).getValue())
                    arv_device_set_integer_feature_value(device, pm->arv_ident.c_str(), 1);
                else
                    arv_device_set_integer_feature_value(device, pm->arv_ident.c_str(), 0);

                pm->prop->setStruct(p.getStruct());
                break;
            }

            //break;
        }
        case Property::UNDEFINED:
        default:
        {
            tcam_log(TCAM_LOG_ERROR, "NOT SUPPORTED!!!");
            break;
        }
    }
    return true;
}


bool AravisDevice::getProperty (Property&)
{
    return false;
}


bool AravisDevice::setVideoFormat (const VideoFormat& new_format)
{
    // bool valid = false;
    // for (const auto& v : available_videoformats)
    // {
    // if (v.isValidVideoFormat( _format))
    // {
    // valid = true;
    // break;
    // }
    // }

    // if (valid == false)
    // {
    // return false;
    // }

    tcam_log(TCAM_LOG_DEBUG, "Setting format to '%s'", new_format.toString().c_str());

    arv_camera_set_frame_rate (this->arv_camera, 3.75);

    arv_camera_set_pixel_format(this->arv_camera, fourcc2aravis(new_format.getFourcc()));

    // TODO: auto center

    arv_camera_set_region(this->arv_camera, 0, 0, new_format.getSize().width, new_format.getSize().height);

    determine_active_video_format();

    return true;
}


VideoFormat AravisDevice::getActiveVideoFormat () const
{
    return active_video_format;
}


void AravisDevice::determine_active_video_format ()
{

    this->active_video_format.setFramerate(arv_camera_get_frame_rate(this->arv_camera));
    active_video_format.setFourcc(aravis2fourcc(arv_camera_get_pixel_format(this->arv_camera)));

    int dx = 0;
    int dy = 0;

    arv_camera_get_binning(this->arv_camera, &dx, &dy);

    // TODO binning

    int x1, x2, y1, y2;
    arv_camera_get_region(this->arv_camera, &x1, &y1, &x2, &y2);

    unsigned int height = y2 - y1;
    unsigned int width = x2 - x1;

    active_video_format.setSize(width, height);

}


std::vector<VideoFormatDescription> AravisDevice::getAvailableVideoFormats ()
{
    if (this->available_videoformats.empty())
    {
        this->index_genicam();
    }

    return available_videoformats;
}


bool AravisDevice::setSink (std::shared_ptr<SinkInterface> s)
{
    this->external_sink = s;

    return true;
}


bool AravisDevice::initialize_buffers (std::vector<std::shared_ptr<MemoryBuffer>> b)
{
    this->buffers = b;

    return true;
}


bool AravisDevice::release_buffers ()
{
    buffers.clear();

    return true;
}


bool AravisDevice::start_stream ()
{
    if (arv_camera == NULL)
    {
        tcam_log(TCAM_LOG_ERROR, "ArvCamera missing!");
        return false;
    }
    if (external_sink == nullptr)
    {
        tcam_log(TCAM_LOG_ERROR, "No sink specified");
        return false;
    }

    this->stream = arv_camera_create_stream(this->arv_camera, NULL, NULL);

    if (this->stream == NULL)
    {
        tcam_log(TCAM_LOG_ERROR, "Unable to create ArvStream.");
        // TODO errno
        return false;
    }
    int payload = arv_camera_get_payload(this->arv_camera);

    if (ARV_IS_GV_STREAM (this->stream))
    {
        if (this->arv_options.auto_socket_buffer)
        {
            g_object_set (this->stream,
                          "socket-buffer", ARV_GV_STREAM_SOCKET_BUFFER_AUTO,
                          "socket-buffer-size", 0,
                          NULL);
        }
        if (this->arv_options.no_packet_resend)
        {
            g_object_set (this->stream,
                          "packet-resend", ARV_GV_STREAM_PACKET_RESEND_NEVER,
                          NULL);
        }
        g_object_set (this->stream,
                      "packet-timeout", (unsigned) this->arv_options.packet_timeout * 1000,
                      "frame-retention", (unsigned) this->arv_options.frame_retention * 1000,
                      NULL);
    }

    for (int i = 0; i < 50; ++i)
    {
        arv_stream_push_buffer(this->stream, arv_buffer_new(payload, NULL));
    }

    arv_stream_set_emit_signals (this->stream, TRUE);

    arv_camera_set_acquisition_mode(this->arv_camera, ARV_ACQUISITION_MODE_CONTINUOUS);

    // a work thread is not required as aravis already pushes the images asynchroniously

    g_signal_connect (stream, "new-buffer", G_CALLBACK (callback), this);

    tcam_log(TCAM_LOG_INFO, "Starting actual stream...");

    arv_camera_start_acquisition(this->arv_camera);

    return true;
}


bool AravisDevice::stop_stream ()
{
    if (arv_camera == NULL)
        return false;

    arv_camera_stop_acquisition(arv_camera);

    return true;
}


void AravisDevice::callback (ArvStream* stream, void* user_data)
{
    AravisDevice* self = static_cast<AravisDevice*>(user_data);
    if (self == NULL)
    {
        tcam_log(TCAM_LOG_ERROR, "Callback camera instance is NULL.");
        return;
    }
    if (self->stream == NULL)
    {
        return;
    }

    ArvBuffer* buffer = arv_stream_pop_buffer (self->stream);

    if (buffer != NULL)
    {
        ArvBufferStatus status = arv_buffer_get_status(buffer);

        //ArvBufferStatus status = buffer->;

        if (status == ARV_BUFFER_STATUS_SUCCESS)
        {
            struct tcam_image_buffer desc = {};

            desc.format = self->active_video_format.getStruct();

            size_t size = 0;
            desc.pData = ( unsigned char* ) arv_buffer_get_data ( buffer, &size );
            desc.length = size;
            desc.pitch = desc.format.width * img::get_bits_per_pixel(desc.format.fourcc) / 8;

            if (desc.pData == NULL)
            {
                tcam_log(TCAM_LOG_ERROR, "FUCKING HELL");
            }

            self->buffers.at(self->current_buffer)->setImageBuffer(desc);
            //tcam_log(TCAM_LOG_DEBUG, "Pushing new image buffer to sink.");
            self->external_sink->pushImage(self->buffers.at(self->current_buffer));

            if (self->current_buffer < self->buffers.size() -1)
            {
                self->current_buffer++;
            }
            else
            {
                self->current_buffer = 0;
            }
        }
        else
        {
            std::string msg;

            switch (status)
            {
                case ARV_BUFFER_STATUS_SUCCESS:
                    msg = "the buffer is cleared";
                    break;
                case ARV_BUFFER_STATUS_TIMEOUT:
                    msg = "Timeout has been reached before all packets were received";
                    break;
                case ARV_BUFFER_STATUS_MISSING_PACKETS:
                {
                    msg = "Stream has missing packets";

                    // struct tcam_image_buffer desc = {0};

                    // desc.format = self->active_video_format.getStruct();


                    // desc.pData = (unsigned char*)buffer->data;
                    // desc.length = buffer->size;

                    // if (buffer->data == NULL)
                    // {
                    // tcam_log(TCAM_LOG_ERROR, "FUCKING HELL");
                    // }

                    // self->buffers.at(self->current_buffer)->setImageBuffer(desc);
                    // tcam_log(TCAM_LOG_DEBUG, "Pushing new image buffer to sink.");
                    // self->external_sink->pushImage(self->buffers.at(self->current_buffer));

                    // if (self->current_buffer < self->buffers.size() -1)
                    // self->current_buffer++;
                    // else
                    // self->current_buffer = 0;

                    break;
                }
                case ARV_BUFFER_STATUS_WRONG_PACKET_ID:
                    msg = "Stream has packet with wrong id";
                    break;
                case ARV_BUFFER_STATUS_SIZE_MISMATCH:
                    msg = "The received image did not fit in the buffer data space";
                    break;
                case ARV_BUFFER_STATUS_FILLING:
                    msg = "The image is currently being filled";
                    break;
                case ARV_BUFFER_STATUS_ABORTED:
                    msg = "The filling was aborted before completion";
                    break;
                case ARV_BUFFER_STATUS_CLEARED:
                    msg = "Buffer cleared";
                    break;
                case ARV_BUFFER_STATUS_UNKNOWN:
                    msg = "This should not happen";
                    break;
            }
            tcam_log(TCAM_LOG_WARNING, msg.c_str());
        }
        //tcam_log(TCAM_LOG_DEBUG, "Returning buffer to aravis.");
        arv_stream_push_buffer(self->stream, buffer);
    }
    else
    {
        // switch (type)
        // {
        // case ARV_STREAM_CALLBACK_TYPE_INIT:
        // // g_log(NULL, G_LOG_LEVEL_DEBUG, "%s - Stream callback: thread initialization", source->mClassName.c_str());
        // break;
        // case ARV_STREAM_CALLBACK_TYPE_EXIT:
        // // g_log(NULL, G_LOG_LEVEL_DEBUG, "%s - Stream callback: thread end", source->mClassName.c_str());
        // break;
        //case ARV_STREAM_CALLBACK_TYPE_START_BUFFER:
        // g_log(NULL, G_LOG_LEVEL_DEBUG, "%s - Stream callback: buffer filling start", source->mClassName.c_str());
        // break;
        //case ARV_STREAM_CALLBACK_TYPE_BUFFER_DONE:
        // g_log(NULL, G_LOG_LEVEL_DEBUG, "%s - Stream callback: buffer filled", source->mClassName.c_str());
        // break;
        // }
    }

}


struct aravis_fourcc
{
    uint32_t fourcc;
    uint32_t aravis;
};


static std::vector<aravis_fourcc> arv_fourcc_conversion_table =
{
    { FOURCC_Y800, ARV_PIXEL_FORMAT_MONO_8 },
    { 0, ARV_PIXEL_FORMAT_MONO_8_SIGNED },
    { 0, ARV_PIXEL_FORMAT_MONO_10 },
    { 0, ARV_PIXEL_FORMAT_MONO_10_PACKED },
    { 0, ARV_PIXEL_FORMAT_MONO_12 },
    { 0, ARV_PIXEL_FORMAT_MONO_12_PACKED },
    { 0, ARV_PIXEL_FORMAT_MONO_14 },
    { FOURCC_Y16,    ARV_PIXEL_FORMAT_MONO_16 },
    { FOURCC_GRBG8,  ARV_PIXEL_FORMAT_BAYER_GR_8 },
    { FOURCC_RGGB8,  ARV_PIXEL_FORMAT_BAYER_RG_8 },
    { FOURCC_GBRG8,  ARV_PIXEL_FORMAT_BAYER_GB_8 },
    { FOURCC_BGGR8,  ARV_PIXEL_FORMAT_BAYER_BG_8 },
    { FOURCC_GRBG10, ARV_PIXEL_FORMAT_BAYER_GR_10 },
    { FOURCC_RGGB10, ARV_PIXEL_FORMAT_BAYER_RG_10 },
    { FOURCC_GBRG10, ARV_PIXEL_FORMAT_BAYER_GB_10 },
    { FOURCC_BGGR10, ARV_PIXEL_FORMAT_BAYER_BG_10 },
    { FOURCC_GRBG12, ARV_PIXEL_FORMAT_BAYER_GR_12 },
    { FOURCC_RGGB12, ARV_PIXEL_FORMAT_BAYER_RG_12 },
    { FOURCC_GBRG12, ARV_PIXEL_FORMAT_BAYER_GB_12 },
    { FOURCC_BGGR12, ARV_PIXEL_FORMAT_BAYER_BG_12 },
    { 0, ARV_PIXEL_FORMAT_BAYER_GR_12_PACKED },
    { 0, ARV_PIXEL_FORMAT_BAYER_RG_12_PACKED },
    { 0, ARV_PIXEL_FORMAT_BAYER_GB_12_PACKED },
    { 0, ARV_PIXEL_FORMAT_BAYER_BG_12_PACKED },
    { FOURCC_GRBG16, ARV_PIXEL_FORMAT_BAYER_GR_16 },
    { FOURCC_RGGB16, ARV_PIXEL_FORMAT_BAYER_RG_16 },
    { FOURCC_GBRG16, ARV_PIXEL_FORMAT_BAYER_GB_16 },
    { FOURCC_BGGR16, ARV_PIXEL_FORMAT_BAYER_BG_16 },
    { 0, ARV_PIXEL_FORMAT_RGB_8_PACKED },
    { 0, ARV_PIXEL_FORMAT_BGR_8_PACKED },
    { 0, ARV_PIXEL_FORMAT_RGBA_8_PACKED },
    { 0, ARV_PIXEL_FORMAT_BGRA_8_PACKED },
    { 0, ARV_PIXEL_FORMAT_RGB_10_PACKED },
    { 0, ARV_PIXEL_FORMAT_BGR_10_PACKED },
    { 0, ARV_PIXEL_FORMAT_RGB_12_PACKED },
    { 0, ARV_PIXEL_FORMAT_BGR_12_PACKED },
    { 0, ARV_PIXEL_FORMAT_YUV_411_PACKED },
    { 0, ARV_PIXEL_FORMAT_YUV_422_PACKED },
    { 0, ARV_PIXEL_FORMAT_YUV_444_PACKED },
    { 0, ARV_PIXEL_FORMAT_RGB_8_PLANAR },
    { 0, ARV_PIXEL_FORMAT_RGB_10_PLANAR },
    { 0, ARV_PIXEL_FORMAT_RGB_12_PLANAR },
    { 0, ARV_PIXEL_FORMAT_RGB_16_PLANAR },
    { 0, ARV_PIXEL_FORMAT_YUV_422_YUYV_PACKED },
    { 0, ARV_PIXEL_FORMAT_CUSTOM_BAYER_GR_12_PACKED },
    { 0, ARV_PIXEL_FORMAT_CUSTOM_BAYER_RG_12_PACKED },
    { 0, ARV_PIXEL_FORMAT_CUSTOM_BAYER_GB_12_PACKED },
    { 0, ARV_PIXEL_FORMAT_CUSTOM_BAYER_BG_12_PACKED },
    { 0, ARV_PIXEL_FORMAT_CUSTOM_YUV_422_YUYV_PACKED },
    { 0, ARV_PIXEL_FORMAT_CUSTOM_BAYER_GR_16 },
    { 0, ARV_PIXEL_FORMAT_CUSTOM_BAYER_RG_16 },
    { 0, ARV_PIXEL_FORMAT_CUSTOM_BAYER_GB_16 },
    { 0, ARV_PIXEL_FORMAT_CUSTOM_BAYER_BG_16 },
};


uint32_t AravisDevice::aravis2fourcc (uint32_t aravis)
{
    for (const auto& e : arv_fourcc_conversion_table)
    {
        if (e.aravis == aravis)
        {
            return e.fourcc;
        }
    }

    return 0;
}


uint32_t AravisDevice::fourcc2aravis (uint32_t fourcc)
{
    for (const auto& e : arv_fourcc_conversion_table)
    {
        if (e.fourcc == fourcc)
        {
            return e.aravis;
        }
    }

    return 0;
}


//// genicam handling

void AravisDevice::index_genicam ()
{
    if (this->arv_camera == nullptr)
    {
        return;
    }
    genicam = arv_device_get_genicam(arv_camera_get_device(this->arv_camera));

    iterate_genicam("Root");
    index_genicam_format(NULL);
}


void AravisDevice::iterate_genicam (const char* feature)
{


    ArvGcNode* node = arv_gc_get_node (genicam, feature);

    if (ARV_IS_GC_FEATURE_NODE (node) &&
        arv_gc_feature_node_is_implemented (ARV_GC_FEATURE_NODE (node), NULL) &&
        arv_gc_feature_node_is_available (ARV_GC_FEATURE_NODE (node), NULL))
    {

        if (ARV_IS_GC_CATEGORY (node))
        {
            const GSList* features;
            const GSList* iter;

            features = arv_gc_category_get_features (ARV_GC_CATEGORY (node));

            for (iter = features; iter != NULL; iter = iter->next)
            {
                iterate_genicam ((char*)iter->data);
            }
            return;
        }

        // TODO: how to handle private settings
        static std::vector<std::string> private_settings = { "TLParamsLocked",
                                                             "GevSCPSDoNotFragment",
                                                             "GevTimestampTickFrequency",
                                                             "GevTimeSCPD",
                                                             "GevSCPD",
                                                             "PayloadSize",
                                                             "PayloadPerFrame",
                                                             "PayloadPerPacket",
                                                             "TotalPacketSize",
                                                             "PacketsPerFrame",
                                                             "PacketTimeUS",
                                                             "GevSCPSPacketSize",
                                                             "GevSCPSFireTestPacket"};

        static std::vector<std::string> format_member = { "AcquisitionStart",
                                                          "AcquisitionStop",
                                                          "AcquisitionMode",
                                                          // "Binning",
                                                          "SensorWidth",
                                                          "SensorHeight",
                                                          "Width",
                                                          "Height",
                                                          "FPS",
                                                          "PixelFormat"};
        property_mapping m;

        m.arv_ident = feature;
        m.prop = createProperty(arv_camera, node, handler);

        if (m.prop == nullptr)
        {
            tcam_log(TCAM_LOG_ERROR, "Property '%s' is null", m.arv_ident.c_str());
            return;
        }


        if (std::find(private_settings.begin(), private_settings.end(), feature) != private_settings.end())
        {
            // TODO: implement handling
        }
        // is part of the format description
        else if (std::find(format_member.begin(), format_member.end(), feature) != format_member.end())
        {
            // index_genicam_format(camera, node, frmt_mapping);
            this->format_nodes.push_back(node);
        }
        else
        {
            handler->properties.push_back(m);
        }
    }
}


void AravisDevice::index_genicam_format (ArvGcNode* /* node */ )
{
    // genicam formats behave like follows:
    // All framerates are valid for all frame sizes
    // All frame sizes are valid for all formats

    // We search for the wanted node and save the intermediate result
    std::string node_to_use;
    auto find_node = [&node_to_use] (ArvGcNode* node)
        {
            return (node_to_use.compare(arv_gc_feature_node_get_name((ArvGcFeatureNode*) node)) == 0);
        };

    // work your way from bottom to top
    // start with frame rates and use everthing until all format descriptions are complete

    node_to_use = "FPS";
    auto fps_node = std::find_if(format_nodes.begin() , format_nodes.end(), find_node);

    std::vector<double> fps;

    if (fps_node != format_nodes.end())
    {

        if (ARV_IS_GC_ENUMERATION (*fps_node))
        {
            const GSList* childs;
            const GSList* iter;

            childs = arv_gc_enumeration_get_entries (ARV_GC_ENUMERATION (*fps_node));
            for (iter = childs; iter != NULL; iter = iter->next)
            {
                if (arv_gc_feature_node_is_implemented ((ArvGcFeatureNode*)iter->data, NULL))
                {
                    if (strcmp(arv_dom_node_get_node_name ((ArvDomNode*)iter->data), "EnumEntry") == 0)
                    {
                        GError* error = NULL;

                        // this is the denominator of our framerate
                        uint64_t val = arv_gc_enum_entry_get_value(ARV_GC_ENUM_ENTRY(iter->data), &error);
                        double f = 1.0 / (uint32_t)val * 10000000;

                        fps.push_back(f);
                    }
                }
            }
        }
        else
        {
            double min;
            double max;
            arv_camera_get_frame_rate_bounds(this->arv_camera ,&min, &max);

            fps.push_back(min);
            fps.push_back(max);
        }
    }
    else
    {
        tcam_log(TCAM_LOG_ERROR, "Unable to find fps node.");
        // fallback and atleast try to not wreck the whole system

        // show that something went wrong
        fps.push_back(0.0);
    }


    node_to_use = "Binning";
    auto binning_node = std::find_if(format_nodes.begin() , format_nodes.end(), find_node);

    std::vector<int> binning;

    if (binning_node != format_nodes.end())
    {

        if (ARV_IS_GC_ENUMERATION (*binning_node))
        {
            const GSList* childs;
            const GSList* iter;

            childs = arv_gc_enumeration_get_entries (ARV_GC_ENUMERATION (*binning_node));
            for (iter = childs; iter != NULL; iter = iter->next)
            {
                if (arv_gc_feature_node_is_implemented ((ArvGcFeatureNode*)iter->data, NULL))
                {
                    if (strcmp(arv_dom_node_get_node_name ((ArvDomNode*)iter->data), "EnumEntry") == 0)
                    {
                        GError* error = NULL;

                        // this is the denominator of our framerate
                        int64_t val = arv_gc_enum_entry_get_value(ARV_GC_ENUM_ENTRY(iter->data), &error);

                        // std::cout << "Binning entry: "
                        // << arv_gc_feature_node_get_name ((ArvGcFeatureNode*)iter->data)
                        // << " - "
                        // << val << std::endl;

                        binning.push_back(val);
                    }
                }
            }
        }
        else
        {
            // int range
            // TODO implement
        }

    }
    else
    {
        // default value
        binning.push_back(0);
    }

    // TODO are there cameras that only have fixed resolutions?

    int width_min = 0;
    int width_max = 0;

    int height_min = 0;
    int height_max = 0;

    arv_camera_get_width_bounds(this->arv_camera, &width_min, &width_max);
    arv_camera_get_height_bounds(this->arv_camera, &height_min, &height_max);

    tcam_image_size min = {(unsigned int)width_min, (unsigned int)height_min};
    tcam_image_size max = {(unsigned int)width_max, (unsigned int)height_max};

    node_to_use = "PixelFormat";

    auto pixel_node = std::find_if(format_nodes.begin(), format_nodes.end(), find_node);

    if (pixel_node != format_nodes.end())
    {

        // we know there are formats
        // use official API to retrieve info
        // if needed we can perform additional genicam interpretation here

        unsigned  n_formats;
        gint64* format_ptr = arv_camera_get_available_pixel_formats ( this->arv_camera, &n_formats );
        unsigned  n2_formats;
        const char** format_str = arv_camera_get_available_pixel_formats_as_strings ( this->arv_camera, &n2_formats );

        if ( n_formats != n2_formats )
        {
            tcam_log ( TCAM_LOG_ERROR, "Format retrieval encountered nonsensical information" );
        }

        for ( int i = 0; i < n_formats; ++i )
        {
            struct tcam_video_format_description desc = {};

            desc.fourcc = format_ptr[i];

            desc.fourcc = aravis2fourcc(desc.fourcc);

            if (desc.fourcc == 0)
            {
                tcam_log(TCAM_LOG_ERROR, "Input format no supported! \"%x\"", format_ptr[i]);
            }

            memcpy(desc.description, format_str[i], sizeof(desc.description));

            res_fps rf = {};

            rf.resolution = {};

            double fps_min;
            double fps_max;

            //arv_camera_get_frame_rate_bounds ( this->arv_camera, &fps_min, &fps_max );


            rf.resolution = min;
            // rf.fps.push_back ( fps_min );
            // rf.fps.push_back ( fps_max );

            rf.fps =fps;

            std::vector<res_fps> res_vec;

            res_vec.push_back ( rf );

            rf.resolution = max;

            res_vec.push_back ( rf );

            struct tcam_video_format_description d = desc;

            d.framerate_type = TCAM_FRAMERATE_TYPE_RANGE;
            d.min_size = min;
            d.max_size = max;

            tcam_log(TCAM_LOG_DEBUG, "Adding format desc: %s (%x) ", desc.description, desc.fourcc);

            this->available_videoformats.push_back ( VideoFormatDescription ( d, res_vec ) );

        }
    }
    else
    {
        tcam_log(TCAM_LOG_ERROR, "NO PixelFormat Node");

        // TODO
    }

}
