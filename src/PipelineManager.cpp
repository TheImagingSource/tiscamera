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

#include "PipelineManager.h"

#include "internal.h"

#include <ctime>
#include <cstring>
#include <algorithm>
#include <utils.h>

using namespace tcam;

PipelineManager::PipelineManager ()
    : status(TCAM_PIPELINE_UNDEFINED), current_ppl_buffer(0)
{}


PipelineManager::~PipelineManager ()
{
    if (status == TCAM_PIPELINE_PLAYING)
    {
        stop_playing();
    }

    available_filter.clear();
    filter_pipeline.clear();
    filter_properties.clear();
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
    this->output_format = f;
    return true;
}


VideoFormat PipelineManager::getVideoFormat () const
{
    return this->output_format;
}


bool PipelineManager::set_status (TCAM_PIPELINE_STATUS s)
{
    if (status == s)
        return true;

    this->status = s;

    if (status == TCAM_PIPELINE_PLAYING)
    {
        if (create_pipeline())
        {
            start_playing();
            tcam_log(TCAM_LOG_INFO, "All pipeline elements set to PLAYING.");
        }
        else
        {
            status = TCAM_PIPELINE_ERROR;
            return false;
        }
    }
    else if (status == TCAM_PIPELINE_STOPPED)
    {
      stop_playing();
    }

    return true;
}


TCAM_PIPELINE_STATUS PipelineManager::get_status () const
{
    return status;
}


bool PipelineManager::destroyPipeline ()
{
    set_status(TCAM_PIPELINE_STOPPED);

    source = nullptr;
    sink = nullptr;

    return true;
}


bool PipelineManager::setSource (std::shared_ptr<DeviceInterface> device)
{
    if (status == TCAM_PIPELINE_PLAYING || status == TCAM_PIPELINE_PAUSED)
    {
        return false;
    }

    device_properties = device->getProperties();
    available_input_formats = device->get_available_video_formats();

    tcam_log(TCAM_LOG_DEBUG, "Received %zu formats.", available_input_formats.size());

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

    available_output_formats = available_input_formats;

    if (available_output_formats.empty())
    {
        tcam_log(TCAM_LOG_ERROR, "No output formats available.");
        return false;
    }

    return true;
}


std::shared_ptr<ImageSource> PipelineManager::getSource ()
{
    return source;
}


bool PipelineManager::setSink (std::shared_ptr<SinkInterface> s)
{
    if (status == TCAM_PIPELINE_PLAYING || status == TCAM_PIPELINE_PAUSED)
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
        f->setDeviceProperties(device_properties);
    }
}


static bool isFilterApplicable (uint32_t fourcc,
                                const std::vector<uint32_t>& vec)
{
    if (std::find(vec.begin(), vec.end(), fourcc) == vec.end())
    {
        return false;
    }
    return true;
}


void PipelineManager::create_input_format (uint32_t fourcc)
{
    input_format = output_format;
    input_format.set_fourcc(fourcc);
}


std::vector<uint32_t> PipelineManager::getDeviceFourcc ()
{
    // for easy usage we create a vector<fourcc> for avail. inputs
    std::vector<uint32_t> device_fourcc;

    for (const auto& v : available_input_formats)
    {
        tcam_log(TCAM_LOG_DEBUG,
                "Found device fourcc '%s' - %d",
                fourcc2description(v.get_fourcc()),
                v.get_fourcc());

        device_fourcc.push_back(v.get_fourcc());
    }
    return device_fourcc;
}


bool PipelineManager::set_source_status (TCAM_PIPELINE_STATUS status)
{
    if (source == nullptr)
    {
        tcam_log(TCAM_LOG_ERROR, "Source is not defined");
        return false;
    }

    if (!source->set_status(status))
    {
        tcam_log(TCAM_LOG_ERROR, "Source did not accept status change");
        return false;
    }

    return true;
}


bool PipelineManager::set_sink_status (TCAM_PIPELINE_STATUS status)
{
    if (sink == nullptr)
    {
        tcam_log(TCAM_LOG_WARNING, "Sink is not defined.");
        return false;
    }

    if (!sink->set_status(status))
    {
        tcam_log(TCAM_LOG_ERROR, "Sink spewed error");
        return false;
    }

    return true;
}


