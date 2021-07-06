
#include "gst_element_chain.h"

gst_helper::gst_ptr<GstElement> gst_helper::find_upstream_element(
    GstElement* start_element,
    std::function<bool(GstElement*)> pred)
{
    auto cur_element = start_element;

    gst_object_ref(start_element);

    do {
        auto sink_pad = gst_helper::get_static_pad(cur_element, "sink");
        if (!sink_pad)
        {
            //auto name = gst_element_get_name(cur_element);
            //GST_ERROR_OBJECT(
            //    start_element,
            //    "Failed to find source element upstream, because the element %s has no sink pin.",
            //    name);
            //g_free(name);
            return nullptr;
        }

        auto src_pad = gst_helper::get_peer_pad(sink_pad);

        if (!src_pad)
        { // this means we have reached a dead end where no valid tcamsrc exists
            //GST_ERROR_OBJECT(
            //    start_element,
            //    "Failed to find source element upstream, because one sink pad was unconnected");
            return nullptr;
        }

        auto prev_element = gst_pad_get_parent_element(src_pad.get());

        gst_object_unref(cur_element);

        if (pred(prev_element))
        {
            return gst_helper::make_consume_ptr(prev_element);
        }

        cur_element = prev_element;
    } while (cur_element);

    return nullptr;
}
