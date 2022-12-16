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

#include "gsttcammainsrc.h"

#include "../../../libs/gst-helper/include/tcamprop1.0_gobject/tcam_property_serialize.h"
#include "../../../libs/tcam-property/src/tcam-property-1.0.h"
#include "../../logging.h"
#include "../tcamgstbase/tcamgstbase.h"
#include "../tcamgstbase/tcamgststrings.h"
#include "gst/gstvalue.h"
#include "../../../libs/tcam-property/src/gst/meta/gstmetatcamstatistics.h"
#include "gsttcambufferpool.h"
#include "mainsrc_device_state.h"
#include "mainsrc_tcamprop_impl.h"
#include "tcambind.h"

#define GST_TCAM_MAINSRC_DEFAULT_N_BUFFERS 10

GST_DEBUG_CATEGORY(tcam_mainsrc_debug);
#define GST_CAT_DEFAULT tcam_mainsrc_debug

struct destroy_transfer
{
    GstTcamMainSrc* self;
    std::shared_ptr<tcam::ImageBuffer> ptr;
};

static void gst_tcam_mainsrc_close_camera(GstTcamMainSrc* self);

G_DEFINE_TYPE_WITH_CODE(GstTcamMainSrc,
                        gst_tcam_mainsrc,
                        GST_TYPE_PUSH_SRC,
                        G_IMPLEMENT_INTERFACE(TCAM_TYPE_PROPERTY_PROVIDER,
                                              tcam::mainsrc::gst_tcam_mainsrc_tcamprop_init))


GType gst_tcam_io_mode_get_type(void)
{
    static GType tcam_io_mode = 0;

    if (!tcam_io_mode)
    {
        static const GEnumValue io_modes[] = {
            { GST_TCAM_IO_AUTO, "GST_TCAM_IO_AUTO", "auto" },
            { GST_TCAM_IO_MMAP, "GST_TCAM_IO_MMAP", "mmap" },
            { GST_TCAM_IO_USERPTR, "GST_TCAM_IO_USERPTR", "userptr" },
            //{ GST_TCAM_IO_DMABUF, "GST_TCAM_IO_DMABUF", "dmabuf" },
            //{ GST_TCAM_IO_DMABUF_IMPORT, "GST_TCAM_IO_DMABUF_IMPORT", "dmabuf-import" },

            { 0, NULL, NULL }
        };
        tcam_io_mode = g_enum_register_static("GstTcamIOMode", io_modes);
    }
    return tcam_io_mode;
}


enum
{
    SIGNAL_DEVICE_OPEN,
    SIGNAL_DEVICE_CLOSE,
    SIGNAL_LAST,
};

enum
{
    PROP_0,
    PROP_SERIAL,
    PROP_DEVICE_TYPE,
    PROP_CAMERA_BUFFERS,
    PROP_NUM_BUFFERS,
    PROP_IO_MODE,
    PROP_DROP_INCOMPLETE_BUFFER,
    PROP_TCAM_PROPERTIES_GSTSTRUCT,
};

static guint gst_tcammainsrc_signals[SIGNAL_LAST] = {
    0,
};


static GstStaticPadTemplate tcam_mainsrc_template =
    GST_STATIC_PAD_TEMPLATE("src", GST_PAD_SRC, GST_PAD_ALWAYS, GST_STATIC_CAPS("ANY"));

static GstCaps* gst_tcam_mainsrc_fixate_caps(GstBaseSrc* bsrc, GstCaps* caps);


