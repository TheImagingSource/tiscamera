
#include "PropertyFilter.h"

#include "logging.h"

#include <chrono>
#include <dutils_img/image_fourcc_func.h>

namespace tcam::stream::filter
{

PropertyFilter::PropertyFilter(
    const std::vector<std::shared_ptr<tcam::property::IPropertyBase>>& props,
    const std::vector<VideoFormatDescription>& device_formats)
{
    bool has_bayer = false;
    for (const auto& format : device_formats)
    {
        auto fcc = (img::fourcc)format.get_fourcc();
        if (img::is_by8_fcc(fcc) || img::is_by10_packed_fcc(fcc) || img::is_by12_packed_fcc(fcc)
            || img::is_by16_fcc(fcc) || img::is_by12_fcc(fcc) || img::is_by10_fcc(fcc)
            || img::is_byfloat_fcc(fcc))
        {
            has_bayer = true;
            break;
        }
    }
    m_impl = std::make_shared<tcam::property::SoftwareProperties>(props, has_bayer);
}


bool PropertyFilter::apply(std::shared_ptr<ImageBuffer> buffer)
{
    auto bs = buffer->getImageBuffer();

    img::dim dim = {};
    dim.cx = m_format.get_size().width;
    dim.cy = m_format.get_size().height;

    // TODO: generate from VideoFormat
    img::img_type src_type = { bs.format.fourcc, dim, (int)bs.length };

    img::img_descriptor src =
        img::make_img_desc_raw(src_type, img::img_plane { buffer->get_data(), (int)bs.pitch });

    m_impl->auto_pass(src);

    return true;
}


bool PropertyFilter::setStatus(TCAM_PIPELINE_STATUS new_state)
{
    m_pipeline_state = new_state;
    return true;
}


TCAM_PIPELINE_STATUS PropertyFilter::getStatus() const
{
    return m_pipeline_state;
}


bool PropertyFilter::setVideoFormat(const VideoFormat& in, const VideoFormat& /* out */)
{
    m_format = in;

    m_impl->update_to_new_format(in);

    return true;
}


void PropertyFilter::getVideoFormat(VideoFormat& in, VideoFormat& out) const
{
    in = m_format;
    out = m_format;
}


std::vector<std::shared_ptr<tcam::property::IPropertyBase>> PropertyFilter::getProperties()
{
    return m_impl->get_properties();
}

} // namespace tcam::stream::filter
