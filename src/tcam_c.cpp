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

#include "tcam_c.h"

#include "tcam.h"
#include "internal.h"
#include "Error.h"

#include <string.h>

using namespace tcam;

/* error / debug */

int tcam_last_errno ()
{
    return get_error().get_errno();
}


const char* tcam_last_error_messsage ()
{
    return get_error().get_string().c_str();
}



int tcam_device_index_get_device_count ()
{
    auto vec = get_device_list();

    return vec.size();
}


int tcam_device_index_get_device_infos (tcam_device_info* arr,
                                        size_t array_size)
{
    auto vec = get_device_list();

    if (vec.size() < array_size)
    {
        set_error(Error("Array is to small to contain all devices", EFAULT));
        return -1;
    }

    int i = 0;
    for (const auto& v : vec)
    {
        arr[i] = v.get_info();
        i++;
    }

    return vec.size();
}


/* image source */


tcam_capture_device* tcam_open_device (const char* serial)
{
    auto dev = open_device(serial);

    if (dev == nullptr)
    {
        return NULL;
    }
    else
    {
        //return (tcam_capture_device*) dev;
    }
}


tcam_capture_device* tcam_create_new_capture_device (const tcam_device_info* info)
{
    return (tcam_capture_device*) new CaptureDevice(DeviceInfo(*info));
}


void tcam_destroy_capture_device (tcam_capture_device* source)
{
    delete reinterpret_cast<CaptureDevice*>(source);
}


/* device related */

bool tcam_capture_device_is_device_open (tcam_capture_device* source)
{
    return reinterpret_cast<CaptureDevice*>(source)->is_device_open();
}


bool tcam_capture_device_get_device_info (struct tcam_capture_device* source,
                                          struct tcam_device_info* info)
{
    auto dev = reinterpret_cast<const CaptureDevice*>(source)->get_device();

    auto io = dev.get_info();
    memcpy(info, &io , sizeof(struct tcam_device_info));

    return true;
}

/* property related*/

int tcam_capture_device_get_properties_count (const tcam_capture_device* source)
{
    return ((CaptureDevice*)source)->get_available_properties().size();
}


int tcam_capture_device_get_properties (const tcam_capture_device* source,
                                        struct tcam_device_property* array,
                                        const size_t array_size)
{
    if (tcam_capture_device_get_properties_count(source) > array_size)
    {
        return 0;
    }

    auto vec = ((CaptureDevice*)source)->get_available_properties();

    size_t x = 0;
    for (auto& v : vec)
    {
        array[x] = v->get_struct();
        if (x == array_size)
        {
            return array_size;
        }
        x++;
    }

    return tcam_capture_device_get_properties_count(source);
}


bool  tcam_capture_device_find_property (tcam_capture_device* source,
                                         TCAM_PROPERTY_ID id,
                                         struct tcam_device_property* property)
{
    if (source == nullptr)
    {
        tcam_log(TCAM_LOG_ERROR, "Source is null");
        return false;
    }

    // auto vec = reinterpret_cast<CaptureDevice*>(source)->get_available_properties();
    auto vec = ((CaptureDevice*)source)->get_available_properties();

    for (auto& v : vec)
    {
        if (v->get_ID() == id)
        {
            auto prop = v->get_struct();

            //tcam_log(TCAM_LOG_ERROR, "==================== name: %s - %d", prop.name , prop.value.i.value);

            memcpy(property, &prop, sizeof(struct tcam_device_property));

            return true;
        }
    }
    return false;
}


int tcam_capture_device_set_property (tcam_capture_device* source,
                                      const struct tcam_device_property* property)
{
    auto vec = reinterpret_cast<CaptureDevice*>(source)->get_available_properties();

    for (auto& v :vec)
    {
        if (v->get_ID() == property->id)
        {
            v->set_property_from_struct(*property);
            return true;
        }
    }
    set_error(Error("No such property found", ENOENT));

    return -1;
}

int tcam_capture_device_get_property (tcam_capture_device* source,
					   struct tcam_device_property* property)
{
    auto vec = reinterpret_cast<CaptureDevice*>(source)->get_available_properties();

