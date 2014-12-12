

#include "MemoryBuffer.h"

#include <cstring>
#include <cstdlib>

using namespace tcam;


MemoryBuffer::MemoryBuffer (const struct tcam_image_buffer& buf)
    : buffer(buf), lock_count(0)
{}


MemoryBuffer::~MemoryBuffer ()
{
    if (buffer.pData != nullptr)
    {
        free(buffer.pData);
        buffer.pData = nullptr;

        buffer.length = 0;
    }
}


tcam_image_buffer MemoryBuffer::getImageBuffer ()
{
    return buffer;
}

void MemoryBuffer::setImageBuffer (tcam_image_buffer buf)
{
    this->buffer = buf;
}


unsigned char* MemoryBuffer::getData ()
{
    return buffer.pData;
}


struct tcam_stream_statistics MemoryBuffer::getStatistics () const
{
    return buffer.statistics;
}


bool MemoryBuffer::setStatistics (const struct tcam_stream_statistics& stats)
{
    buffer.statistics = stats;;

    return true;
}


void MemoryBuffer::clear ()
{
    memset(buffer.pData, 0, buffer.length);
}
