/*
 * Copyright 2022 The Imaging Source Europe GmbH
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

#include "V4L2Allocator.h"

#include "../base_types.h"
#include "../error.h"
#include "../logging.h"
#include "../utils.h"

#include <linux/videodev2.h>
#include <sys/mman.h> /* mmap PROT_READ*/

using namespace tcam;

namespace
{

//outcome::result<void> reqbufs(int fd_,
bool reqbufs(int fd_,
                              struct v4l2_requestbuffers& req,
                              const std::string& err_str = "")
{
    if (tcam::tcam_xioctl(fd_, VIDIOC_REQBUFS, &req) == -1)
    {
        if (EINVAL == errno)
        {
            SPDLOG_INFO("Device does not support {}", err_str);
        }
        else
        {
            SPDLOG_ERROR("VIDIOC_REQBUFS: {}", strerror(errno));
        }
        return false;
    }
    return true;
 //   return outcome::success();
}

} // namespace


void tcam::V4L2Allocator::query_supported_memory_types()
{
    memory_types_.clear();

    struct v4l2_requestbuffers req = {};

    req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req.count = 1;
    req.memory = V4L2_MEMORY_USERPTR;

    if (reqbufs(fd_, req, "user pointer i/o"))
    {
        memory_types_.push_back(TCAM_MEMORY_TYPE_USERPTR);
        req.count = 0;
        tcam_xioctl(fd_, VIDIOC_REQBUFS, &req);
    }

    req.count = 1;
    req.memory = V4L2_MEMORY_MMAP;

    if (reqbufs(fd_, req, "mmap"))
    {
        memory_types_.push_back(TCAM_MEMORY_TYPE_MMAP);
        req.count = 0;
        tcam_xioctl(fd_, VIDIOC_REQBUFS, &req);
    }

    req.memory = V4L2_MEMORY_DMABUF;
    req.count = 1;

    if (reqbufs(fd_, req, "DMA"))
    {
        //memory_types_.push_back(TCAM_MEMORY_TYPE_DMA);
        //memory_types_.push_back(TCAM_MEMORY_TYPE_DMA_IMPORT);
        // TODO: implement dma support
        req.count = 0;
        tcam_xioctl(fd_, VIDIOC_REQBUFS, &req);
    }
}


std::vector<std::shared_ptr<Memory>> V4L2Allocator::allocate_userptr(size_t length,
                                                                     size_t buffer_count)
{
    std::vector<std::shared_ptr<Memory>> buffers;
    buffers.reserve(buffer_count);

    for (unsigned int i = 0; i < buffer_count; ++i)
    {
        auto ptr = malloc(length);
        if (!ptr)
        {
            SPDLOG_ERROR("malloc failed ({}): {}", errno, strerror(errno));
            continue;
        }
        buffers.push_back(std::make_shared<Memory>(shared_from_this(), TCAM_MEMORY_TYPE_USERPTR, length, ptr));
    }

    return buffers;
}


std::vector<std::shared_ptr<Memory>> V4L2Allocator::allocate_mmap(size_t length,
                                                                  size_t buffer_count)
{
    if (buffer_count < 2)
    {
        SPDLOG_ERROR("Insufficient buffer memory for mmap");
        return {};
    }

    std::vector<std::shared_ptr<Memory>> buffers;

    buffers.reserve(buffer_count);

    size_t buffer_size = length;

    struct v4l2_requestbuffers req = {};

    req.count = buffer_count;
    req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req.memory = V4L2_MEMORY_MMAP;

    if (-1 == tcam_xioctl(fd_, VIDIOC_REQBUFS, &req))
    {
            SPDLOG_ERROR("VIDIOC_REQBUFS {}", strerror(errno));

    }

    if (req.count != buffer_count)
    {
        SPDLOG_ERROR("Can only allocate {} mmap buffer. Aborting.", req.count);
        return {};
    }

    for (unsigned int n_buffers = 0; n_buffers < buffer_count; ++n_buffers)
    {
        struct v4l2_buffer buf = {};

        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
        buf.index = n_buffers;

        auto ptr =
            (unsigned char*)mmap(NULL,
                                 buffer_size,
                                 PROT_READ | PROT_WRITE, /* required */
                                 MAP_SHARED, /* recommended */
                                 fd_,
                                 buf.m.offset);

        if (ptr == MAP_FAILED)
        {
            SPDLOG_ERROR("MMAP failed for buffer {}. Aborting. {}", n_buffers, strerror(errno));
            continue;
        }

        SPDLOG_TRACE("New mmap buffer {} {}", n_buffers, fmt::ptr(ptr));
        buffers.push_back(std::make_shared<Memory>(shared_from_this(), TCAM_MEMORY_TYPE_MMAP, length, ptr));

        // TODO: find way to ensure fourcc is correctly handled

        // if (buffer.format.fourcc == mmioFOURCC('G', 'R', 'E', 'Y'))
        // {
        //     buffer.format.fourcc = FOURCC_Y800;
        // }

        // if (emulate_bayer)
        // {
        //     if (emulated_fourcc != 0)
        //     {
        //         buffer.format.fourcc = emulated_fourcc;
        //     }
        // }

        //        buffer.pitch = format.get_pitch_size();
    }
    return buffers;
}