static gboolean gst_tcam_mainsrc_negotiate(GstBaseSrc* self)
{
    GstCaps* caps = NULL;
    gboolean result = FALSE;

    /* first see what is possible on our source pad */
    GstCaps* thiscaps = gst_pad_query_caps(GST_BASE_SRC_PAD(self), NULL);

    // nothing or anything is allowed, we're done
    if (gst_caps_is_empty(thiscaps) || gst_caps_is_any(thiscaps))
    {
        if (thiscaps)
        {
            gst_caps_unref(thiscaps);
        }
        return TRUE;
    }
    /* get the peer caps */
    GstCaps* peercaps = gst_pad_peer_query_caps(GST_BASE_SRC_PAD(self), nullptr);
    GST_DEBUG_OBJECT(self, "caps of peer: %s", gst_helper::to_string(*peercaps).c_str());

    if (!gst_caps_is_empty(peercaps) && !gst_caps_is_any(peercaps))
    {
        GstCaps* tmp = gst_caps_intersect_full(thiscaps, peercaps, GST_CAPS_INTERSECT_FIRST);

        GstCaps* icaps = NULL;

        int caps_count = static_cast<int>(gst_caps_get_size(tmp));

        /* Prefer the first caps we are compatible with that the peer proposed */
        for (int i = caps_count - 1; i >= 0; i--)
        {
            /* get intersection */
            GstCaps* ipcaps = gst_caps_copy_nth(tmp, i);

            /* Sometimes gst_caps_is_any returns FALSE even for ANY caps?!?! */
            bool is_any_caps = gst_helper::to_string(*ipcaps) == "ANY";

            if (gst_caps_is_any(ipcaps) || is_any_caps || gst_caps_is_empty(ipcaps))
            {
                continue;
            }

            //GST_DEBUG("peer: %" GST_PTR_FORMAT, static_cast<void*>(ipcaps));

            icaps = gst_caps_intersect_full(thiscaps, ipcaps, GST_CAPS_INTERSECT_FIRST);
            gst_caps_unref(ipcaps);

            if (icaps && !gst_caps_is_empty(icaps))
            {
                break;
            }
            gst_caps_unref(icaps);
            icaps = NULL;
        }

        //GST_DEBUG("intersect: %" GST_PTR_FORMAT, static_cast<void*>(icaps));

        if (icaps)
        {
            /* If there are multiple intersections pick the one with the smallest
             * resolution strictly bigger then the first peer caps */
            if (gst_caps_get_size(icaps) > 1)
            {
                int best = 0;
                int width = 0, height = 0;

                /* Walk the structure backwards to get the first entry of the
                     * smallest resolution bigger (or equal to) the preferred resolution)
                     */
                for (gint i = (gint)gst_caps_get_size(icaps) - 1; i >= 0; i--)
                {
                    GstStructure* is = gst_caps_get_structure(icaps, i);
                    int w, h;

                    if (gst_structure_get_int(is, "width", &w)
                        && gst_structure_get_int(is, "height", &h))
                    {
                        if (w >= width && h >= height)
                        {
                            width = w;
                            height = h;
                            best = i;
                        }
                    }
                }

                caps = gst_caps_copy_nth(icaps, best);
                gst_caps_unref(icaps);
            }
            else
            {
                // ensure that there is no range but a high resolution with adequate framerate

                int best = 0;
                int twidth = 0, theight = 0;
                int width = G_MAXINT, height = G_MAXINT;

                /* Walk the structure backwards to get the first entry of the
                 * smallest resolution bigger (or equal to) the preferred resolution)
                 */
                for (guint i = 0; i >= gst_caps_get_size(icaps); i++)
                {
                    GstStructure* is = gst_caps_get_structure(icaps, i);
                    int w, h;

                    if (gst_structure_get_int(is, "width", &w)
                        && gst_structure_get_int(is, "height", &h))
                    {
                        if (w >= twidth && w <= width && h >= theight && h <= height)
                        {
                            width = w;
                            height = h;
                            best = i;
                        }
                    }
                }

                /* caps = icaps; */
                caps = gst_caps_copy_nth(icaps, best);

                GstStructure* structure;
                double frame_rate = G_MAXINT;

                structure = gst_caps_get_structure(caps, 0);

                if (gst_structure_has_field(structure, "width"))
                {
                    gst_structure_fixate_field_nearest_int(structure, "width", G_MAXUINT);
                }
                if (gst_structure_has_field(structure, "height"))
                {
                    gst_structure_fixate_field_nearest_int(structure, "height", G_MAXUINT);
                }
                if (gst_structure_has_field(structure, "framerate"))
                {
                    gst_structure_fixate_field_nearest_fraction(
                        structure, "framerate", frame_rate, 1);
                }
                gst_caps_unref(icaps);
            }
        }
        gst_caps_unref(tmp);

        gst_caps_unref(thiscaps);
    }
    else
    {
        /* no peer or peer have ANY caps, work with our own caps then */
        caps = thiscaps;
    }

    if (peercaps)
    {
        gst_caps_unref(peercaps);
    }

    if (caps)
    {
        caps = gst_caps_truncate(caps);

        /* now fixate */
        if (!gst_caps_is_empty(caps))
        {
            caps = gst_tcam_mainsrc_fixate_caps(self, caps);
            GST_DEBUG_OBJECT(self, "fixated to: %" GST_PTR_FORMAT, static_cast<void*>(caps));

            if (gst_caps_is_any(caps))
            {
                /* hmm, still anything, so element can do anything and
                 * nego is not needed */
                result = TRUE;
            }
            else if (gst_caps_is_fixed(caps))
            {
                /* yay, fixed caps, use those then */
                result = gst_base_src_set_caps(self, caps);
            }
        }
        gst_caps_unref(caps);
    }
    return result;
}


static GstCaps* gst_tcam_mainsrc_get_caps(GstBaseSrc* src, GstCaps* filter __attribute__((unused)))
{
    GstTcamMainSrc* self = GST_TCAM_MAINSRC(src);

    auto caps = self->device->get_device_caps();
    if (caps == nullptr)
    {
        GST_WARNING_OBJECT(self, "Device not initialized. Must be in state >= GST_STATE_READY.");
        return nullptr;
    }
    return caps;
}


