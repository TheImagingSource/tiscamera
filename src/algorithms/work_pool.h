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

#ifndef TCAM_ALGORITHM_WORK_POOL_H
#define TCAM_ALGORITHM_WORK_POOL_H

#include <thread>
#include <mutex>
#include <vector>
#include <atomic>
#include "pthread_semaphore.h"
#include "std_latch.h"

namespace tcam
{

namespace algorithms
{

namespace work_pool
{

int get_logical_cpu_count ();

struct work_context
{
    virtual void do_one () = 0;
};

class work_pool
{
public:
    work_pool ();
    ~work_pool ();

    bool start ();
    void stop ();

    template<class T>
    void queue_items_and_wait (threading::latch& all_items_finished, T* vec, int cnt)
    {
        all_items_finished.reset(cnt);

        queue_items_(vec, cnt - 1);

        vec[cnt - 1].do_one();

        all_items_finished.wait();
    }

private:

    template<class T>
    void queue_items_ (T* vec, int cnt)
    {
        {
            std::lock_guard<std::mutex> lck(vec_lock_);

            vec_.insert(vec_.begin(), cnt, (work_context*) nullptr);
            for (int i = 0; i < cnt; ++i)
            {
                vec_[i] = vec + i;
            }
        }
        sem_.up( cnt );
    }

    void worker_thread_function (int thread_index);

    std::vector<std::thread> thread_list_;
    std::mutex vec_lock_;

    std::vector<work_context*> vec_;

    work_context* pop ();

    threading::semaphore sem_;

    std::atomic<bool> ended_flag_;

}; // class work_pool


work_pool* acquire_default_work_pool ();

void release_default_work_pool (work_pool* pool);

}; // namespace work_pool

}; // namespace algorithms

}; // namespace tcam

#endif /* TCAM_ALGORITHM_WORK_POOL_H */
