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

#ifndef TCAM_FILTERBASE_H
#define TCAM_FILTERBASE_H

#include "ImageBuffer.h"
#include "PropertyInterfaces.h"
#include "VideoFormatDescription.h"
#include "base_types.h"

#include <memory>
#include <string>
#include <vector>

namespace tcam
{

enum class FILTER_TYPE
{
    FILTER_TYPE_UNKNOWN = 0,
    FILTER_TYPE_CONVERSION, /* used for static transformations (e.g. colorspace conversions) */
    FILTER_TYPE_INTERPRET, /* image interpretation / manipulation from device properties */
};


struct FilterDescription
{
    std::string name;
    FILTER_TYPE type;

    // 0 == all types
    std::vector<uint32_t> output_fourcc;
    std::vector<uint32_t> input_fourcc;
};


class FilterBase
{
public:
    virtual ~FilterBase() {};

    virtual struct FilterDescription getDescription() const = 0;

    virtual bool transform(ImageBuffer& in, ImageBuffer& out) = 0;

    virtual bool apply(std::shared_ptr<ImageBuffer>) = 0;

    virtual bool setStatus(TCAM_PIPELINE_STATUS) = 0;

    virtual TCAM_PIPELINE_STATUS getStatus() const = 0;

    virtual bool setVideoFormat(const VideoFormat& in, const VideoFormat& out) = 0;
    virtual void getVideoFormat(VideoFormat& in, VideoFormat& out) const = 0;

    virtual std::vector<std::shared_ptr<tcam::property::IPropertyBase>> getProperties() = 0;
};

} /* namespace tcam */

#endif /* TCAM_FILTERBASE_H */