static gboolean gst_tcam_mainsrc_set_caps(GstBaseSrc* src, GstCaps* caps)
{
    GstTcamMainSrc* self = GST_TCAM_MAINSRC(src);

    GST_DEBUG_OBJECT(self, "Requested caps = %s", gst_helper::to_string(*caps).c_str());

    // TODO: move to somewhere more sensible?
    self->device->stop_and_clear();
    self->device->sink = nullptr;

    tcam::tcam_video_format format = {};

    if (!tcam::mainsrc::caps_to_format(*caps, format))
    {
        GST_ERROR("Unable to interpret caps. Aborting");
        return false;
    }

    self->fps = format.framerate;

    self->device->format_ = tcam::VideoFormat(format);
    if (!self->device->device_->set_video_format(tcam::VideoFormat(format)))
    {
        GST_ERROR_OBJECT(self, "Unable to set format in device");

        return FALSE;
    }

    // self->device->device_->set_drop_incomplete_frames(state.drop_incomplete_frames_);

    // self->device->is_streaming_ = true;
    GST_INFO_OBJECT(self, "Successfully set caps to: %s", gst_helper::to_string(*caps).c_str());

    return TRUE;
}


static void gst_tcam_mainsrc_device_lost_callback(const tcam::tcam_device_info* info
                                                  __attribute__((unused)),
                                                  void* user_data)
{
    GstTcamMainSrc* self = (GstTcamMainSrc*)user_data;

    GstState state;

    // wait for 1 seconds max
    gst_element_get_state(GST_ELEMENT(self), &state, nullptr, 1000000000);

    if (!self->device || state == GST_STATE_NULL)
    {
        // device does not exist
        // or source is null (aka no device exists)
        // do nothing
        return;
    }

    if (!self->device->is_streaming_)
    {
        return;
    }

    auto serial = self->device->get_device_serial();

    // set serial as args entry and in actual message
    // that way users have multiple ways of accessing the serial
    GST_ELEMENT_ERROR_WITH_DETAILS(GST_ELEMENT(self),
                                   RESOURCE,
                                   NOT_FOUND,
                                   ("Device lost (%s)", serial.c_str()),
                                   ((nullptr)),
                                   ("serial", G_TYPE_STRING, serial.c_str(), nullptr));

    self->device->is_streaming_ = false;

    // the device is considered lost.
    // might as well inform via all possible channels to keep
    // property queries, etc from appearing while everything is shutting down
    g_signal_emit(G_OBJECT(self), gst_tcammainsrc_signals[SIGNAL_DEVICE_CLOSE], 0);

    // do not send EOS here
    // this can cause a deadlock in the tcambin state handling
    // EOS will be triggered in mainsrc_create and transmitted through the capture thread
    // this has been triggered by setting self->device->is_running to false

    // gst_element_send_event(GST_ELEMENT(self), gst_event_new_eos());


    // do not call stop
    // some users experience segfaults
    // let EOS handle this. gstreamer will call stop for us

    // gst_tcam_mainsrc_stop(GST_BASE_SRC(self));
}

static bool gst_tcam_mainsrc_init_camera(GstTcamMainSrc* self)
{
    if (!self->device->open_camera())
    {
        return false;
    }

    // this cb will automatically be deleted once the device is closed
    // no need for explicit cleanup
    self->device->device_->register_device_lost_callback(gst_tcam_mainsrc_device_lost_callback,
                                                         self);

    // emit a signal to let other elements/users know that a device has been opened
    // and properties, etc are now usable
    g_signal_emit(G_OBJECT(self), gst_tcammainsrc_signals[SIGNAL_DEVICE_OPEN], 0);

    return true;
}

static void gst_tcam_mainsrc_close_camera(GstTcamMainSrc* self)
{
    g_signal_emit(G_OBJECT(self), gst_tcammainsrc_signals[SIGNAL_DEVICE_CLOSE], 0);

    self->device->close();
}

static GstStateChangeReturn gst_tcam_mainsrc_change_state(GstElement* element,
                                                          GstStateChange change)
{
    GstStateChangeReturn ret = GST_STATE_CHANGE_SUCCESS;

    GstTcamMainSrc* self = GST_TCAM_MAINSRC(element);

    switch (change)
    {
        case GST_STATE_CHANGE_NULL_TO_READY:
        {
            if (!self->device->is_device_open())
            {
                if (!gst_tcam_mainsrc_init_camera(self))
                {
                    return GST_STATE_CHANGE_FAILURE;
                }
            }
            break;
        }
        case GST_STATE_CHANGE_READY_TO_PAUSED:
        {
            self->device->n_buffers_delivered_ = 0;
            ret = GST_STATE_CHANGE_NO_PREROLL;
            break;
        }
        case GST_STATE_CHANGE_PAUSED_TO_PLAYING:
        {
            self->device->is_streaming_ = true;
            self->device->stream_cv_.notify_all();
            break;
        }
        default:
        {
            break;
        }
    }

    gst_element_set_locked_state(element, TRUE);
    ret = GST_ELEMENT_CLASS(gst_tcam_mainsrc_parent_class)->change_state(element, change);
    gst_element_set_locked_state(element, FALSE);

    if (ret == GST_STATE_CHANGE_FAILURE)
    {
        return ret;
    }

    switch (change)
    {
        case GST_STATE_CHANGE_PLAYING_TO_PAUSED:
        {
            self->device->is_streaming_ = false;
            self->device->stream_cv_.notify_all();
            ret = GST_STATE_CHANGE_NO_PREROLL;
            break;
        }
        case GST_STATE_CHANGE_PAUSED_TO_READY:
        {
            bool stop_ret = self->device->device_->stop_stream();
            if (!stop_ret)
            {
                GST_ERROR("Could not stop stream.");
            }
            if (self->pool)
            {
                gst_buffer_pool_set_active(self->pool, FALSE);

                gst_tcam_buffer_pool_delete_buffer(GST_TCAM_BUFFER_POOL(self->pool));

                if (self->pool->object.object.ref_count > 1)
                {
                    // gst_query_add_allocation_pool increases the ref count
                    // but does not decrease it
                    // get rid of the query reference
                    // so that our (actual) unref later on causes a cleanup of the pool
                    gst_object_unref(self->pool);
                }
                gst_object_unref(self->pool);

                self->pool = nullptr;
            }
            break;
        }
        case GST_STATE_CHANGE_READY_TO_NULL:
        {
            if (self->device->is_device_open())
            {
                gst_tcam_mainsrc_close_camera(self);
            }

            break;
        }
        default:
            break;
    }
    return ret;
}


