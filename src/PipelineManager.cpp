
#include "PipelineManager.h"

#include "tis_logging.h"

#include <ctime>
#include <cstring>
#include <algorithm>
#include <tis_utils.h>

using namespace tis_imaging;

PipelineManager::PipelineManager ()
    : status(PIPELINE_UNDEFINED), filter_loader(), frame_count(0)
{
    filter_loader.index_possible_filter();
    filter_loader.open_possible_filter();
    available_filter = filter_loader.get_all_filter();



}


PipelineManager::~PipelineManager ()
{
    if (status == PIPELINE_PLAYING)
    {
        stop_playing();
    }

    available_filter.clear();
    filter_pipeline.clear();
    filter_properties.clear();
    filter_loader.drop_all_filter();
}


std::vector<std::shared_ptr<Property>> PipelineManager::getFilterProperties ()
{
    return filter_properties;
}


std::vector<VideoFormatDescription> PipelineManager::getAvailableVideoFormats () const
{
    return available_output_formats;
}


bool PipelineManager::setVideoFormat(const VideoFormat& f)
{
    this->format = f;
}


bool PipelineManager::setStatus (const PIPELINE_STATUS& s)
{
    if (status == s)
        return true;

    this->status = s;

    if (status == PIPELINE_PLAYING)
    {
        frame_count = 0;
        second_count = time(0);
        if (create_pipeline())
        {
            start_playing();
            tis_log(TIS_LOG_INFO, "All pipeline elements set to PLAYING.");
        }
        else
        {
            status = PIPELINE_ERROR;
            // TODO: error
            return false;
        }
    }
    else if (status == PIPELINE_STOPPED)
    {

    }

    return true;
}


PIPELINE_STATUS PipelineManager::getStatus () const
{
    return status;
}


bool PipelineManager::destroyPipeline ()
{
    setStatus(PIPELINE_STOPPED);

    source = nullptr;
    sink = nullptr;

    return true;
}


void PipelineManager::index_output_formats ()
{
    if (available_input_formats.empty() || available_filter.empty())
    {

        return ;
    }

    uint32_t fourcc = 0;
    auto dev_formats = [&fourcc] (const VideoFormatDescription& vfd)
        {
            tis_log(TIS_LOG_DEBUG,
                    "Testing %s against %s",
                    fourcc2description(fourcc),
                    fourcc2description(vfd.getFormatDescription().fourcc));

            return vfd.getFormatDescription().fourcc == fourcc;
        };


    for (auto f : available_filter)
    {
        if (f->getDescription().type == FILTER_TYPE_CONVERSION)
        {

            auto output = f->getDescription().output_fourcc;
            auto input = f->getDescription().input_fourcc;

            bool match = false;
            for (const uint32_t& fi : input)
            {
                fourcc = fi;
                auto format_match = std::find_if (available_input_formats.begin(),
                                                  available_input_formats.end(),
                                                  dev_formats);

                if (format_match != available_input_formats.end())
                {
                    for (uint32_t fcc : output)
                    {
                        auto desc = format_match->getFormatDescription();
                        auto rf   = format_match-> getResolutionsFramesrates();

                        desc.fourcc = fcc;
                        memcpy(desc.description, fourcc2description(fcc), sizeof(desc.description));
                        
                        VideoFormatDescription v (desc, rf);
                        available_output_formats.push_back (v);
                    }
                }
            }

            if (match == false) // does not allow for input format
            {
                continue;
            }
        }
    }

    // to finalize iterate input formats and select those that shall be passed through
    
    std::vector<uint32_t> pass_through = {FOURCC_Y800, FOURCC_Y16};

    for (auto f : pass_through)
    {
        fourcc = f;
        auto match = std::find_if(available_input_formats.begin(), available_input_formats.end(), dev_formats);

        if (match != available_input_formats.end())
        {
            tis_log(TIS_LOG_ERROR, "Passing format through %s", fourcc2description(f));
            available_output_formats.push_back(*match);
        }
        else
        {
            tis_log(TIS_LOG_DEBUG, "Device format '%s' will not be passed to user.", fourcc2description(f));
        }
    }
}


bool PipelineManager::setSource (std::shared_ptr<DeviceInterface> device)
{
    if (status == PIPELINE_PLAYING || status == PIPELINE_PAUSED)
    {
        return false;
    }

    properties = device->getProperties();
    available_input_formats = device->getAvailableVideoFormats();

    distributeProperties();

    this->source = std::make_shared<ImageSource>();
    source->setSink(shared_from_this());

    source->setDevice(device);

    for (const auto& f : available_filter)
    {
        auto p = f->getFilterProperties();

        if (!p.empty())
        {
            filter_properties.insert(filter_properties.end(), p.begin(), p.end());
        }
    }

    index_output_formats();

    return true;
}


std::shared_ptr<ImageSource> PipelineManager::getSource ()
{
    return source;
}


