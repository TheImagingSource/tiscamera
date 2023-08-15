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

#include "gsttcambufferpool.h"

#include "../../../libs/tcam-property/src/gst/meta/gstmetatcamstatistics.h"
#include "gst/gstbufferpool.h"
#include "gst/gstinfo.h"
#include "gsttcammainsrc.h"
#include "mainsrc_device_state.h"
#include "../tcamgstbase/tcamgstbase.h"

struct tcam_pool_state
{
    std::vector<tcam::mainsrc::buffer_info> buffer;
    bool is_mjpeg = false;
};

#define GST_CAT_DEFAULT tcam_mainsrc_debug

#define gst_tcam_buffer_pool_parent_class parent_class
G_DEFINE_TYPE(GstTcamBufferPool, gst_tcam_buffer_pool, GST_TYPE_BUFFER_POOL)


static void statistics_to_gst_structure(const tcam::tcam_stream_statistics& stat,
                                        GstStructure& struc)
{
    // Disable this code when tracing is not enabled
    // if (gst_debug_category_get_threshold(GST_CAT_DEFAULT) >= GST_LEVEL_TRACE)
    // {
    //     const char* damaged = nullptr;
    //     if (stat.is_damaged)
    //     {
    //         damaged = "true";
    //     }
    //     else
    //     {
    //         damaged = "false";
    //     }
    //     uint64_t gst_frame_count = self->element.parent.segment.position;
    //     auto test = fmt::format("Added meta info: \n"
    //                             "gst frame_count: {}\n"
    //                             "backend frame_count {}\n"
    //                             "frames_dropped {}\n"
    //                             "capture_time_ns: {}\n"
    //                             "camera_time_ns: {}\n"
    //                             "is_damaged: {}\n",
    //                             gst_frame_count,
    //                             stat.frame_count,
    //                             stat.frames_dropped,
    //                             stat.capture_time_ns,
    //                             stat.camera_time_ns,
    //                             damaged);

    //     GST_TRACE("%s", test.c_str());
    // }
    gst_structure_set(&struc,
                      "frame_count",
                      G_TYPE_UINT64,
                      stat.frame_count,
                      "frames_dropped",
                      G_TYPE_UINT64,
                      stat.frames_dropped,
                      "capture_time_ns",
                      G_TYPE_UINT64,
                      stat.capture_time_ns,
                      "camera_time_ns",
                      G_TYPE_UINT64,
                      stat.camera_time_ns,
                      "is_damaged",
                      G_TYPE_BOOLEAN,
                      stat.is_damaged,
                      nullptr);
}


static void gst_tcam_buffer_pool_sh_callback(std::shared_ptr<tcam::ImageBuffer> buffer, void* data)
{
    GstTcamBufferPool* self = GST_TCAM_BUFFER_POOL(data);
    struct device_state* state = GST_TCAM_MAINSRC(self->src_element)->device;

    if (!state->is_streaming_)
    {
        // requeue the buffer so that the backend does not run out
        state->sink->requeue_buffer(buffer);
        return;
    }

    std::unique_lock<std::mutex> lck(state->stream_mtx_);

    for (auto& info : self->state_->buffer)
    {
        if (info.tcam_buffer == buffer)
        {
            auto stats = buffer->get_statistics();
            GstMeta* meta =
                gst_buffer_get_meta(info.gst_buffer, g_type_from_name("TcamStatisticsMetaApi"));
            if (meta)
            {
                GstStructure* struc = ((TcamStatisticsMeta*)meta)->structure;

                if (struc)
                {
                    statistics_to_gst_structure(stats, *struc);
                }
            }

            if (stats.is_damaged && !state->drop_incomplete_frames_)
            {
                GST_WARNING_OBJECT(GST_OBJECT(self), "Delivering damaged buffer.");
                gst_buffer_set_flags(info.gst_buffer, GST_BUFFER_FLAG_CORRUPTED);
            }

            // ubuntu 18 has gstreamer 1.14.x
            // there set_size causes a stream termination when using mjpeg
            // other cameras cause problems when this function is not called
            // more modern systems seem to always need to call this
            if (GST_VERSION_MINOR >= 15 || !self->state_->is_mjpeg)
            {
                gst_buffer_set_size(info.gst_buffer, info.tcam_buffer->get_valid_data_length());
            }
            info.pooled = false;
            state->queue.push(info);

            state->stream_cv_.notify_all();
            break;
        }
    }

    lck.unlock();
}


