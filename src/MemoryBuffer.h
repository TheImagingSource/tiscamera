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

#ifndef TCAM_MEMORYBUFFER_H
#define TCAM_MEMORYBUFFER_H

#include "base_types.h"

/**
 * @addtogroup API
 * @{
 */

namespace tcam
{

class MemoryBuffer
{

public:

    MemoryBuffer (const struct tcam_image_buffer&);

    MemoryBuffer () = delete;

    ~MemoryBuffer ();

    /**
     *
     */
    tcam_image_buffer getImageBuffer ();

    void set_image_buffer (tcam_image_buffer);

    /**
     * @return Pointer to actual image data
     */
    unsigned char* get_data ();


    struct tcam_stream_statistics get_statistics () const;

    bool set_statistics (const struct tcam_stream_statistics&);

    bool lock ();

    bool unlock ();

    bool is_locked () const;

    /**
     * @brief Fills MemoryBuffer with 0
     */
    void clear ();

private:

    struct tcam_image_buffer buffer;

};


} /* namespace tcam*/

/** @} */

#endif /* TCAM_MEMORYBUFFER_H */