static GstFlowReturn gst_tcam_mainsrc_create(GstPushSrc* push_src, GstBuffer** buffer)
{
    GstTcamMainSrc* self = GST_TCAM_MAINSRC(push_src);

    if (self->device->n_buffers_ != -1)
    {
        /*
              TODO: self->n_buffers should have same type as ptr->get_statistics().frame_count
            */
        if (self->device->n_buffers_delivered_ >= (guint)self->device->n_buffers_)
        {
            GST_INFO_OBJECT(
                self,
                "Stopping stream after %llu buffers.",
                static_cast<long long unsigned int>(self->device->n_buffers_delivered_));
            return GST_FLOW_EOS;
        }
        self->device->n_buffers_delivered_++;
    }

start_create:

    GstBufferPool* src_pool = gst_base_src_get_buffer_pool(GST_BASE_SRC(push_src));

    if (src_pool)
    {
        // this blocks until we get a buffer
        GstFlowReturn pool_ret = gst_buffer_pool_acquire_buffer(src_pool, buffer, nullptr);

        if (pool_ret == GST_FLOW_FLUSHING)
        {
            return GST_FLOW_FLUSHING;
        }

        if (!*buffer)
        {
            goto start_create;
        }

        gst_object_unref(src_pool);
    }
    /* TODO: check why aravis throws an incomplete buffer error
       but the received images are still valid */
    // if (!tcam::is_image_buffer_complete(self->ptr))
    // {
    //     GST_DEBUG_OBJECT (self, "Received incomplete buffer. Returning to waiting position.");

    //     goto wait_again;
    // }

    return GST_FLOW_OK;
}


static GstCaps* gst_tcam_mainsrc_fixate_caps(GstBaseSrc* bsrc, GstCaps* caps)
{
    GstTcamMainSrc* self = GST_TCAM_MAINSRC(bsrc);

    GstStructure* structure = nullptr;
    gint width = 0;
    gint height = 0;
    double frame_rate = 0.0;

    structure = gst_caps_get_structure(
        caps,
        0); // #TODO this seems to be at best curious, at another place we fixate to highest, and here fixate goes to lowest

    if (gst_structure_has_field(structure, "width"))
    {
        gst_structure_fixate_field_nearest_int(structure, "width", width);
    }
    if (gst_structure_has_field(structure, "height"))
    {
        gst_structure_fixate_field_nearest_int(structure, "height", height);
    }
    if (gst_structure_has_field(structure, "framerate"))
    {
        gst_structure_fixate_field_nearest_fraction(
            structure, "framerate", (double)(0.5 + frame_rate), 1);
    }

    GST_DEBUG_OBJECT(self, "Fixated caps to %s", gst_helper::to_string(*caps).c_str());

    return GST_BASE_SRC_CLASS(gst_tcam_mainsrc_parent_class)->fixate(bsrc, caps);
}


