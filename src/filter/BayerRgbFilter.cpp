

#include "BayerRgbFilter.h"

#include "tcam_algorithms.h"

#include "logging.h"

#include <type_traits>
#include <algorithm>

#include <iostream>

using namespace tcam;

BayerRgbFilter::BayerRgbFilter ()
    : status(TCAM_PIPELINE_UNDEFINED)
{
    description.name = "bayer2rgb";
    description.type = FILTER_TYPE_CONVERSION;
    description.input_fourcc = {FOURCC_GBRG8,
                                FOURCC_RGGB8,
                                FOURCC_BGGR8,
                                FOURCC_GRBG8};
    description.output_fourcc = {FOURCC_RGB32,
                                 FOURCC_RGB24};
}


struct FilterDescription BayerRgbFilter::getDescription () const
{
    return description;
}


bool BayerRgbFilter::transform (MemoryBuffer& in, MemoryBuffer& out)
{

    auto i = in.getImageBuffer();
    auto o = out.getImageBuffer();

    convert_to_format(&i, &o);

    return true;
}


bool BayerRgbFilter::apply (std::shared_ptr<MemoryBuffer>)
{
    return false;
}


bool BayerRgbFilter::setStatus (TCAM_PIPELINE_STATUS s)
{
    if (status == s)
    {
        return true;
    }

    status = s;

    return true;

}


TCAM_PIPELINE_STATUS BayerRgbFilter::getStatus () const
{
    return status;
}


void BayerRgbFilter::getVideoFormat (VideoFormat& in, VideoFormat& out) const
{
    in = input_format;
    out = output_format;
}


bool BayerRgbFilter::setVideoFormat (const VideoFormat& in, const VideoFormat& out)
{
    input_format = in;
    output_format = out;

    auto res = std::find(description.input_fourcc.begin(),
                         description.input_fourcc.end(),
                         input_format.getFourcc());

    if (res == description.input_fourcc.end())
    {
        return false;
    }

    res = std::find(description.output_fourcc.begin(),
                    description.output_fourcc.end(),
                    output_format.getFourcc());

    if (res == description.output_fourcc.end())
    {
        return false;
    }

    tcam_log(TCAM_LOG_DEBUG,
             "BayerRgb in:'%s' != out:'%s'",
             input_format.toString().c_str(),
             output_format.toString().c_str());

    return true;
}


void BayerRgbFilter::setDeviceProperties (std::vector<std::shared_ptr<Property>>)
{}


std::vector<std::shared_ptr<Property>> BayerRgbFilter::getFilterProperties ()
{
    return std::vector<std::shared_ptr<Property>>();
}
