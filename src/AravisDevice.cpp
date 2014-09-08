



#include "AravisDevice.h"

#include "PropertyGeneration.h"
#include "tis_logging.h"

#include <algorithm>
#include <cstring>


using namespace tis_imaging;

AravisDevice::AravisDevice (const CaptureDevice& _device)
    : device(_device), current_buffer(0), stream(NULL)
{
    this->arv_camera = arv_camera_new (this->device.getInfo().identifier);

    if (this->arv_camera == NULL)
    {
        throw std::runtime_error("Error while creating ArvCamera");
    }

    arv_options.auto_socket_buffer = false;
    arv_options.no_packet_resend = false;
    // arv_options.no_packet_resend = TRUE;
    arv_options.packet_timeout = 20;
    arv_options.frame_retention = 100;

    index_genicam();

}


AravisDevice::~AravisDevice ()
{
    if (arv_camera != NULL)
    {
        g_object_unref (arv_camera);
        arv_camera = NULL;
    }
}


CaptureDevice AravisDevice::getDeviceDescription () const
{
    return device;
}


std::vector<std::shared_ptr<Property>> AravisDevice::getProperties ()
{
    if (this->properties.empty())
    {
        index_genicam();
    }
    std::vector<std::shared_ptr<Property>> vec;

    for (auto& p : properties)
    {
        vec.push_back(p.prop);
    }
    tis_log(TIS_LOG_DEBUG, "Returning %d properties", vec.size());

    return vec;
}


bool AravisDevice::isAvailable (const Property&)
{
    return false;
}


bool AravisDevice::setProperty (const Property& p)
{
    auto f = [p] (const property_mapping& m)
        {
            return p.getName().compare(m.prop->getName()) == 0;
        };

    auto pm = std::find_if(properties.begin(), properties.end(), f);

    if (pm == properties.end())
    {
        return false;
    }

    auto device = arv_camera_get_device(arv_camera);

    Property::VALUE_TYPE value_type = pm->prop->getValueType();

    PROPERTY_TYPE type = pm->prop->getType();

    if (type == PROPERTY_TYPE_INTEGER)
    {
        auto prop_impl = (PropertyInteger&) (*pm->prop);
    }
    else if (type == PROPERTY_TYPE_DOUBLE)
    {

    }
    else
    {
        return false;
    }

    switch (value_type)
    {
        case Property::BOOLEAN:
        {
            break;
        }
        case Property::STRING:
        {
            break;
        }
        case Property::ENUM:
        {
            break;
        }
        case Property::INTEGER:
        {
            // PropertyInteger&
            arv_device_set_integer_feature_value(device, pm->arv_ident.c_str(), ((PropertyInteger&) (*pm->prop)).getValue());
            break;
        }
        case Property::INTSWISSKNIFE:
        {
            break;
        }
        case Property::FLOAT:
        {
            arv_device_set_float_feature_value(device, pm->arv_ident.c_str(), ((PropertyInteger&) (*pm->prop)).getValue());
            break;
        }
        case Property::COMMAND:
        {
            //arv_device_
            arv_device_execute_command(device, pm->arv_ident.c_str());
            break;
        }
        case Property::BUTTON:
        {
            break;
        }
        case Property::UNDEFINED:
        default:
        {
            break;
        }
    }
    return true;
}


bool AravisDevice::getProperty (Property&)
{
    return false;
}


bool AravisDevice::setVideoFormat (const VideoFormat& _format)
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

    arv_camera_set_frame_rate (this->arv_camera, _format.getFramerate());

    arv_camera_set_pixel_format(this->arv_camera, _format.getFourcc());

    // TODO: validity check
    arv_camera_set_binning(this->arv_camera, _format.getBinning(), _format.getBinning());

    // TODO: auto center

    arv_camera_set_region(this->arv_camera, 0, 0, _format.getSize().width, _format.getSize().height);

    determine_active_video_format();

    return true;
}


VideoFormat AravisDevice::getActiveVideoFormat () const
{
    return active_video_format;
}