// Returns true, when the query could be answered, otherwise returns false
static bool fetch_framerate_via_query_caps_worker(device_state& device,
                                                  GstQuery* query,
                                                  GstCaps* query_caps)
{
    GstStructure* structure = gst_caps_get_structure(query_caps, 0);

    int height = 0;
    int width = 0;

    if (!gst_structure_get_int(structure, "width", &width) || width <= 0)
    {
        GST_WARNING("Failed to fetch 'width' from GstCaps structure.");
        return false;
    }
    if (!gst_structure_get_int(structure, "height", &height) || height <= 0)
    {
        GST_WARNING("Failed to fetch 'width' from GstCaps structure.");
        return false;
    }

    const char* format_string = gst_structure_get_string(structure, "format");
    if (format_string == nullptr)
    {
        GST_WARNING("Failed to fetch 'format' from GstCaps structure.");
        return false;
    }

    tcam::image_scaling scale_info = tcam::gst::caps_get_scaling(query_caps);

    uint32_t fourcc = tcam::gst::tcam_fourcc_from_gst_1_0_caps_string(
        gst_structure_get_name(structure), format_string);

    auto format_list = device.device_->get_available_video_formats();

    auto fps_res = device.device_->get_framerate_info(
        tcam ::VideoFormat { fourcc, { (uint32_t)width, (uint32_t)height }, scale_info });
    if (fps_res.has_error())
    {
        GST_ERROR("Failed to get framerates for '%s (%dx%d)' from device.",
                  format_string, width, height);
        return false;
    }

    const auto& fps_list = fps_res.value();
    GValue fps_val = G_VALUE_INIT;

    if (!fps_list.is_discrete_list())
    {
        int fps_min_num = 0;
        int fps_min_den = 0;
        int fps_max_num = 0;
        int fps_max_den = 0;
        gst_util_double_to_fraction(fps_list.min(), &fps_min_num, &fps_min_den);
        gst_util_double_to_fraction(fps_list.max(), &fps_max_num, &fps_max_den);

        g_value_init(&fps_val, GST_TYPE_FRACTION_RANGE);

        gst_value_set_fraction_range_full(&fps_val, fps_min_num, fps_min_den, fps_max_num, fps_max_den);
    }
    else
    {
        g_value_init(&fps_val, GST_TYPE_LIST);
        for (const auto& fps : fps_list.to_list())
        {
            int frame_rate_numerator = 0;
            int frame_rate_denominator = 0;
            gst_util_double_to_fraction(fps, &frame_rate_numerator, &frame_rate_denominator);

            GValue fraction = G_VALUE_INIT;
            g_value_init(&fraction, GST_TYPE_FRACTION);
            gst_value_set_fraction(&fraction, frame_rate_numerator, frame_rate_denominator);

            gst_value_list_append_value(&fps_val, &fraction);
            g_value_unset(&fraction);
        }
    }
    GstCaps* result_caps = gst_caps_copy(query_caps);

    gst_caps_set_value(result_caps, "framerate", &fps_val);

    gst_query_set_caps_result(query, result_caps);
    return true;
}


static gboolean gst_tcam_mainsrc_query(GstBaseSrc* bsrc, GstQuery* query)
{
    GstTcamMainSrc* self = GST_TCAM_MAINSRC(bsrc);
    gboolean res = FALSE;

    switch (GST_QUERY_TYPE(query))
    {
        case GST_QUERY_LATENCY:
        {
            GstClockTime min_latency;
            GstClockTime max_latency;

            /* device must be open */
            if (!self->device->is_device_open())
            {
                GST_WARNING_OBJECT(self, "Can't give latency since device isn't open !");
                goto done;
            }

            /* we must have a framerate */
            if (self->fps == 0.0)
            {
                GST_WARNING_OBJECT(self, "Can't give latency since framerate isn't fixated !");
                goto done;
            }

            /* min latency is the time to capture one frame/field */
            min_latency = gst_util_gdouble_to_guint64(GST_SECOND / self->fps);

            /* max latency is set to NONE because cameras may enter trigger mode
               and not deliver images for an unspecified amount of time */
            max_latency = GST_CLOCK_TIME_NONE;

            GST_DEBUG_OBJECT(bsrc,
                             "report latency min %" GST_TIME_FORMAT " max %" GST_TIME_FORMAT,
                             GST_TIME_ARGS(min_latency),
                             GST_TIME_ARGS(max_latency));

            /* we are always live, the min latency is 1 frame and the max latency is
             * the complete buffer of frames. */
            gst_query_set_latency(query, TRUE, min_latency, max_latency);

            res = TRUE;
            break;
        }
        case GST_QUERY_ACCEPT_CAPS:
        {

            if (!self->device->is_device_open())
            {
                GST_ERROR("Can't query accept caps since device isn't open !");
                goto done;
            }

            GstCaps* c = nullptr;
            gst_query_parse_accept_caps(query, &c);

            tcam::tcam_video_format format;

            if (tcam::mainsrc::caps_to_format(*c, format))
            {
                tcam::VideoFormat fmt(format);
                auto formats = self->device->device_->get_available_video_formats();

                for (const auto& f : formats)
                {
                    if (f.is_compatible(fmt))
                    {
                        gst_query_set_accept_caps_result(query, TRUE);
                        return TRUE;
                    }
                }
                gst_query_set_accept_caps_result(query, FALSE);
                return TRUE;
            }
            else
            {
                gst_query_set_accept_caps_result(query, FALSE);
                return TRUE;
            }
        }
        case GST_QUERY_CAPS:
        {
            GstCaps* query_caps = nullptr;
            gst_query_parse_caps(query, &query_caps);

            if (!query_caps || gst_caps_is_empty(query_caps))
            {
                return GST_BASE_SRC_CLASS(gst_tcam_mainsrc_parent_class)->query(bsrc, query);
            }

            // these queries require device interaction
            // no device means we return FALSE

            if (!self->device->is_device_open())
            {
                GST_ERROR_OBJECT(
                    self,
                    "device must be open to answer query. Ensure element is in state READY "
                    "or higher.");
                return FALSE;
            }

            // check what is missing and complete accordingly

            auto has_field = [](const GstCaps& caps, const std::string& name) -> bool
            {
                GstStructure* struc = gst_caps_get_structure(&caps, 0);

                return gst_structure_has_field(struc, name.c_str());
            };

            gst_helper::gst_ptr<GstCaps> tmp = nullptr;

            // we have a generic query
            // let the base src handle this
            // basesrc basically calls get_caps(filter=query_caps)
            if (!has_field(*query_caps, "format")
                || !has_field(*query_caps, "width")
                || !has_field(*query_caps, "height"))
            {
                return GST_BASE_SRC_CLASS(gst_tcam_mainsrc_parent_class)->query(bsrc, query);
            }
            else
            {
                tmp = gst_helper::make_consume_ptr(gst_caps_copy(query_caps));
            }

            if (!has_field(*query_caps, "framerate"))
            {
                return fetch_framerate_via_query_caps_worker(*self->device, query, tmp.get());
            }

            GST_DEBUG("Unsure about caps query. Falling back to default handling.");

            // fallthrough, use basic behavior
            return GST_BASE_SRC_CLASS(gst_tcam_mainsrc_parent_class)->query(bsrc, query);
        }
        default:
        {
            res = GST_BASE_SRC_CLASS(gst_tcam_mainsrc_parent_class)->query(bsrc, query);
            break;
        }
    }

done:

    return res;
}

