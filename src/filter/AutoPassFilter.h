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

#ifndef TCAM_AUTOPASSFILTER_H
#define TCAM_AUTOPASSFILTER_H

#include "tcam_algorithms.h"

#include <FilterBase.h>

#include "compiler_defines.h"

#include <vector>

VISIBILITY_INTERNAL

// forward declaration
class AutoPassFilter;

class AutoPassPropertyHandler: public PropertyImpl
{
public:

    friend class AutoPassFilter;

    AutoPassPropertyHandler (AutoPassFilter*);

    bool setProperty (const Property&);

    bool getProperty (Property&);

protected:

    AutoPassFilter* filter;

    // device properties
    std::weak_ptr<PropertyInteger> property_exposure;
    std::weak_ptr<PropertyInteger> property_gain;
    std::weak_ptr<PropertyInteger> property_iris;
    std::weak_ptr<PropertyInteger> property_focus;

    // generated properties
    std::shared_ptr<PropertyBoolean> prop_auto_exposure;
    std::shared_ptr<PropertyBoolean> prop_auto_gain;
    std::shared_ptr<PropertyBoolean> prop_auto_iris;

    std::shared_ptr<PropertyButton> focus_onepush;

    std::shared_ptr<PropertyBoolean> prop_wb;
    std::shared_ptr<PropertyBoolean> prop_auto_wb;
    std::shared_ptr<PropertyInteger> prop_wb_r;
    std::shared_ptr<PropertyInteger> prop_wb_g;
    std::shared_ptr<PropertyInteger> prop_wb_b;


};


class AutoPassFilter : public FilterBase
{
public:

    AutoPassFilter ();

    void reset ();

    struct FilterDescription getDescription () const;

    bool transform (MemoryBuffer& in, MemoryBuffer& out );

    bool apply (std::shared_ptr<MemoryBuffer>);

    bool setStatus (TCAM_PIPELINE_STATUS);
    TCAM_PIPELINE_STATUS getStatus () const;

    void getVideoFormat (VideoFormat& in, VideoFormat& out) const;

    bool setVideoFormat(const VideoFormat& in, const VideoFormat& out);

    void setDeviceProperties (std::vector<std::shared_ptr<Property>>);

    std::vector<std::shared_ptr<Property>> getFilterProperties ();

    void activate_focus_run ();

private:

    void update_params ();

    void set_gain     (int);
    void set_exposure (int);
    void set_iris     (int);
    void set_focus    (int);

    unsigned int calculate_exposure_max ();

    static const unsigned char default_color_value = 64;

    // general
    bool valid;

    int		wb_r;
    int		wb_g;
    int		wb_b;
    unsigned int exposure_max;

    FilterDescription description;
    TCAM_PIPELINE_STATUS current_status;
    tcam::VideoFormat input_format;

    tcam_auto_alg_params params;

    std::vector<unsigned char> context;
    tcam_auto_alg_context state;


    struct tcam_create_auto_alg_params init_params;

    std::shared_ptr<AutoPassPropertyHandler> handler;


};


extern "C"
{

    // FilterBase* create ();
    FB* create ();

    void close (FB*);

}

VISIBILITY_POP

#endif /* TCAM_AUTOPASSFILTER_H */
