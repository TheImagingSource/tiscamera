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

#ifndef TCAM_PIPELINEMANAGER_H
#define TCAM_PIPELINEMANAGER_H

#include "compiler_defines.h"
#include "base_types.h"
#include "ImageSource.h"
#include "ImageSink.h"
#include "FilterBase.h"

#include <memory>

VISIBILITY_INTERNAL

namespace tcam
{

class PipelineManager : public SinkInterface, public std::enable_shared_from_this<PipelineManager>
{
public:

    PipelineManager ();

    ~PipelineManager ();

    /**
     * @return vector containing all videoformats the pipeline can create
     */
    std::vector<VideoFormatDescription> getAvailableVideoFormats () const;

    /**
     * @brief informs all filter about new format and checks if a valid pipeline can be created
     * @return true is device and pipeline allow format; else false
     */
    bool setVideoFormat (const VideoFormat&);

    VideoFormat getVideoFormat () const;

    /**
     *
     * @return vector containing all filter properties; empty on error
     */
    std::vector<std::shared_ptr<Property>> getFilterProperties ();

    bool set_status (TCAM_PIPELINE_STATUS);

    /**
     * @return TCAM_PIPELINE_STATUS the pipeline currently has
     */
    TCAM_PIPELINE_STATUS get_status () const;

    /**
     * @brief Reset the pipeline to PIPELINE_STOPPED.
     * Stop all streams. Destroys all buffer.
     * @return true on success
     */
    bool destroyPipeline ();

    /**
     * @brief Define the device the pipeline shall use
     * @return true on success
     */
    bool setSource (std::shared_ptr<DeviceInterface>);

    std::shared_ptr<ImageSource> getSource ();

    bool setSink (std::shared_ptr<SinkInterface>);

    std::shared_ptr<SinkInterface> getSink ();

    // @brief callback for ImageSource
    void push_image (std::shared_ptr<MemoryBuffer>);

    std::vector<std::shared_ptr<MemoryBuffer>> get_buffer_collection ();

private:

    VideoFormat output_format;
    VideoFormat input_format;

    std::vector<VideoFormatDescription> available_input_formats;
    std::vector<VideoFormatDescription> available_output_formats;

    TCAM_PIPELINE_STATUS status;

    std::shared_ptr<ImageSource> source;
    std::shared_ptr<SinkInterface> sink;

    // @brief device properties
    std::vector<std::shared_ptr<Property>> device_properties;

    std::vector<std::shared_ptr<Property>> filter_properties;

    /**
     * @brief collection of found filter
     */
    std::vector<std::shared_ptr<FilterBase>> available_filter;

    /**
     * @brief Filter that are used in the actual pipeline
     */
    std::vector<std::shared_ptr<FilterBase>> filter_pipeline;

    /**
     * @brief Additional buffer used for internal image copies
     */
    std::vector<std::shared_ptr<MemoryBuffer>> pipeline_buffer;
    unsigned int current_ppl_buffer;

    void distributeProperties ();

    void create_input_format (uint32_t fourcc);

    std::vector<uint32_t> getDeviceFourcc ();

    bool set_source_status (TCAM_PIPELINE_STATUS status);

    bool set_sink_status (TCAM_PIPELINE_STATUS);

    bool validate_pipeline ();

    bool create_conversion_pipeline ();

    bool add_interpretation_filter ();

    bool allocate_conversion_buffer ();

    bool create_pipeline ();

    bool start_playing ();

    bool stop_playing ();

};

} /* namespace tcam */

VISIBILITY_POP

#endif /* TCAM_PIPELINEMANAGER_H */