static void gst_tcam_mainsrc_init(GstTcamMainSrc* self)
{
    gst_base_src_set_live(GST_BASE_SRC(self), TRUE);
    gst_base_src_set_format(GST_BASE_SRC(self), GST_FORMAT_TIME);

    self->device = new device_state(self);

    // this has to be defined in set_caps
    self->fps = 0.0;
}


static void gst_tcam_mainsrc_finalize(GObject* object)
{
    GstTcamMainSrc* self = GST_TCAM_MAINSRC(object);

    gst_tcam_mainsrc_close_camera(self);

    if (self->device)
    {
        delete self->device;
        self->device = nullptr;
    }

    if (self->pool)
    {
        if (self->pool)
            gst_buffer_pool_set_active(self->pool, FALSE);
        gst_object_unref(self->pool);
        self->pool = nullptr;
    }

    G_OBJECT_CLASS(gst_tcam_mainsrc_parent_class)->finalize(object);
}


static bool is_state_null(GstTcamMainSrc* self)
{
    return tcam::gst::is_gst_state_equal_or_less(GST_ELEMENT(self), GST_STATE_NULL);
}

static bool is_state_ready_or_lower(GstTcamMainSrc* self)
{
    return tcam::gst::is_gst_state_equal_or_less(GST_ELEMENT(self), GST_STATE_READY);
}


static void gst_tcam_mainsrc_set_property(GObject* object,
                                          guint prop_id,
                                          const GValue* value,
                                          GParamSpec* pspec)
{
    GstTcamMainSrc* self = GST_TCAM_MAINSRC(object);

    auto& state = *self->device;

    switch (prop_id)
    {
        case PROP_SERIAL:
        {
            if (!is_state_null(self))
            {
                GST_ERROR_OBJECT(
                    self, "GObject property 'serial' is not writable in state >= GST_STATE_READY.");
                return;
            }
            if (g_value_get_string(value) == nullptr)
            {
                state.set_device_serial(std::string {});
            }
            else
            {
                std::string string_value = g_value_get_string(value);

                auto [s, t] = tcambind::separate_serial_and_type(string_value);
                if (!t.empty())
                {
                    state.set_device_serial(s);
                    state.set_device_type(tcam::tcam_device_from_string(t));

                    GST_INFO_OBJECT(
                        self,
                        "Set camera serial to '%s', Type to '%s'. (from %s).",
                        state.get_device_serial().c_str(),
                        tcam::tcam_device_type_to_string(state.get_device_type()).c_str(),
                        string_value.c_str());
                }
                else
                {
                    state.set_device_serial(s);
                }
            }
            break;
        }
        case PROP_DEVICE_TYPE:
        {
            if (!is_state_null(self))
            {
                GST_ERROR_OBJECT(
                    self, "GObject property 'type' is not writable in state >= GST_STATE_READY.");
                return;
            }

            const char* type = g_value_get_string(value);
            if (!type)
            {
                state.set_device_type(tcam::TCAM_DEVICE_TYPE_UNKNOWN);
            }
            else
            {
                std::string type_str = type;

                // this check is simply for messaging the user about invalid values
                auto vec = tcam::get_device_type_list_strings();
                if (std::find(vec.begin(), vec.end(), type_str) == vec.end())
                {
                    GST_ERROR_OBJECT(self, "Unknown device type '%s'", type);
                    state.set_device_type(tcam::TCAM_DEVICE_TYPE_UNKNOWN);
                }
                else
                {
                    state.set_device_type(tcam::tcam_device_from_string(type_str));
                }
            }
            break;
        }
        case PROP_CAMERA_BUFFERS:
        {
            if (!is_state_ready_or_lower(self))
            {
                GST_ERROR_OBJECT(self,
                                 "GObject property 'camera-buffers' is not writable in state >= "
                                 "GST_STATE_PAUSED.");
                return;
            }
            else
            {
                state.imagesink_buffers_ = g_value_get_int(value);
            }
            break;
        }
        case PROP_IO_MODE:
        {
            state.io_mode_ = (GstTcamIOMode)g_value_get_enum(value);
            break;
        }
        case PROP_NUM_BUFFERS:
        {
            if (!is_state_ready_or_lower(self))
            {
                GST_ERROR_OBJECT(self,
                                 "GObject property 'num-buffers' is not writable in state >= "
                                 "GST_STATE_PAUSED.");
            }
            else
            {
                state.n_buffers_ = g_value_get_int(value);
            }
            break;
        }
        case PROP_DROP_INCOMPLETE_BUFFER:
        {
            state.drop_incomplete_frames_ = g_value_get_boolean(value) != FALSE;
            if (self->device->device_)
            {
                self->device->device_->set_drop_incomplete_frames(state.drop_incomplete_frames_);
            }
            break;
        }
        case PROP_TCAM_PROPERTIES_GSTSTRUCT:
        {
            auto strc = gst_value_get_structure(value);
            self->device->set_tcam_properties(strc);
            break;
        }
        default:
        {
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
            break;
        }
    }
}