bool PipelineManager::validate_pipeline ()
{
    // check if pipeline is valid
    if (source.get() == nullptr || sink.get() == nullptr)
    {
        return false;
    }

    // check source format
    auto in_format = source->getVideoFormat();

    if (in_format != this->input_format)
    {
        tcam_log(TCAM_LOG_DEBUG,
                "Video format in source does not match pipeline: '%s' != '%s'",
                in_format.to_string().c_str(),
                input_format.to_string().c_str());
        return false;
    }
    else
    {
        tcam_log(TCAM_LOG_DEBUG,
                 "Starting pipeline with format: '%s'",
                 in_format.to_string().c_str());
    }

    VideoFormat in;
    VideoFormat out;
    for (auto f : filter_pipeline)
    {

        f->getVideoFormat(in, out);

        if (in != in_format)
        {
            tcam_log(TCAM_LOG_ERROR,
                    "Ingoing video format for filter %s is not compatible with previous element. '%s' != '%s'",
                    f->getDescription().name.c_str(),
                    in_format.to_string().c_str(),
                    in.to_string().c_str());
            return false;
        }
        else
        {
            tcam_log(TCAM_LOG_DEBUG, "Filter %s connected to pipeline -- %s",
                    f->getDescription().name.c_str(),
                    out.to_string().c_str());
            // save output for next comparison
            in_format = out;
        }
    }

    if (in_format != this->output_format)
    {
        tcam_log(TCAM_LOG_ERROR, "Video format in sink does not match pipeline '%s' != '%s'",
                in_format.to_string().c_str(),
                output_format.to_string().c_str());
        return false;
    }

    return true;
}


bool PipelineManager::create_conversion_pipeline ()
{
    if (source.get() == nullptr || sink.get() == nullptr)
    {
        return false;
    }

    auto device_fourcc = getDeviceFourcc();
    create_input_format(output_format.get_fourcc());

    for (auto f : available_filter)
    {
        std::string s = f->getDescription().name;

        if (f->getDescription().type == FILTER_TYPE_CONVERSION)
        {

            if (isFilterApplicable(output_format.get_fourcc(), f->getDescription().output_fourcc))
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
                    if (f->setVideoFormat(input_format, output_format))
                    {
                        tcam_log(TCAM_LOG_DEBUG,
                                "Added filter \"%s\" to pipeline",
                                s.c_str());
                        filter_pipeline.push_back(f);
                    }
                    else
                    {
                        tcam_log(TCAM_LOG_DEBUG,
                                "Filter %s did not accept format settings",
                                s.c_str());
                    }
                }
                else
                {
                    tcam_log(TCAM_LOG_DEBUG, "Filter %s does not use the device output formats.", s.c_str());
                }
            }
            else
            {
                tcam_log(TCAM_LOG_DEBUG, "Filter %s is not applicable", s.c_str());

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

            if (all_formats ||isFilterApplicable(input_format.get_fourcc(), f->getDescription().input_fourcc))
            {
                tcam_log(TCAM_LOG_DEBUG, "Adding filter '%s' after source", s.c_str());
                f->setVideoFormat(input_format, input_format);
                filter_pipeline.insert(filter_pipeline.begin(), f);
                continue;
            }
            else
            {
                tcam_log(TCAM_LOG_DEBUG, "Filter '%s' not usable after source", s.c_str());
            }

            if (f->setVideoFormat(input_format, input_format))
            {
                continue;
            }
        }
    }
    return true;
}


bool PipelineManager::allocate_conversion_buffer ()
{
    pipeline_buffer.clear();

    for (int i = 0; i < 5; ++i)
    {
        tcam_image_buffer b = {};
        b.pitch = output_format.get_size().width * img::get_bits_per_pixel(output_format.get_fourcc()) / 8;
        b.length = b.pitch * output_format.get_size().height;

        b.pData = (unsigned char*)malloc(b.length);

        b.format.fourcc = output_format.get_fourcc();
        b.format.width = output_format.get_size().width;
        b.format.height = output_format.get_size().height;
        b.format.framerate = output_format.get_framerate();

        this->pipeline_buffer.push_back(std::make_shared<MemoryBuffer>(b));
    }

    current_ppl_buffer = 0;

    return true;
}


