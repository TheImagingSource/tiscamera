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

#ifndef TCAM_ALGORITHM_PARALLEL_H
#define TCAM_ALGORITHM_PARALLEL_H

#include "tcam.h"
#include "work_pool.h"
#include "std_latch.h"

using namespace tcam::algorithms::work_pool;

namespace tcam
{

namespace algorithms
{

namespace parallel
{

/**
 * @param img_to_split
 * @param idx_to_generate
 * @param split_height
 * @param is_last_item
 */
tcam_image_buffer split_image_buffer (const tcam_image_buffer& img_to_split,
                                      int idx_to_generate,
                                      unsigned int split_height,
                                      bool is_last_item);

/**
 * @param image_height
 * @param suggested_split_count
 */
unsigned int calc_split_height (unsigned int image_height,
                                int& suggested_split_count);

static const int max_work_item_count = 32;

struct func_caller
{
    virtual void call (const tcam_image_buffer& dst,
                       const tcam_image_buffer& src) = 0;
};

class parallel_state
{
public:
    parallel_state ();
    ~parallel_state ();

    int get_default_concurrency ();

    void queue_and_wait (func_caller* func,
                         tcam_image_buffer dst,
                         tcam_image_buffer src,
                         int max_conc);

private:

    work_pool::work_pool* thread_pool_ref_ = nullptr;
    threading::latch img_ready_latch_;

    void split_image_context() {};

    func_caller* func;

    struct split_image_context : work_pool::work_context
    {
        split_image_context() {}

        func_caller* func;

        threading::latch* shared_latch;

        tcam_image_buffer split_dst;
        tcam_image_buffer split_src;

        virtual void do_one () final;
    };

    void construct_from_split_ (func_caller* func,
                                tcam_image_buffer& dst,
                                tcam_image_buffer& src,
                                int split_count,
                                int split_height);

    struct split_image_context work_item_space_[max_work_item_count];


}; // class parallel_state

} // namespace parallel

} // namespace algorithms

} // namespace tcam

#endif /* TCAM_ALGORITHM_PARALLEL_H */