static void gst_tcam_mainsrc_get_property(GObject* object,
                                          guint prop_id,
                                          GValue* value,
                                          GParamSpec* pspec)
{
    GstTcamMainSrc* self = GST_TCAM_MAINSRC(object);
    auto& state = *self->device;

    switch (prop_id)
    {
        case PROP_SERIAL:
        {
            g_value_set_string(value, state.get_device_serial().c_str());
            break;
        }
        case PROP_DEVICE_TYPE:
        {
            g_value_set_string(value,
                               tcam::tcam_device_type_to_string(state.get_device_type()).c_str());

            break;
        }
        case PROP_CAMERA_BUFFERS:
        {
            g_value_set_int(value, state.imagesink_buffers_);
            break;
        }
        case PROP_NUM_BUFFERS:
        {
            g_value_set_int(value, state.n_buffers_);
            break;
        }
        case PROP_IO_MODE:
        {
            g_value_set_enum(value, state.io_mode_);
            break;
        }
        case PROP_DROP_INCOMPLETE_BUFFER:
        {
            g_value_set_boolean(value, state.drop_incomplete_frames_);
            break;
        }
        case PROP_TCAM_PROPERTIES_GSTSTRUCT:
        {
            gst_helper::gst_ptr<GstStructure> ptr = self->device->get_tcam_properties();
            gst_value_set_structure(value, ptr.get());
            break;
        }
        default:
        {
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
            break;
        }
    }
}


static gboolean tcam_mainsrc_decide_allocation(GstBaseSrc* base_src, GstQuery* query)
{
    GstTcamMainSrc* self = GST_TCAM_MAINSRC(base_src);

    if (GST_QUERY_TYPE(query) != GST_QUERY_ALLOCATION)
    {
        return FALSE;
    }

    GstCaps* caps = gst_pad_get_current_caps(GST_BASE_SRC_PAD(base_src));

    GST_DEBUG("Allocation caps: %s", gst_caps_to_string(caps));

    tcam::tcam_video_format format;
    tcam::mainsrc::caps_to_format(*caps, format);

    gst_caps_unref(caps);


    GstBufferPool* src_pool = gst_base_src_get_buffer_pool(GST_BASE_SRC(base_src));

    if (src_pool)
    {
        // these steps are taking to prevent a reconfigure

        if (gst_buffer_pool_is_active(self->pool))
        {
            GstBufferPool* pool = src_pool;
            if (gst_query_get_n_allocation_pools(query))
            {
                gst_query_set_nth_allocation_pool(
                    query, 0, pool, self->device->format_.get_required_buffer_size(), 1, 0);
            }
            else
            {
                gst_query_add_allocation_pool(
                    query, pool, self->device->format_.get_required_buffer_size(), 1, 0);
            }
        }

        gst_object_unref(src_pool);
    }
    else
    {
        if (self->pool)
        {
            gst_buffer_pool_set_active(self->pool, FALSE);

            gst_object_unref(self->pool);
            self->pool = nullptr;
        }
        self->pool = gst_tcam_buffer_pool_new(GST_ELEMENT(self), caps);
        unsigned int size = 10;

        auto* config = gst_buffer_pool_get_config(self->pool);

        gst_buffer_pool_config_set_params(config, caps, tcam::VideoFormat(format).get_required_buffer_size(), 10, 10);
        gst_buffer_pool_set_config(self->pool, config);

        if (gst_query_get_n_allocation_pools(query))
        {
            gst_query_set_nth_allocation_pool(
                query, 0, self->pool, self->device->imagesink_buffers_, 1, 0);
        }
        else
        {
            gst_query_add_allocation_pool(query, self->pool, size, self->device->imagesink_buffers_, 0);
        }

        self->device->device_->set_drop_incomplete_frames(self->device->drop_incomplete_frames_);


        self->device->format_ = tcam::VideoFormat(format);

        self->device->is_streaming_ = true;

        return GST_BASE_SRC_CLASS(gst_tcam_mainsrc_parent_class)->decide_allocation(base_src, query);
    }
    return TRUE;
}