bool PipelineManager::setSink (std::shared_ptr<SinkInterface> s)
{
    if (status == PIPELINE_PLAYING || status == PIPELINE_PAUSED)
    {
        return false;
    }

    this->sink = s;

    return true;
}


std::shared_ptr<SinkInterface> PipelineManager::getSink ()
{
    return sink;
}


void PipelineManager::distributeProperties ()
{
    for (auto& f : available_filter)
    {
        f->setDeviceProperties(properties);
    }
}


bool isFilterApplicable (const uint32_t& fourcc,
                         const std::vector<uint32_t>& vec)
{
    if (std::find(vec.begin(), vec.end(), fourcc) == vec.end())
    {
        return false;
    }
    return true;
}


void PipelineManager::create_input_format (const uint32_t& fourcc)
{
    input_format = format;
    input_format.setFourcc(fourcc);
}


std::vector<uint32_t> PipelineManager::getDeviceFourcc ()
{
    // for easy usage we create a vector<fourcc> for avail. inputs
    std::vector<uint32_t> device_fourcc;

    for (const auto& v : available_input_formats)
    {
        tis_log(TIS_LOG_DEBUG,
                "Found device fourcc '%s' - %d",
                fourcc2description(v.getFormatDescription().fourcc),
                v.getFormatDescription().fourcc);

        device_fourcc.push_back(v.getFormatDescription().fourcc);
    }
    return device_fourcc;
}


bool PipelineManager::validate_pipeline ()
{
    // check if pipeline is valid
    if (source.get() == nullptr || sink.get() == nullptr)
    {
        // TODO: error
        return false;
    }

    // check source format
    auto in_format = source->getVideoFormat();

    if (in_format != this->input_format)
    {
        tis_log(TIS_LOG_DEBUG,
                "Video format in source does not match pipeline: '%s' != '%s'",
                in_format.toString().c_str(),
                input_format.toString().c_str());
        return false;
    }

    VideoFormat in;
    VideoFormat out;
    for (auto f : filter_pipeline)
    {

        f->getVideoFormat(in, out);

        if (in != in_format)
        {
            tis_log(TIS_LOG_ERROR,
                    "Ingoing video format for filter %s is not compatible with previous element. '%s' != '%s'",
                    f->getDescription().name.c_str(),
                    in_format.toString().c_str(),
                    in.toString().c_str());
            // TODO: error
            return false;
        }
        else
        {
            tis_log(TIS_LOG_DEBUG, "Filter %s connected to pipeline -- %s",
                    f->getDescription().name.c_str(),
                    out.toString().c_str());
            // save output for next comparison
            in_format = out;
        }
    }

    if (in_format != this->format)
    {
        tis_log(TIS_LOG_ERROR, "Video format in sink does not match pipeline '%s' != '%s'",
                in_format.toString().c_str(),
                format.toString().c_str());
        return false;
    }

    return true;
}


bool PipelineManager::create_conversion_pipeline ()
{
    if (source.get() == nullptr || sink.get() == nullptr)
    {
        // TODO: error
        return false;
    }

    auto device_fourcc = getDeviceFourcc();

    for (auto f : available_filter)
    {
        std::string s = f->getDescription().name;

        if (f->getDescription().type == FILTER_TYPE_CONVERSION)
        {

            if (isFilterApplicable(format.getFourcc(), f->getDescription().output_fourcc))
            {
                bool filter_valid = false;
                uint32_t fourcc_to_use = 0;
                for (const auto& cc : device_fourcc)
                {
                    if (isFilterApplicable(cc, f->getDescription().input_fourcc))
                    {
                        filter_valid = true;
                        fourcc_to_use = cc;
                        break;
                    }
                }

                // set device format to use correct fourcc
                create_input_format(fourcc_to_use);

                if (filter_valid)
                {
                    if (f->setVideoFormat(input_format, format))
                    {
                        tis_log(TIS_LOG_DEBUG,
                                "Added filter \"%s\" to pipeline",
                                s.c_str());
                        filter_pipeline.push_back(f);
                    }
                    else
                    {
                        tis_log(TIS_LOG_DEBUG,
                                "Filter %s did not accept format settings",
                                s.c_str());
                    }
                }
                else
                {
                    tis_log(TIS_LOG_DEBUG, "Filter %s does not use the device output formats.", s.c_str());
                }
            }
            else
            {
                tis_log(TIS_LOG_DEBUG, "Filter %s is not applicable", s.c_str());

            }
        }
    }
    return true;
}


