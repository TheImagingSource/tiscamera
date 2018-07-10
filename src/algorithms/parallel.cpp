/*
 * Copyright 2018 The Imaging Source Europe GmbH
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

#include "parallel.h"

#include "utils.h"

using namespace tcam::algorithms::parallel;


tcam_image_buffer tcam::algorithms::parallel::split_image_buffer (const tcam_image_buffer& img_to_split,
                                                                  int idx_to_generate,
                                                                  unsigned int split_height,
                                                                  bool is_last_item)
{
    unsigned int pitch = get_pitch_length(img_to_split.format.width, img_to_split.format.fourcc);

    unsigned char* tile_start_ptr = img_to_split.pData + pitch * idx_to_generate * split_height ;

    unsigned int height_for_item = split_height;

    if (is_last_item)
    {
        // give the last item the remaining lines so nothing is missed
        height_for_item = img_to_split.format.height - idx_to_generate * split_height;
    }

    tcam_image_buffer ret = img_to_split;

    ret.pData = tile_start_ptr;
    ret.format.height = height_for_item;

    return ret;
}


unsigned int tcam::algorithms::parallel::calc_split_height (unsigned int image_height,
                                                            int& suggested_split_count)
{
    if (suggested_split_count <= 1)
    {
        return 0;
    }

    if (image_height < 128)
    {
        return 0;
    }

    if ( (image_height / suggested_split_count) < 64)
    {
        suggested_split_count = (image_height / 64) + 1;
    }

    while (suggested_split_count > 1)
    {
        int single_height = image_height / suggested_split_count;
        if (single_height % 4)
        {
            single_height -= single_height % 4;
        }

        if (single_height >= 64)
        {
            return single_height;
        }

        --suggested_split_count;
    }
    return 0;
}



tcam::algorithms::parallel::parallel_state::parallel_state ()
    : thread_pool_ref_(acquire_default_work_pool())
{
    for( auto& item : work_item_space_ )
    {
        item.func = nullptr;
        item.shared_latch = &img_ready_latch_;
    }
}


tcam::algorithms::parallel::parallel_state::~parallel_state ()
{
    release_default_work_pool(thread_pool_ref_);
}


static int default_concurrency = 0;

int tcam::algorithms::parallel::parallel_state::get_default_concurrency ()
{
    if (default_concurrency == 0)
    {
        default_concurrency = work_pool::get_logical_cpu_count();
    }

    return default_concurrency;
}


void tcam::algorithms::parallel::parallel_state::queue_and_wait (func_caller* func,
                                                                 tcam_image_buffer dst,
                                                                 tcam_image_buffer src,
                                                                 int max_conc)
{
    if (max_conc == 0)
    {
        max_conc = get_default_concurrency();
    }

    int split_count = std::min(max_work_item_count, max_conc);

    int split_height = calc_split_height(src.format.height, split_count);

    if (split_height == 0)
    {
        func->call(dst, src);
        return;
    }

    construct_from_split_(func, dst, src, split_count, split_height);

    thread_pool_ref_->queue_items_and_wait(img_ready_latch_,
                                           work_item_space_,
                                           split_count);
}


void tcam::algorithms::parallel::parallel_state::split_image_context::do_one ()
{
    func->call(split_dst, split_src);
    shared_latch->count_down();
}


void tcam::algorithms::parallel::parallel_state::construct_from_split_ (func_caller* func,
                                                                        tcam_image_buffer& dst,
                                                                        tcam_image_buffer& src,
                                                                        int split_count,
                                                                        int split_height)
{
    for (int index = 0; index < split_count; ++index)
    {
        bool is_last_item = index == (split_count - 1);

        auto& item = work_item_space_[index];
        item.func = func;
        item.split_dst = split_image_buffer(dst, index, split_height, is_last_item);
        item.split_src = split_image_buffer(src, index, split_height, is_last_item);
    }
}
