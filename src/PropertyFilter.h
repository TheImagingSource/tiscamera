#pragma once

#include "PropertyInterfaces.h"
#include "compiler_defines.h"

#include <memory>
#include <vector>

VISIBILITY_INTERNAL

namespace tcam
{
class VideoFormat;
class VideoFormatDescription;
class ImageBuffer;
}

namespace tcam::property
{
class SoftwareProperties;
}

namespace tcam::stream::filter
{

class SoftwarePropertyWrapper
{
public:
    SoftwarePropertyWrapper() = default;

    void    setup(
        const std::vector<std::shared_ptr<tcam::property::IPropertyBase>>& props,
        const std::vector<VideoFormatDescription>& device_formats);

    void apply(ImageBuffer&);

    void setVideoFormat(const VideoFormat& in);

    std::vector<std::shared_ptr<tcam::property::IPropertyBase>> getProperties();

private:
    std::shared_ptr<tcam::property::SoftwareProperties> m_impl;
};


} // namespace tcam::stream::filter

VISIBILITY_POP