static GstFlowReturn gst_tcam_buffer_pool_acquire_buffer(GstBufferPool* pool,
                                                         GstBuffer** buffer,
                                                         GstBufferPoolAcquireParams* /*params*/)
{
    if (GST_BUFFER_POOL_IS_FLUSHING(pool))
    {
        return GST_FLOW_FLUSHING;
    }

    GstTcamBufferPool* self = GST_TCAM_BUFFER_POOL(pool);
    struct device_state* state = GST_TCAM_MAINSRC(self->src_element)->device;
    std::unique_lock<std::mutex> lck(state->stream_mtx_);
    while (true)
    {
        if (state->queue.empty() && state->is_streaming_)
        {
            // wait until new buffer arrives or stop waiting when we have to shut down
            state->stream_cv_.wait(lck);
        }
        if (!state->is_streaming_)
        {
            return GST_FLOW_FLUSHING;
        }
        if (!state->queue.empty())
        {
            auto buffer_desc = state->queue.front();
            state->queue.pop(); // remove buffer from queue
            *buffer = buffer_desc.gst_buffer;

            return GST_FLOW_OK;
        }
    }

    // TOOD: return flushing only when inactive
    // acquire may block, so we can wait indefinetly

    return GST_FLOW_FLUSHING;
}


static void gst_tcam_buffer_pool_release_buffer(GstBufferPool* pool, GstBuffer* buffer)
{
    GstTcamBufferPool* self = GST_TCAM_BUFFER_POOL(pool);
    struct device_state* state = GST_TCAM_MAINSRC(self->src_element)->device;

    std::unique_lock<std::mutex> lck(state->stream_mtx_);
    for (auto& info : self->state_->buffer)
    {
        if (info.gst_buffer == buffer)
        {
            info.pooled = true;

            if (state->sink)
            {
                state->sink->requeue_buffer(info.tcam_buffer);
            }
            else
            {
                GST_ERROR_OBJECT(self, "Unable to requeue buffer. Device is not open.");
            }
        }
    }
    lck.unlock();
}


static gboolean gst_tcam_buffer_pool_set_config(GstBufferPool* /*bpool*/, GstStructure* /*config*/)
{
    //GST_INFO("set_config============================================");
    return TRUE;
}


static void prepare_gst_buffer_pool(GstTcamBufferPool* self)
{
    struct device_state* state = GST_TCAM_MAINSRC(self->src_element)->device;


    auto tcam_buffers = state->buffer_pool->get_buffer();

    for (auto& tb : tcam_buffers)
    {
        if (auto b = tb.lock())
        {
            void* address = b->get_image_buffer_ptr();
            size_t size = b->get_image_buffer_size();


            GstBuffer* gst_buffer = gst_buffer_new_wrapped_full(
                static_cast<GstMemoryFlags>(0), address, size, 0, size, nullptr, nullptr);

            gst_buffer_set_flags(gst_buffer, GST_BUFFER_FLAG_LIVE);

            // TODO: check config and add meta data that is listed there
            GstStructure* struc = gst_structure_new_empty("TcamStatistics");
            auto meta = gst_buffer_add_tcam_statistics_meta(gst_buffer, struc);

            if (!meta)
            {
                GST_WARNING_OBJECT(self, "Unable to add meta!");
            }
            else
            {
                auto m = (GstMeta*)meta;
                m->flags = static_cast<GstMetaFlags>(m->flags | GST_META_FLAG_POOLED);
            }

            tcam::mainsrc::buffer_info info;
            info.addr = address;
            info.tcam_buffer = b;
            info.gst_buffer = gst_buffer;
            info.pooled = true;

            self->state_->buffer.push_back(info);
        }
    }
}