static void gst_tcam_mainsrc_class_init(GstTcamMainSrcClass* klass)
{
    GObjectClass* gobject_class = G_OBJECT_CLASS(klass);
    GstElementClass* element_class = GST_ELEMENT_CLASS(klass);
    GstBaseSrcClass* gstbasesrc_class = GST_BASE_SRC_CLASS(klass);
    GstPushSrcClass* gstpushsrc_class = GST_PUSH_SRC_CLASS(klass);

    gobject_class->finalize = gst_tcam_mainsrc_finalize;
    gobject_class->set_property = gst_tcam_mainsrc_set_property;
    gobject_class->get_property = gst_tcam_mainsrc_get_property;

    g_object_class_install_property(
        gobject_class,
        PROP_SERIAL,
        g_param_spec_string("serial",
                            "Camera serial",
                            "Serial of the camera",
                            NULL,
                            static_cast<GParamFlags>(G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS)));

    g_object_class_install_property(
        gobject_class,
        PROP_DEVICE_TYPE,
        g_param_spec_string("type",
                            "Camera type",
                            "type/backend of the camera",
                            "auto",
                            static_cast<GParamFlags>(G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS)));

    g_object_class_install_property(
        gobject_class,
        PROP_CAMERA_BUFFERS,
        g_param_spec_int("camera-buffers",
                         "Number of Buffers",
                         "Number of buffers to use for retrieving images",
                         1,
                         256,
                         GST_TCAM_MAINSRC_DEFAULT_N_BUFFERS,
                         static_cast<GParamFlags>(G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS)));

    g_object_class_install_property(
        gobject_class,
        PROP_IO_MODE,
        g_param_spec_enum("io-mode",
                          "IO Mode",
                          "",
                          GST_TYPE_TCAM_IO_MODE,
                          0,
                          static_cast<GParamFlags>(G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS)));

    g_object_class_install_property(
        gobject_class,
        PROP_NUM_BUFFERS,
        g_param_spec_int("num-buffers",
                         "Number of Buffers",
                         "Number of buffers to send before ending pipeline (-1 = unlimited)",
                         -1,
                         G_MAXINT,
                         -1,
                         static_cast<GParamFlags>(G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS)));
    g_object_class_install_property(
        gobject_class,
        PROP_DROP_INCOMPLETE_BUFFER,
        g_param_spec_boolean("drop-incomplete-buffer",
                             "Drop incomplete buffers",
                             "Drop buffer that are incomplete.",
                             true,
                             static_cast<GParamFlags>(G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS
                                                      | G_PARAM_CONSTRUCT)));

    g_object_class_install_property(
        gobject_class,
        PROP_TCAM_PROPERTIES_GSTSTRUCT,
        g_param_spec_boxed(
            "tcam-properties",
            "Properties via GstStructure",
            "In GST_STATE_NULL, sets the initial values for tcam-property 1.0 properties."
            "In GST_STATE_READY, sets the current properties of the device, or reads the current "
            "state of all properties"
            "Names and types are the ones found in the tcam-property 1.0 interface."
            "(Usage e.g.: 'gst-launch-1.0 tcammainsrc "
            "tcam-properties=tcam,ExposureAuto=Off,ExposureTime=33333 ! ...')",
            GST_TYPE_STRUCTURE,
            static_cast<GParamFlags>(G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS)));

    gst_tcammainsrc_signals[SIGNAL_DEVICE_OPEN] = g_signal_new("device-open",
                                                               G_TYPE_FROM_CLASS(klass),
                                                               G_SIGNAL_RUN_LAST,
                                                               0,
                                                               nullptr,
                                                               nullptr,
                                                               nullptr,
                                                               G_TYPE_NONE,
                                                               0,
                                                               G_TYPE_NONE);
    gst_tcammainsrc_signals[SIGNAL_DEVICE_CLOSE] = g_signal_new("device-close",
                                                                G_TYPE_FROM_CLASS(klass),
                                                                G_SIGNAL_RUN_LAST,
                                                                0,
                                                                nullptr,
                                                                nullptr,
                                                                nullptr,
                                                                G_TYPE_NONE,
                                                                0,
                                                                G_TYPE_NONE);

    GST_DEBUG_CATEGORY_INIT(tcam_mainsrc_debug, "tcammainsrc", 0, "tcam interface");

    gst_element_class_set_static_metadata(element_class,
                                          "Tcam Video Source",
                                          "Source/Video",
                                          "Tcam based source",
                                          "The Imaging Source <support@theimagingsource.com>");

    gst_element_class_add_pad_template(element_class,
                                       gst_static_pad_template_get(&tcam_mainsrc_template));

    element_class->change_state = gst_tcam_mainsrc_change_state;

    gstbasesrc_class->get_caps = gst_tcam_mainsrc_get_caps;
    gstbasesrc_class->set_caps = gst_tcam_mainsrc_set_caps;
    gstbasesrc_class->fixate = gst_tcam_mainsrc_fixate_caps;
    gstbasesrc_class->negotiate = gst_tcam_mainsrc_negotiate;
    gstbasesrc_class->query = gst_tcam_mainsrc_query;
    gstbasesrc_class->decide_allocation = tcam_mainsrc_decide_allocation;

    gstpushsrc_class->create = gst_tcam_mainsrc_create;
}
