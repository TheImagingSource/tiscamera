



#ifndef PIPELINEMANAGER_H_
#define PIPELINEMANAGER_H_

#include "base_types.h"
#include "ImageSource.h"
#include "ImageSink.h"
#include "FilterBase.h"

#include "FilterLoader.h"

#include <memory>
#include <ctime>

namespace tis_imaging
{

class PipelineManager : public SinkInterface, public std::enable_shared_from_this<PipelineManager>
{
public:

    PipelineManager ();

    ~PipelineManager ();


    std::vector<VideoFormatDescription> getAvailableVideoFormats () const;

    /**
     * @brief informs all filter about new format and checks if a valid pipeline can be created
     * @return true is device and pipeline allow format; else false
     */
    bool setVideoFormat (const VideoFormat&);

    /**
     *
     * @return vector containing all filter properties; empty on error
     */
    std::vector<std::shared_ptr<Property>> getFilterProperties ();
    
    bool setStatus (const PIPELINE_STATUS&);
    
    PIPELINE_STATUS getStatus () const;

    /**
     * @brief Reset the pipeline to an undefined state.
     * Stop all streams.
     * @return true on success
     */
    bool destroyPipeline ();

    bool setSource (std::shared_ptr<DeviceInterface>);

    std::shared_ptr<ImageSource> getSource ();
    
    bool setSink (std::shared_ptr<ImageSink>);
    
    std::shared_ptr<ImageSink> setSink ();

    // @brief callback for ImageSource
    void pushImage (std::shared_ptr<MemoryBuffer>);
    
private:

    unsigned long long frame_count;
    time_t second_count;
    
    
    VideoFormat format;
    VideoFormat input_format;

    std::vector<VideoFormatDescription> available_input_formats;
    std::vector<VideoFormatDescription> available_output_formats;

    
    PIPELINE_STATUS status;
    
    std::shared_ptr<ImageSource> source;
    std::shared_ptr<ImageSink> sink;

    // @brief device properties
    std::vector<std::shared_ptr<Property>> properties;

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


    /**
     * Informs all available transformation filter about the available
     * device formats to create a list about indirectly available formats
     *
     * requires defined source
     */
    void index_output_formats ();
    
    void distributeProperties ();

    void create_input_format (const uint32_t& fourcc);

    std::vector<uint32_t> getDeviceFourcc ();

    bool validate_pipeline ();

    bool create_conversion_pipeline ();

    bool add_interpretation_filter ();

    bool allocate_conversion_buffer ();
    
    bool create_pipeline ();

    bool start_playing ();

    bool stop_playing ();

    FilterLoader filter_loader;

};

} /* namespace tis_imaging */

#endif /* PIPELINEMANAGER_H_ */


