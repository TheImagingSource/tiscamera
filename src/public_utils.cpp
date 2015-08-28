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

#include "public_utils.h"

#include "utils.h"

using namespace tcam;

const char* tcam_fourcc_to_description (uint32_t fourcc)
{
    return fourcc2description(fourcc);
}


uint32_t tcam_description_to_fourcc (const char* description)
{
    return description2fourcc(description);
}


struct tcam_image_buffer* tcam_allocate_image_buffers (const struct tcam_video_format* format,
                                                       size_t n_buffers)
{
    struct tcam_image_buffer* ptr = nullptr;

    if (format != nullptr && n_buffers > 0)
    {
        // allocate buffer array
        ptr = (struct tcam_image_buffer*)malloc(sizeof(struct tcam_image_buffer) * n_buffers);

        unsigned int length = 0;
        // allocate the actual image data fields
        for (unsigned int i = 0; i < n_buffers; ++i)
        {
            struct tcam_image_buffer* tmp = &ptr[i];

            tmp->pData = (unsigned char*)malloc(tcam_get_required_buffer_size(format));

            // fill the rest
            tmp->length = length;
            tmp->format = *format;
            tmp->pitch = get_pitch_length(format->width, format->fourcc);
        }

    }

    return ptr;
}


void tcam_free_image_buffers (struct tcam_image_buffer* ptr, size_t n_buffer)
{
    if (ptr == nullptr || n_buffer < 1)
        return;

    free (ptr);
}
