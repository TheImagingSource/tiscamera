

#include "BayerRgbFilter.h"

#include <by8/by8torgb_conv.h>
#include <dutils_header.h>

#include <type_traits>
#include <algorithm>

#include <iostream>

using namespace tis_imaging;

BayerRgbFilter::BayerRgbFilter ()
    : status(PIPELINE_UNDEFINED)
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

BayerRgbFilter::~BayerRgbFilter ()
{}


struct FilterDescription BayerRgbFilter::getDescription () const
{
    return description;
}


bool BayerRgbFilter::transform (MemoryBuffer& in, MemoryBuffer& out)
{

    // debayer_bayer_rgb24(in.getData(), out.getData())

// std::remove_const<MemoryBuffer>::type in;

    auto img_out = to_img_desc(out);
    auto img_in  = to_img_desc(in);
    
    by8_transform::transform_by8_options options = {};

    options.options = 0;
    options.opt_level = 0;

    bool hflip = true;
    
    by8_transform::transform_by8_to_dest(img_out, img_in, hflip, options);
    
    return true;
}


bool BayerRgbFilter::apply (std::shared_ptr<MemoryBuffer>)
{
    return false;
}


bool BayerRgbFilter::setStatus (const PIPELINE_STATUS& s)
{
    if (status == s)
    {
        return true;
    }

    status = s;

    return true;
    
}


PIPELINE_STATUS BayerRgbFilter::getStatus () const
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

    std::cout << "OUTPUT FOURCC IS : " << output_format.getFourcc() << std::endl;

    
    return true;
}


void BayerRgbFilter::setDeviceProperties (std::vector<std::shared_ptr<Property>>)
{}


std::vector<std::shared_ptr<Property>> BayerRgbFilter::getFilterProperties ()
{
    return std::vector<std::shared_ptr<Property>>();
}


FB* create ()
{
    return (FB*)new BayerRgbFilter();
}


void destroy (FB* filter)
{
    delete reinterpret_cast<BayerRgbFilter*>( filter);
}

