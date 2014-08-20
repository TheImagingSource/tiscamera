

#include "MemoryBuffer.h"

#include <cstring>
#include <cstdlib>

using namespace tis_imaging;


MemoryBuffer::MemoryBuffer (const struct image_buffer& buf)
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


image_buffer MemoryBuffer::getImageBuffer ()
{
    return buffer;
}

void MemoryBuffer::setImageBuffer (image_buffer buf)
{
    this->buffer = buf;
}


unsigned char* MemoryBuffer::getData ()
{
    return buffer.pData;
}

void MemoryBuffer::clear ()
{
    memset(buffer.pData, 0, buffer.length);
}


void MemoryBuffer::lock ()
{
    lock_count++;
}


bool MemoryBuffer::isLocked ()
{
    if (lock_count > 0)
    {
        return true;
    }
    return false;
}


void MemoryBuffer::unlock ()
{
    if (lock_count > 0)
    {    
        lock_count--;
    }
}


void MemoryBuffer::forceUnlock ()
{
    lock_count = 0;
}
    