void AravisDevice::determine_active_video_format ()
{

    this->active_video_format.setFramerate(arv_camera_get_frame_rate (this->arv_camera));
    active_video_format.setFourcc(arv_camera_get_pixel_format(this->arv_camera));

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
        tis_log(TIS_LOG_ERROR, "ArvCamera missing!");
        return false;
    }
    if (external_sink == nullptr)
    {
        tis_log(TIS_LOG_ERROR, "No sink specified");
        return false;
    }

    this->stream = arv_camera_create_stream(this->arv_camera, NULL, NULL);

    if (this->stream == NULL)
    {
        tis_log(TIS_LOG_ERROR, "Unable to create ArvStream.");
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

    tis_log(TIS_LOG_INFO, "Starting actual stream...");

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
        tis_log(TIS_LOG_ERROR, "Callback camera instance is NULL.");
        return;
    }
    if (self->stream == NULL)
    {
        return;
    }

    ArvBuffer* buffer = arv_stream_pop_buffer (self->stream);

    if (buffer != NULL)
    {
        if (buffer->status == ARV_BUFFER_STATUS_SUCCESS)
        {
            struct image_buffer desc = {0};

            desc.format = self->active_video_format.getFormatDescription();


            desc.pData = (unsigned char*)buffer->data;
            desc.length = buffer->size;

            if (buffer->data == NULL)
            {
                tis_log(TIS_LOG_ERROR, "FUCKING HELL");
            }

            self->buffers.at(self->current_buffer)->setImageBuffer(desc);
            //tis_log(TIS_LOG_DEBUG, "Pushing new image buffer to sink.");
            self->external_sink->pushImage(self->buffers.at(self->current_buffer));

            if (self->current_buffer < self->buffers.size() -1)
                self->current_buffer++;
            else
                self->current_buffer = 0;
        }
        else
        {
            std::string msg;
            switch (buffer->status)
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

                    // struct image_buffer desc = {0};

                    // desc.format = self->active_video_format.getFormatDescription();


                    // desc.pData = (unsigned char*)buffer->data;
                    // desc.length = buffer->size;

                    // if (buffer->data == NULL)
                    // {
                    // tis_log(TIS_LOG_ERROR, "FUCKING HELL");
                    // }

                    // self->buffers.at(self->current_buffer)->setImageBuffer(desc);
                    // tis_log(TIS_LOG_DEBUG, "Pushing new image buffer to sink.");
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
            }
            tis_log(TIS_LOG_WARNING, msg.c_str());
        }
        //tis_log(TIS_LOG_DEBUG, "Returning buffer to aravis.");
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


//// genicam handling

void AravisDevice::index_genicam ()
{
    if (this->arv_camera == nullptr)
    {
        return;
    }

    iterate_genicam("Root");
    index_genicam_format(NULL);
}


void AravisDevice::iterate_genicam (const char* feature)
{

    ArvGc* genicam = arv_device_get_genicam(arv_camera_get_device(this->arv_camera));

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
        std::vector<std::string> private_settings = { "TLParamsLocked",
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

        std::vector<std::string> format_member = { "AcquisitionStart",
                                                   "AcquisitionStop",
                                                   "AcquisitionMode",
                                                   "Binning",
                                                   "SensorWidth",
                                                   "SensorHeight",
                                                   "FPS",
                                                   "PixelFormat"};

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
            property_mapping m;

            m.arv_ident = feature;
            m.prop = createProperty(arv_camera, node, shared_from_this());

            if (m.prop == nullptr)
            {
                tis_log(TIS_LOG_ERROR, "Property '%s' is null", m.arv_ident.c_str());
                return;
            }

            properties.push_back(m);
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

                        // std::cout << "FPS entry: "
                        // << arv_gc_feature_node_get_name ((ArvGcFeatureNode*)iter->data)
                        // << " - "
                        // << (uint32_t)val << "" << std::endl;

                        double f = (double)1 / (uint32_t)val;

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
        tis_log(TIS_LOG_ERROR, "Unable to find fps node.");
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

    IMG_SIZE min = {(unsigned int)width_min, (unsigned int)height_min};
    IMG_SIZE max = {(unsigned int)width_max, (unsigned int)height_max};

    node_to_use = "PixelFormat";

    auto pixel_node = std::find_if(format_nodes.begin(), format_nodes.end(), find_node);

    if (pixel_node != format_nodes.end())
    {
        if (ARV_IS_GC_ENUMERATION (*pixel_node))
        {
            const GSList* childs;
            const GSList* iter;

            childs = arv_gc_enumeration_get_entries (ARV_GC_ENUMERATION (*pixel_node));
            for (iter = childs; iter != NULL; iter = iter->next)
            {
                if (arv_gc_feature_node_is_implemented ((ArvGcFeatureNode*)iter->data, NULL))
                {
                    if (strcmp(arv_dom_node_get_node_name ((ArvDomNode*)iter->data), "EnumEntry") == 0)
                    {
                        GError* error = NULL;

                        struct video_format_description desc = {0};

                        memcpy(desc.description,
                               arv_gc_feature_node_get_name ((ArvGcFeatureNode*)iter->data),
                               sizeof(desc.description));

                        // this is fourcc. unless otherwise specified it should assumed,
                        // that this fourcc is v4l2 compatible
                        desc.fourcc = arv_gc_enum_entry_get_value(ARV_GC_ENUM_ENTRY(iter->data), &error);
                        tis_log(TIS_LOG_WARNING, "Found Format: %d", desc.fourcc);

                        // merge frame_size and frame rates
                        // struct buffer_size_desc bsd = {0};
                        // bsd.frame = frame_size;
                        // bsd.fps = fps;

                        // std::cout << "FPS size " << fps.size() << std::endl;

                        // std::vector<buffer_size_desc> bsd_vec;
                        // bsd_vec.push_back(bsd);

                        // we create a format for every binning value and store it seperately
                        for ( const auto& b : binning)
                        {
                            struct video_format_description d = desc;

                            d.binning = b;
                            d.framerate_type = TIS_FRAMERATE_TYPE_RANGE;
                            d.min_size = min;
                            d.max_size = max;

                            this->available_videoformats.push_back(VideoFormatDescription(d, fps));
                        }
                    }
                }
            }
        }
        else
        {
            tis_log(TIS_LOG_ERROR, "No PixelFormat Enumeration");
            // TODO
        }
    }
    else
    {
        tis_log(TIS_LOG_ERROR, "NO PixelFormat Node");

        // TODO
    }

}
