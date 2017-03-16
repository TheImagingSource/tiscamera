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

#include "Archive.h"

#include "lib/7z/7zCrc.h"

// gcc only offers a full c++11 implementation with >= v5.1
// codecvt for unicode conversions is not always available
// thus use iconv in such cases
#include <features.h>
#if __GNUC_PREREQ(5,1)
#include <codecvt>
#else
#include <iconv.h>
#endif


#include <string>
#include <locale>
#include <stdexcept>
#include <memory>

namespace lib33u
{
namespace util
{
namespace sz
{
struct DefaultAlloc : public ISzAlloc
{
    DefaultAlloc()
    {
        Alloc = SzAlloc;
        Free = SzFree;
    }
};

struct TempAlloc : public ISzAlloc
{
    TempAlloc()
    {
        Alloc = SzAllocTemp;
        Free = SzFreeTemp;
    }
};

int SzCrcGenerateTableOnce()
{
    CrcGenerateTable();
    return 0;
}

Archive::Data::Data( const std::string& fn )
{
    static int x = SzCrcGenerateTableOnce();

    auto wres = InFile_Open( &archive_stream_.file, fn.c_str() );
    if( wres )
    {
        throw std::runtime_error( "Unable to open archive file" );
    }

    FileInStream_CreateVTable( &archive_stream_ );

    LookToRead_CreateVTable( &look_stream_, False );
    look_stream_.realStream = &archive_stream_.s;
    LookToRead_Init( &look_stream_ );

    SzArEx_Init( &db_ );

    DefaultAlloc default_alloc_;
    TempAlloc temp_alloc_;

    auto sres = SzArEx_Open(&db_, &look_stream_.s,
                            &default_alloc_, &temp_alloc_);
    if( sres )
    {
        throw std::runtime_error( "Failed to open archive" );
    }

    struct ArchiveBuffer
    {
        UInt32 block_index_ = 0xFFFFFFFF;
        Byte* out_buffer_ = nullptr;
        size_t out_buffer_size_ = 0;

        ~ArchiveBuffer()
        {
            DefaultAlloc default_alloc;
            IAlloc_Free( &default_alloc, out_buffer_ );
        }
    };

#ifdef __linux__
    typedef std::u16string utf16string;
#else
    typedef std::wstring utf16string;
#endif

    utf16string utf16name;
    auto buffer = std::make_shared<ArchiveBuffer>();

    for( unsigned i = 0; i < db_.NumFiles; ++i )
    {
        size_t len = SzArEx_GetFileNameUtf16( &db_, i, nullptr );

        utf16name.resize(len);

        SzArEx_GetFileNameUtf16( &db_, i, (UInt16*)utf16name.data() );


#if __GNUC_PREREQ(5,1)

        std::wstring_convert<std::codecvt_utf8_utf16<utf16string::value_type>,
                             utf16string::value_type> conversion;
        std::string utf8name = conversion.to_bytes( utf16name );

#else

        iconv_t ico = iconv_open("UTF8", "UTF16");

        size_t out_size = 1024;

        char buf [out_size];

        char* tmp = buf;
        const char16_t* tmp16 = utf16name.data();

        size_t in_size = len*2;

        size_t ret = iconv(ico, (char**)&tmp16, &in_size, &tmp, &out_size);

        std::string utf8name(buf);

        iconv_close(ico);

#endif

        while( utf8name.back() == '\0' )
            utf8name.pop_back();

        size_t offset = 0;
        size_t outSizeProcessed = 0;

        sres = SzArEx_Extract(&db_, &look_stream_.s, i,
                              &buffer->block_index_,
                              &buffer->out_buffer_,
                              &buffer->out_buffer_size_,
                              &offset,
                              &outSizeProcessed,
                              &default_alloc_,
                              &temp_alloc_ );

        // file_index_.try_emplace( utf8name, std::shared_ptr<uint8_t>( buffer, buffer->out_buffer_ + offset ), outSizeProcessed );
        file_index_.emplace(utf8name,
                            Entry(std::shared_ptr<uint8_t>(buffer, buffer->out_buffer_ + offset),
                                  outSizeProcessed ));
    }
}

Archive::Entry Archive::Data::read (const std::string& fn) const
{
    auto it = file_index_.find(fn);
    if (it == file_index_.end())
    {
        throw std::runtime_error("Entry not found");
    }
    return it->second;
}

Archive::Archive (const std::string& fn)
    : data { std::unique_ptr<Data>(new Data(fn)) }
 {
 }

Archive Archive::open (const std::string& fn)
{
    return Archive( fn );
}

Archive::Entry Archive::read (const std::string& fn) const
{
    return data->read( fn );
}
} /* namespace sz */
} /* namespace util */
} /* namespace lib33u */
