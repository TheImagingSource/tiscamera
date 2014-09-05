
#include "PipelineManager.h"

#include "tis_logging.h"

#include <ctime>
#include <cstring>
#include <algorithm>

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


bool PipelineManager::setPipelineStatus (const PIPELINE_STATUS& s)
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


PIPELINE_STATUS PipelineManager::getPipelineStatus () const
{
    return status;
}


bool PipelineManager::destroyPipeline ()
{
    setPipelineStatus(PIPELINE_STOPPED);

    source = nullptr;
    sink = nullptr;

    return true;
}


const char* fourcc2description (const uint32_t& fourcc)
{
    switch (fourcc)
    {
        // these are pseudo fourcc's used in the library to signify the formats
        case FOURCC_RGB8:
			return "RGB8";
        case FOURCC_RGB24:
            return "RGB24";
        case FOURCC_RGB32:
            return "RGB32";
        case FOURCC_RGB64:
            return "RGB64";

        case FOURCC_YUY2:
			return "YUY2";
        case FOURCC_Y800:
			return "Y800";
        case FOURCC_BY8:
			return "BY8";
        case FOURCC_UYVY:
			return "UYVY";

        case FOURCC_YGB0:
			return "YGB0";
        case FOURCC_YGB1:
			return "YGB1";
        case FOURCC_Y16:
			return "Y16";

        case FOURCC_Y444:
			return "Y444";  // TIYUV: 1394 conferencing camera 4:4:4 mode 0, 24 bit
        case FOURCC_Y411:
			return "Y411";  // TIYUV: 1394 conferencing camera 4:1:1 mode 2, 12 bit
        // case FOURCC_B800:
			// return "BY8";  
        // case FOURCC_Y422:
		// 	FOURCC_UYVY

        case FOURCC_BGGR8:
            return "BA81"; /*  8  BGBG.. GRGR.. */
        case FOURCC_GBRG8:
            return "GBRG"; /*  8  GBGB.. RGRG.. */
        case FOURCC_GRBG8:
            return "GRBG"; /*  8  GRGR.. BGBG.. */
        case FOURCC_RGGB8:
            return "RGGB"; /*  8  RGRG.. GBGB.. */

        case FOURCC_BGGR10:
            return "BG10"; /* 10  BGBG.. GRGR.. */
        case FOURCC_GBRG10:
            return "GB10"; /* 10  GBGB.. RGRG.. */
        case FOURCC_GRBG10:
            return "BA10"; /* 10  GRGR.. BGBG.. */
        case FOURCC_RGGB10:
            return "RG10"; /* 10  RGRG.. GBGB.. */

        case FOURCC_BGGR12:
            return "BG12"; /* 12  BGBG.. GRGR.. */
        case FOURCC_GBRG12:
            return "GB12"; /* 12  GBGB.. RGRG.. */
        case FOURCC_GRBG12:
            return "BA12"; /* 12  GRGR.. BGBG.. */
        case FOURCC_RGGB12:
            return "RG12"; /* 12  RGRG.. GBGB.. */

        case FOURCC_BGGR16:
            return "BG16"; /* 16  BGBG.. GRGR.. */
        case FOURCC_GBRG16:
            return "GB16"; /* 16  GBGB.. RGRG.. */
        case FOURCC_GRBG16:
            return "BA16"; /* 16  GRGR.. BGBG.. */
        case FOURCC_RGGB16:
            return "RG16"; /* 16  RGRG.. GBGB.. */

        case FOURCC_I420:
			return "I420"; 
        case FOURCC_YV16:
			return "YV16";		// YUV planar Y plane, U plane, V plane. U and V sub sampled in horz 
//case FOURCC_YV12			return "Y', 'V', '1', '2"; 
        case FOURCC_YUV8PLANAR:
            return "YUV8 planar";		// unofficial, YUV planar, Y U V planes, all 8 bit, no sub-sampling
        case FOURCC_YUV16PLANAR:
            return "YUV16 planar";		// unofficial, YUV planar, Y U V planes, all 16 bit, no sub-sampling
        case FOURCC_YUV8:
			return "YUV8";      // 8 bit, U Y V _ ordering, 32 bit

        case FOURCC_H264:
            return "H264";
        case FOURCC_MJPG:
            return "MJPG";
        default:
            return "";
    }
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
            tis_log(TIS_LOG_ERROR, "Passing format through" );
            available_output_formats.push_back(*match);
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


bool PipelineManager::setSink (std::shared_ptr<ImageSink> s)
{
    if (status == PIPELINE_PLAYING || status == PIPELINE_PAUSED)
    {
        return false;
    }

    this->sink = s;

    return true;
}


std::shared_ptr<ImageSink> PipelineManager::setSink ()
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


bool PipelineManager::create_pipeline ()
{
    if (source.get() == nullptr || sink.get() == nullptr)
    {
        // TODO: error
        return false;
    }

    // apply conversion to receive a pipeline that has valid input/output videoformats

    for (auto f : available_filter)
    {
        f->setVideoFormat(format);
    }

    filter_pipeline = available_filter;

    // if a valid pipeline can be created insert additional filter (e.g. autoexposure)

    for (int i = 0; i < 10; ++i)
    {

        image_buffer b = {};
        b.pitch = format.getSize().width * 4;
        b.length = b.pitch * format.getSize().height;

        b.pData = (unsigned char*)malloc(b.length);

        b.format.fourcc    = FOURCC_RGB32;
        b.format.width     = format.getSize().width;
        b.format.height    = format.getSize().height;
        b.format.binning   = format.getBinning();
        b.format.framerate = format.getFramerate();

        this->pipeline_buffer.push_back(std::make_shared<MemoryBuffer>(b));
    }

    tis_log(TIS_LOG_ERROR, "filter in use %d of %d", filter_pipeline.size(), available_filter.size());

    // check if pipeline is valid


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

    status = PIPELINE_STOPPED;

    return true;
}


void PipelineManager::pushImage (std::shared_ptr<MemoryBuffer> buffer)
{
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
