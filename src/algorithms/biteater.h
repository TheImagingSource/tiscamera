/*
 * Copyright 2017 The Imaging Source Europe GmbH
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

#ifndef TCAM_ALGORITHMS_BITEATER_H
#define TCAM_ALGORITHMS_BITEATER_H

#include "parallel.h"
#include "tcam.h"

#include <memory>

using namespace tcam::algorithms;

namespace tcam
{

namespace biteater
{

struct offsets
{
    unsigned char r;
    unsigned char g;
    unsigned char b;
    unsigned char a;

    bool empty() const
    {
        if (r == 0 && g == 0 && b == 0 && a == 0)
        {
            return true;
        }
        return false;
    }
};


struct biteater_meta
{
    offsets offset_in;
    offsets offset_out;
    std::shared_ptr<parallel::parallel_state> para;
};


struct para_callback : parallel::func_caller
{
    void call(const tcam_image_buffer& dst, const tcam_image_buffer& src);
};


struct offsets offsets_for_fourcc(unsigned int fourcc);


bool init_meta(struct biteater_meta& meta,
               const tcam_video_format& in,
               const tcam_video_format& out);


bool transform(const struct tcam_image_buffer* image_in,
               struct tcam_image_buffer* image_out,
               const struct biteater_meta& meta);

} // namespace biteater

} // namespace tcam

#endif /* TCAM_ALGORITHMS_BITEATER_H */
