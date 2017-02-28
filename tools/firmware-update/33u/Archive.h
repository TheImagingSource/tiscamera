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

#pragma once

#include "lib/7z/7z.h"
#include "lib/7z/7zFile.h"
#include "lib/7z/7zAlloc.h"

#include <memory>
#include <map>
#include <string>

namespace lib33u
{
namespace util
{
namespace sz
{
class Archive
{
private:
    class Entry
    {
        std::shared_ptr<uint8_t> data_;
        size_t size_;

    public:
        Entry (std::shared_ptr<uint8_t> ptr, size_t size)
            : data_ { ptr }, size_ { size }
        {}

        const uint8_t* data () const
        {
            return data_.get();
        }

        size_t size () const
        {
            return size_;
        }
    };

    struct Data
    {
        CFileInStream archive_stream_;
        CLookToRead look_stream_;
        CSzArEx db_;
        std::map<std::string, Entry> file_index_;

        Data (const std::string& fn);

        Entry read (const std::string& fn) const;
    };

private:
    std::unique_ptr<Data> data;

    Archive (const std::string& fn);

public:
    static Archive open (const std::string& fn);

public:
    Entry read (const std::string& fn) const;
};

} /* namespace sz */
} /* namespace util */
} /* namespace lib33u */