    for (auto& v :vec)
    {
        if (v->get_ID() == property->id)
        {
            v->get_property_from_struct(*property);
            return true;
        }
    }
    set_error(Error("No such property found", ENOENT));

    return -1;
}


/* video format related */

int tcam_capture_device_get_image_format_descriptions_count (const tcam_capture_device* source)
{
    if (source == nullptr)
    {
        return -1;
    }

    return reinterpret_cast<const CaptureDevice*>(source)->get_available_video_formats().size();
}


int tcam_capture_device_get_image_format_descriptions (const tcam_capture_device* source,
                                                      tcam_video_format_description* arr,
                                                      const size_t array_size)
{

    auto vec = reinterpret_cast<const CaptureDevice*>(source)->get_available_video_formats();

    if (vec.size() > array_size)
    {
        return -1;
    }

    int i = 0;
    for (const auto& v : vec)
    {
        arr[i] = v.get_struct();
        i++;
    }

    return vec.size();
}



int tcam_capture_device_get_format_resolution (const tcam_capture_device* source,
                                               const tcam_video_format_description* desc,
                                               struct tcam_resolution_description* array,
                                               const size_t array_size)
{
    auto formats = reinterpret_cast<const CaptureDevice*>(source)->get_available_video_formats();

    for (const auto& f : formats)
    {
        if (f == *desc)
        {
            auto vec = f.get_resolutions();

            if (vec.size() > array_size)
            {
                return -1;
            }

            memcpy(array, vec.data(), vec.size() * sizeof(struct tcam_resolution_description));
            return vec.size();
        }
    }

    return 0;
}


int tcam_capture_device_get_resolution_framerate (const tcam_capture_device* source,
                                                  const struct tcam_video_format_description* desc,
                                                  const struct tcam_resolution_description* resolution,
                                                  double* array,
                                                  const size_t array_size)
{
    auto formats = reinterpret_cast<const CaptureDevice*>(source)->get_available_video_formats();

    for (const auto& f : formats)
    {
        if (f == *desc)
        {
            auto vec = f.get_frame_rates(*resolution);

            if (vec.size() > array_size)
            {
                return -1;
            }

            if (vec.empty())
            {
                return 0;
            }

            memcpy(array, vec.data(), vec.size() * sizeof(double));
            return vec.size();
        }
    }


    return 0;
}


bool tcam_capture_device_set_image_format (tcam_capture_device* source,
                                           const tcam_video_format* format)
{
    VideoFormat f (*format);

    return reinterpret_cast<CaptureDevice*>(source)->set_video_format(f);
}


bool tcam_capture_device_get_image_format (tcam_capture_device* source,
                                           tcam_video_format* format)
{
    VideoFormat f = reinterpret_cast<CaptureDevice*>(source)->get_active_video_format();

    *format = f.get_struct();

    return true;
}


struct tmp_stream_obj
{
    std::shared_ptr<ImageSink> sink;
};

/* streaming functions */

stream_obj* tcam_capture_device_start_stream (tcam_capture_device* source,
                                             tcam_image_callback callback,
                                             void* user_data)
{
    auto obj =  new tmp_stream_obj();
    obj->sink = std::make_shared<ImageSink>();

    obj->sink->registerCallback(callback, user_data);

    bool ret = reinterpret_cast<CaptureDevice*>(source)->start_stream(obj->sink);

    if (!ret)
    {
        return NULL;
    }

    return (stream_obj*)obj;
}


bool tcam_capture_device_stop_stream (tcam_capture_device* source)
{
    return reinterpret_cast<CaptureDevice*>(source)->stop_stream();
}


unsigned int tcam_image_buffer_lock (const struct tcam_image_buffer* buffer)
{
    const_cast<tcam_image_buffer*>(buffer)->lock_count++;
    return buffer->lock_count;
}


unsigned int tcam_image_buffer_get_lock_count (const struct tcam_image_buffer* buffer)
{
    return buffer->lock_count;
}


unsigned int tcam_image_buffer_unlock (const struct tcam_image_buffer* buffer)
{
    if (buffer->lock_count >= 1)
        const_cast<tcam_image_buffer*>(buffer)->lock_count--;
    return buffer->lock_count;
}
