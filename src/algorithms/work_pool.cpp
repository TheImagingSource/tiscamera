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

#include "work_pool.h"

using namespace tcam::algorithms;


static std::mutex g_pool_lock;
static work_pool::work_pool* g_default_pool = nullptr;
static long g_pool_ref_count = 0;


int tcam::algorithms::work_pool::get_logical_cpu_count ()
{
    return (int) std::thread::hardware_concurrency();
}


work_pool::work_pool* work_pool::acquire_default_work_pool ()
{
    std::lock_guard<std::mutex> lck(g_pool_lock);

    if (g_default_pool == nullptr)
    {
        g_default_pool = new work_pool;
        if (g_default_pool != nullptr)
        {
            if (!g_default_pool->start())
            {
                delete g_default_pool;
                g_default_pool = nullptr;
            }
        }
    }
    if (g_default_pool != nullptr)
    {
        ++g_pool_ref_count;
    }

    return g_default_pool;
}


void work_pool::release_default_work_pool (work_pool* pool)
{
    std::lock_guard<std::mutex> lck(g_pool_lock);

    if (--g_pool_ref_count == 0)
    {
        delete g_default_pool;
        g_default_pool = nullptr;
    }
}


tcam::algorithms::work_pool::work_pool::work_pool ()
    : ended_flag_(false)
{}


tcam::algorithms::work_pool::work_pool::~work_pool ()
{
    stop();
}


bool tcam::algorithms::work_pool::work_pool::start ()
{
    ended_flag_ = false;

    int num_worker_threads = get_logical_cpu_count() - 1;

    thread_list_.reserve(num_worker_threads);

    for (unsigned int index = 0; index < num_worker_threads; ++index)
    {
        thread_list_.emplace_back( [this, index] {worker_thread_function(index); } );
    }
    vec_.reserve( 64 );

    return true;
}


void tcam::algorithms::work_pool::work_pool::stop ()
{
    ended_flag_ = true;

    sem_.up( (int) thread_list_.size() );
    for( auto&& thrd : thread_list_ )
    {
        thrd.join();
    }
    thread_list_.clear();

    sem_.reset();
}


void tcam::algorithms::work_pool::work_pool::worker_thread_function (int /* thread_index*/)
{
    while (!ended_flag_)
    {
        sem_.lock();

        if (ended_flag_)
        {
            break;
        }

        work_context* item = pop();

        if (item != nullptr)
        {
            item->do_one();
        }
    }
}

tcam::algorithms::work_pool::work_context* tcam::algorithms::work_pool::work_pool::pop ()
{
    std::lock_guard<decltype(vec_lock_)> lck(vec_lock_);

    if (vec_.empty())
    {
        return nullptr;
    }

    auto rval = vec_.back();
    vec_.pop_back();
    return rval;
}