// Start the bufferpool.The default implementation will preallocate
// min-buffers buffers and put them in the queue.
static gboolean gst_tcam_buffer_pool_start(GstBufferPool* pool)
{
    GST_INFO("start");

    GstTcamBufferPool* self = GST_TCAM_BUFFER_POOL(pool);
    struct device_state* state = GST_TCAM_MAINSRC(self->src_element)->device;

    if (self->other_pool_)
    {
        GstBuffer* buffer = nullptr;
        if (!gst_buffer_pool_set_active(self->other_pool_, TRUE))
        {
            GST_ERROR_OBJECT(self,
                             "Failed to activate downstream pool. %" GST_PTR_FORMAT,
                             (void*)self->other_pool_);
            return FALSE;
        }
        if (gst_buffer_pool_acquire_buffer(self->other_pool_, &buffer, NULL) != GST_FLOW_OK)
        {
            GST_ERROR_OBJECT(self,
                             "Failed to import buffer from downstream pool. %" GST_PTR_FORMAT,
                             (void*)self->other_pool_);
            return FALSE;
        }

        gst_buffer_unref(buffer);
    }

    GstStructure* config = gst_buffer_pool_get_config(pool);
    GstCaps* caps = nullptr;
    unsigned int size;
    unsigned int min_buffers;
    unsigned int max_buffers;
    if (!gst_buffer_pool_config_get_params(config, &caps, &size, &min_buffers, &max_buffers))
    {
        GST_ERROR_OBJECT(self, "Invalid config for buffer pool.");
        return false;
    }

    self->state_->is_mjpeg = tcam::gst::contains_jpeg(caps);

    auto dev = state->device_;

    tcam::TCAM_MEMORY_TYPE buffer_type = tcam::mainsrc::io_mode_to_memory_type(state->io_mode_);

    tcam::tcam_video_format format;

    tcam::mainsrc::caps_to_format(*caps, format);

    state->buffer_pool = std::make_shared<tcam::BufferPool>(buffer_type, dev->get_allocator());

    auto alloc_res =
        state->buffer_pool->configure(tcam::VideoFormat(format), state->imagesink_buffers_);

    if (!alloc_res)
    {
        GST_ERROR("%s", alloc_res.error().message().c_str());
        GST_ERROR_OBJECT(self, "%s", alloc_res.error().message().c_str());
        return FALSE;
    }

    // prefer user config
    // we do not want to allocate new buffers while running
    // and we have no reason to
    min_buffers = state->imagesink_buffers_;
    max_buffers = state->imagesink_buffers_;

    gst_buffer_pool_config_set_params(config, caps, size, min_buffers, max_buffers);

    gst_structure_free(config);

    // do not flush pool
    // this function might be called from within gst_buffer_pool_set_active
    // which means that the pool is _not yet_ active
    // this would cause warning messages
    // gst_buffer_pool_set_flushing(pool, FALSE);

    auto cb_func = [self](const std::shared_ptr<tcam::ImageBuffer>& cb)
    {
        gst_tcam_buffer_pool_sh_callback(cb, self);
    };


    state->sink =
        std::make_shared<tcam::ImageSink>(cb_func, state->format_, state->imagesink_buffers_);
    state->configure_stream();

    prepare_gst_buffer_pool(self);

    state->start_stream();
    return TRUE;
}


