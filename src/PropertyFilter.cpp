
#include "PropertyFilter.h"

#include "ImageBuffer.h"
#include "SoftwareProperties.h"
#include "VideoFormatDescription.h"
#include "logging.h"

#include <dutils_img/image_fourcc_func.h>

namespace tcam::stream::filter
{

static bool has_bayer_format(const std::vector<VideoFormatDescription>& device_formats)
{
    for (const auto& format : device_formats)
    {
        auto fcc = (img::fourcc)format.get_fourcc();
        if (img::is_by8_fcc(fcc) || img::is_by10_packed_fcc(fcc) || img::is_by12_packed_fcc(fcc)
            || img::is_by16_fcc(fcc) || img::is_by12_fcc(fcc) || img::is_by10_fcc(fcc)
            || img::is_byfloat_fcc(fcc))
        {
            return true;
        }
    }
    return false;
}

void SoftwarePropertyWrapper::setup(
    const std::vector<std::shared_ptr<tcam::property::IPropertyBase>>& props,
    const std::vector<VideoFormatDescription>& device_formats)
{
    bool has_bayer = has_bayer_format(device_formats);
    m_impl = tcam::property::SoftwareProperties::create(props, has_bayer);
}


void SoftwarePropertyWrapper::apply(ImageBuffer& buffer)
{
    img::img_descriptor src = buffer.get_img_descriptor();

    m_impl->auto_pass(src);
}

void SoftwarePropertyWrapper::setVideoFormat(const VideoFormat& in)
{
    m_impl->update_to_new_format(in);
}

std::vector<std::shared_ptr<tcam::property::IPropertyBase>> SoftwarePropertyWrapper::getProperties()
{
    return m_impl->get_properties();
}

} // namespace tcam::stream::filter
