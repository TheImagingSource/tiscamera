#pragma once


#include "FilterBase.h"
#include "SoftwareProperties.h"
#include "compiler_defines.h"

#include <memory>

VISIBILITY_INTERNAL

namespace tcam::stream::filter
{

class PropertyFilter : public FilterBase
{
public:
    explicit PropertyFilter(
        const std::vector<std::shared_ptr<tcam::property::IPropertyBase>>& props,
        const std::vector<VideoFormatDescription>& device_formats);

    virtual struct FilterDescription getDescription() const final
    {
        return m_description;
    }

    virtual bool transform(ImageBuffer& /*in*/, ImageBuffer& /*out*/) final
    {
        return false;
    }

    virtual bool apply(std::shared_ptr<ImageBuffer>) final;

    virtual bool setStatus(TCAM_PIPELINE_STATUS) final;

    virtual TCAM_PIPELINE_STATUS getStatus() const final;

    virtual bool setVideoFormat(const VideoFormat& in, const VideoFormat& out) final;
    virtual void getVideoFormat(VideoFormat& in, VideoFormat& out) const final;

    virtual std::vector<std::shared_ptr<tcam::property::IPropertyBase>> getProperties() final;

private:
    FilterDescription m_description = { "PropertyFilter",
                                        FILTER_TYPE::FILTER_TYPE_INTERPRET,
                                        { 0 },
                                        { 0 } };

    std::shared_ptr<tcam::property::SoftwareProperties> m_impl;

    TCAM_PIPELINE_STATUS m_pipeline_state = TCAM_PIPELINE_UNDEFINED;

    VideoFormat m_format;
};


} // namespace tcam::stream::filter

VISIBILITY_POP