bool PipelineManager::add_interpretation_filter ()
{

    // if a valid pipeline can be created insert additional filter (e.g. autoexposure)
    // interpretations should be done as early as possible in the pipeline

    for (auto& f : available_filter)
    {
        if (f->getDescription().type == FILTER_TYPE_INTERPRET)
        {
            std::string s  = f->getDescription().name;
            // applicable to sink
            bool all_formats = false;
            if (f->getDescription().input_fourcc.size() == 1)
            {
                if (f->getDescription().input_fourcc.at(0) == 0)
                {
                    all_formats = true;
                }
            }

            if (all_formats ||isFilterApplicable(input_format.getFourcc(), f->getDescription().input_fourcc))
            {
                tis_log(TIS_LOG_DEBUG, "Adding filter '%s' after source", s.c_str());
                f->setVideoFormat(input_format, input_format);
                filter_pipeline.insert(filter_pipeline.begin(), f);
                continue;
            }
            else
            {
                tis_log(TIS_LOG_DEBUG, "Filter '%s' not usable after source", s.c_str());
            }

            for (auto& filter : filter_pipeline)
            {
                if (f->setVideoFormat(input_format, input_format))
                {

                    continue;
                }
            }
        }
    }
    return true;
}


bool PipelineManager::allocate_conversion_buffer ()
{

    for (int i = 0; i < 5; ++i)
    {
        image_buffer b = {};
        b.pitch = format.getSize().width * img::getBitsPerPixel(format.getFourcc()) / 8;
        b.length = b.pitch * format.getSize().height;

        b.pData = (unsigned char*)malloc(b.length);

        b.format.fourcc = format.getFourcc();
        b.format.width = format.getSize().width;
        b.format.height = format.getSize().height;
        b.format.binning = format.getBinning();
        b.format.framerate = format.getFramerate();

        this->pipeline_buffer.push_back(std::make_shared<MemoryBuffer>(b));
    }
    return true;
}


bool PipelineManager::create_pipeline ()
{
    if (source.get() == nullptr || sink.get() == nullptr)
    {
        // TODO: error
        return false;
    }

    // assure everything is in a defined state
    filter_pipeline.clear();

    format.setFramerate(10.0);
    format.setBinning(0);

    if (!create_conversion_pipeline())
    {
        return false;
    }

    if (source->setVideoFormat(input_format))
    {
        tis_log(TIS_LOG_ERROR, "Unable to set video format in source.");
        //return false;
    }

    if (!add_interpretation_filter())
    {
        return false;
    }

    if (!allocate_conversion_buffer())
    {
        return false;
    }

    if (!validate_pipeline())
    {
        return false;
    }

    tis_log(TIS_LOG_INFO, "Pipeline creation successful.");

    std::string ppl = "source -> ";

    for (const auto& f : filter_pipeline)
    {
        ppl += f->getDescription().name;
        ppl += " -> ";
    }
    ppl += " sink";
    tis_log(TIS_LOG_INFO, "%s" , ppl.c_str());

    return true;
}


bool PipelineManager::start_playing ()
{

    if (!sink->setStatus(PIPELINE_PLAYING))
    {
        tis_log(TIS_LOG_ERROR, "Sink refused to change to state PLAYING");
        goto error;
    }

    for (auto& f : filter_pipeline)
    {
        if (!f->setStatus(PIPELINE_PLAYING))
        {
            tis_log(TIS_LOG_ERROR,
                    "Filter %s refused to change to state PLAYING",
                    f->getDescription().name.c_str());
            goto error;
        }
    }

    if (!source->setStatus(PIPELINE_PLAYING))
    {
        tis_log(TIS_LOG_ERROR, "Source refused to change to state PLAYING");
        goto error;
    }

    status = PIPELINE_PLAYING;

    return true;

error:

    stop_playing();
    return false;
}


bool PipelineManager::stop_playing ()
{
    status = PIPELINE_STOPPED;

    if (!source->setStatus(PIPELINE_STOPPED))
    {
        tis_log(TIS_LOG_ERROR, "Source refused to change to state STOP");
        return false;
    }

    for (auto& f : filter_pipeline)
    {
        if (!f->setStatus(PIPELINE_STOPPED))
        {
            tis_log(TIS_LOG_ERROR,
                    "Filter %s refused to change to state STOP",
                    f->getDescription().name.c_str());
            return false;
        }
    }

    if (!sink->setStatus(PIPELINE_STOPPED))
    {
        tis_log(TIS_LOG_ERROR, "Sink refused to change to state STOP");
        return false;
    }

    return true;
}


void PipelineManager::pushImage (std::shared_ptr<MemoryBuffer> buffer)
{
    if (status == PIPELINE_STOPPED)
    {
        return;
    }

    frame_count++;
    buffer->lock();

    auto current_buffer = buffer;

    for (auto& f : filter_pipeline)
    {
        if (f->getDescription().type == FILTER_TYPE_INTERPRET)
        {
            f->apply(current_buffer);
        }
        else if (f->getDescription().type == FILTER_TYPE_CONVERSION)
        {
            auto next_buffer = pipeline_buffer.at(0);

            f->transform(*current_buffer, *next_buffer);

            current_buffer = next_buffer;
        }
    }

    sink->pushImage(current_buffer);

    buffer->unlock();
}