// Stop the bufferpool. the default implementation will free the preallocated
// buffers.This function is called when all the buffers are returned to the pool.
static gboolean gst_tcam_buffer_pool_stop(GstBufferPool* pool)
{
    GstTcamBufferPool* self = GST_TCAM_BUFFER_POOL(pool);

    gst_buffer_pool_set_flushing(pool, TRUE);

    struct device_state* state = GST_TCAM_MAINSRC(self->src_element)->device;

    state->stop_stream();
    state->stream_cv_.notify_all();

    return TRUE;
}


void gst_tcam_buffer_pool_delete_buffer(GstTcamBufferPool* self)
{

    for (const auto& b : self->state_->buffer)
    {
        //GST_INFO("buffer refcount: %d sh_ptr usecount: %ld", b.gst_buffer->mini_object.refcount, b.tcam_buffer.use_count());
        gst_buffer_unref(b.gst_buffer);
    }
    struct device_state* state = GST_TCAM_MAINSRC(self->src_element)->device;

    auto res = state->buffer_pool->clear();
    if (!res)
    {
        GST_ERROR("Error while dealing with buffer pool: %s", res.as_failure().error().message().c_str());
    }
    self->state_->buffer.clear();
    state->device_->free_stream();
}


static void gst_tcam_buffer_pool_dispose(GObject* object)
{
    GST_INFO("disp");

    auto self = GST_TCAM_BUFFER_POOL(object);

    delete self->state_;
    self->state_ = nullptr;

    G_OBJECT_CLASS(parent_class)->dispose(object);
}


static void gst_tcam_buffer_pool_finalize(GObject* object)
{
    G_OBJECT_CLASS(parent_class)->finalize(object);
}


static void gst_tcam_buffer_pool_init(GstTcamBufferPool* pool)
{
    pool->state_ = new tcam_pool_state();
}

static void gst_tcam_buffer_pool_class_init(GstTcamBufferPoolClass* klass)
{
    GObjectClass* object_class = G_OBJECT_CLASS(klass);
    GstBufferPoolClass* bp_class = GST_BUFFER_POOL_CLASS(klass);

    object_class->dispose = gst_tcam_buffer_pool_dispose;
    object_class->finalize = gst_tcam_buffer_pool_finalize;

    bp_class->acquire_buffer = gst_tcam_buffer_pool_acquire_buffer;
    bp_class->release_buffer = gst_tcam_buffer_pool_release_buffer;
    bp_class->start = gst_tcam_buffer_pool_start;
    bp_class->stop = gst_tcam_buffer_pool_stop;
    bp_class->set_config = gst_tcam_buffer_pool_set_config;
}


GstBufferPool* gst_tcam_buffer_pool_new(GstElement* src, GstCaps* caps)
{
    static unsigned int pool_seq;
    /* setting a significant unique name */
    char* parent_name = gst_object_get_name(GST_OBJECT(src));
    char* name = g_strdup_printf("%s:pool%u:%s", parent_name, pool_seq, "src");
    g_free(parent_name);

    pool_seq++;

    GstTcamBufferPool* pool =
        (GstTcamBufferPool*)g_object_new(GST_TYPE_TCAM_BUFFER_POOL, "name", name, NULL);
    g_object_ref_sink(pool);
    g_free(name);


    GstStructure* config = gst_buffer_pool_get_config(GST_BUFFER_POOL_CAST(pool));
    gst_buffer_pool_config_set_params(config, caps, 0, 0, 0);
    /* This will simply set a default config, but will not configure the pool
   * because min and max are not valid */
    gst_buffer_pool_set_config(GST_BUFFER_POOL_CAST(pool), config);

    pool->src_element = src;

    return GST_BUFFER_POOL(pool);
}

void gst_tcam_buffer_pool_set_other_pool(GstTcamBufferPool* pool, GstBufferPool* other_pool)
{
    g_return_if_fail(!gst_buffer_pool_is_active(GST_BUFFER_POOL(pool)));

    if (pool->other_pool_)
    {
        gst_object_unref(pool->other_pool_);
    }
    pool->other_pool_ = (GstBufferPool*)gst_object_ref(other_pool);
}