std::vector<std::shared_ptr<Memory>> V4L2Allocator::allocate_dma(size_t /*length*/,
                                                                 int fd,
                                                                 size_t buffer_count)
{
    struct v4l2_requestbuffers rqbufs = {};
    rqbufs.count = buffer_count;
    rqbufs.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    rqbufs.memory = V4L2_MEMORY_DMABUF;

    if (tcam_xioctl(fd_, VIDIOC_REQBUFS, &rqbufs) == -1)
    {
        if (EINVAL == errno)
        {
            fprintf(stderr,
                    "does not support "
                    "dma\n");
            exit(EXIT_FAILURE);
        }
        else
        {
            SPDLOG_ERROR("VIDIOC_REQBUFS");
        }
    }

    struct v4l2_exportbuffer expbuf;

    memset(&expbuf, 0, sizeof(expbuf));
    expbuf.type = rqbufs.type;
    expbuf.index = 0;// index;
    if (tcam_xioctl(fd_, VIDIOC_EXPBUF, &expbuf) == -1)
    {
        perror("VIDIOC_EXPBUF");
        //return -1;
    }

    //int dmafd = expbuf.fd;

    // this is queueing

    for (unsigned int i = 0; i < buffer_count; ++i)
    {
        struct v4l2_buffer buf = {};

        buf.index = i;
        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_DMABUF;
        //buf.m.fd = buffer[i].dbuf_fd;
        buf.m.fd = fd;

        if (tcam_xioctl(fd_, VIDIOC_QBUF, &buf) == -1)
        {
            // TODO: error handling
        }
    }



    return {};
}


void tcam::V4L2Allocator::free_userptr(void* ptr)
{
    if (ptr)
    {
        SPDLOG_TRACE("FREE USERPTR");
        std::free(ptr);
    }
}


void tcam::V4L2Allocator::free_mmap(void* ptr, size_t length)
{
    if (!ptr || length == 0)
    {
        return;
    }

    if (munmap(ptr, length) == -1)
    {
        // TODO: error

        return;
    }
    else
    {
        SPDLOG_TRACE("FREE mmap");
    }
}


void tcam::V4L2Allocator::free_dma(std::vector<std::shared_ptr<ImageBuffer>> /*buffer_collection*/)
{
}


void* tcam::V4L2Allocator::allocate(TCAM_MEMORY_TYPE /*type*/,
                                    size_t /*length*/,
                                    int // fd
    )
{

    return nullptr;
}


void tcam::V4L2Allocator::free(TCAM_MEMORY_TYPE type, void* ptr, size_t length, int /*fd*/)
{
    switch(type)
    {
        case TCAM_MEMORY_TYPE_USERPTR:
        {
            free_userptr(ptr);
            break;
        }
        case TCAM_MEMORY_TYPE_MMAP:
        {
            free_mmap(ptr, length);
            break;
        }
        case TCAM_MEMORY_TYPE_DMA:
        {
            //free_dma(std::vector<std::shared_ptr<ImageBuffer>>)
            break;
        }
        case TCAM_MEMORY_TYPE_DMA_IMPORT:
        {
            SPDLOG_ERROR("Explicitely not supported");
            break;
        }
    }
}


std::vector<std::shared_ptr<Memory>> tcam::V4L2Allocator::allocate(
    size_t buffer_count, TCAM_MEMORY_TYPE type, size_t length, int fd)
{

    switch (type)
    {
        case TCAM_MEMORY_TYPE_USERPTR:
        {
            return allocate_userptr(length, buffer_count);
        }
        case TCAM_MEMORY_TYPE_MMAP:
        {
            return allocate_mmap(length, buffer_count);
        }
        case TCAM_MEMORY_TYPE_DMA:
        {
            return allocate_dma(length, buffer_count, fd);
        }
        case TCAM_MEMORY_TYPE_DMA_IMPORT:
        {
            SPDLOG_ERROR("Nothing to allocate. External memory");
            return {};
        }
    }

    return {};
}