bool PipelineManager::create_pipeline ()
{
    if (source.get() == nullptr || sink.get() == nullptr)
    {
        return false;
    }

    // assure everything is in a defined state
    filter_pipeline.clear();

    if (!create_conversion_pipeline())
    {
        tcam_log(TCAM_LOG_ERROR, "Unable to determine conversion pipeline.");
        return false;
    }

    if (!source->setVideoFormat(input_format))
    {
        tcam_log(TCAM_LOG_ERROR, "Unable to set video format in source.");
        return false;
    }

    if (!sink->setVideoFormat(output_format))
    {
        tcam_log(TCAM_LOG_ERROR, "Unable to set video format in sink.");
        return false;
    }

    if (!source->set_buffer_collection(sink->get_buffer_collection()))
    {
        tcam_log(TCAM_LOG_ERROR, "Unable to set buffer collection.");
        return false;
    }

    tcam_log(TCAM_LOG_INFO, "Pipeline creation successful.");

    std::string ppl = "source -> ";

    for (const auto& f : filter_pipeline)
    {
        ppl += f->getDescription().name;
        ppl += " -> ";
    }
    ppl += " sink";
    tcam_log(TCAM_LOG_INFO, "%s" , ppl.c_str());

    return true;
}


bool PipelineManager::start_playing ()
{

    if (!set_sink_status(TCAM_PIPELINE_PLAYING))
    {
        tcam_log(TCAM_LOG_ERROR, "Sink refused to change to state PLAYING");
        goto error;
    }

    if (!set_source_status(TCAM_PIPELINE_PLAYING))
    {
        tcam_log(TCAM_LOG_ERROR, "Source refused to change to state PLAYING");
        goto error;
    }

    status = TCAM_PIPELINE_PLAYING;

    return true;

error:
    stop_playing();
    return false;
}


bool PipelineManager::stop_playing ()
{
    status = TCAM_PIPELINE_STOPPED;

    if (!set_source_status(TCAM_PIPELINE_STOPPED))
    {
        tcam_log(TCAM_LOG_ERROR, "Source refused to change to state STOP");
        return false;
    }

    for (auto& f : filter_pipeline)
    {
        if (!f->setStatus(TCAM_PIPELINE_STOPPED))
        {
            tcam_log(TCAM_LOG_ERROR,
                    "Filter %s refused to change to state STOP",
                    f->getDescription().name.c_str());
            return false;
        }
    }

    set_sink_status(TCAM_PIPELINE_STOPPED);

    //destroyPipeline();

    return true;
}


void PipelineManager::push_image (std::shared_ptr<MemoryBuffer> buffer)
{
    if (status == TCAM_PIPELINE_STOPPED)
    {
        return;
    }

    auto& current_buffer = buffer;

    for (auto& f : filter_pipeline)
    {
        if (f->getDescription().type == FILTER_TYPE_INTERPRET)
        {
            f->apply(current_buffer);
        }
        else if (f->getDescription().type == FILTER_TYPE_CONVERSION)
        {
            auto next_buffer = pipeline_buffer.at(current_ppl_buffer);

            next_buffer->set_statistics(current_buffer->get_statistics());

            f->transform(*current_buffer, *next_buffer);

            current_buffer = next_buffer;

            current_ppl_buffer++;
            if (current_ppl_buffer == pipeline_buffer.size())
                current_ppl_buffer = 0;
        }
    }


    if (sink != nullptr)
    {
        sink->push_image(current_buffer);
    }
    else
    {
      tcam_log(TCAM_LOG_ERROR, "Sink is NULL");
    }
}


std::vector<std::shared_ptr<MemoryBuffer>> PipelineManager::get_buffer_collection ()
{
    return std::vector<std::shared_ptr<MemoryBuffer>>();
}
