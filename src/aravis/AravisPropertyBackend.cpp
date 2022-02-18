/*
 * Copyright 2021 The Imaging Source Europe GmbH
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

#include "AravisPropertyBackend.h"

#include "../logging.h"
#include "AravisDevice.h"
#include "aravis_utils.h"

#include <arv.h>

using namespace tcam::aravis;

AravisPropertyBackend::AravisPropertyBackend(tcam::AravisDevice& parent)
    : parent_(parent)
{
}

std::recursive_mutex& AravisPropertyBackend::get_mutex() noexcept
{
    return parent_.arv_camera_access_mutex_;
}

tcamprop1::Visibility_t tcam::aravis::to_Visibility(ArvGcVisibility v) noexcept
{
    switch (v)
    {
        case ARV_GC_VISIBILITY_UNDEFINED:
            return tcamprop1 ::Visibility_t::Invisible;
        case ARV_GC_VISIBILITY_INVISIBLE:
            return tcamprop1 ::Visibility_t::Invisible;
        case ARV_GC_VISIBILITY_GURU:
            return tcamprop1 ::Visibility_t::Guru;
        case ARV_GC_VISIBILITY_EXPERT:
            return tcamprop1 ::Visibility_t::Expert;
        case ARV_GC_VISIBILITY_BEGINNER:
            return tcamprop1 ::Visibility_t::Beginner;
    }
    return tcamprop1 ::Visibility_t::Invisible;
}

tcamprop1::Access_t tcam::aravis::to_Access(ArvGcAccessMode v) noexcept
{
    switch (v)
    {
        case ARV_GC_ACCESS_MODE_UNDEFINED:
            return tcamprop1::Access_t::RW;
        case ARV_GC_ACCESS_MODE_RO:
            return tcamprop1::Access_t::RO;
        case ARV_GC_ACCESS_MODE_WO:
            return tcamprop1::Access_t::WO;
        case ARV_GC_ACCESS_MODE_RW:
            return tcamprop1::Access_t::RW;
    }
    return tcamprop1::Access_t::RW;
}
