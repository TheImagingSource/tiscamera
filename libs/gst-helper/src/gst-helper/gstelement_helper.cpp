
#include <gst-helper/gstelement_helper.h>
#include <gst-helper/helper_functions.h>

#include <limits>

gst_helper::gst_ptr<GstElement> gst_helper::find_upstream_element(
    GstElement& start_element,
    std::function<bool(GstElement&)> pred)
{
    auto cur_element = &start_element;

    gst_object_ref(&start_element);

    do {
        auto sink_pad = gst_helper::get_static_pad(*cur_element, "sink");
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

        auto src_pad = gst_helper::get_peer_pad(*sink_pad);

        if (!src_pad)
        { // this means we have reached a dead end where no valid tcamsrc exists
            //GST_ERROR_OBJECT(
            //    start_element,
            //    "Failed to find source element upstream, because one sink pad was unconnected");
            return nullptr;
        }

        auto prev_element = gst_pad_get_parent_element(src_pad.get());

        gst_object_unref(cur_element);

        if (pred(*prev_element))
        {
            return gst_helper::make_consume_ptr(prev_element);
        }

        cur_element = prev_element;

    } while (cur_element);

    return nullptr;
}

bool gst_helper::has_connected_element_upstream( const GstElement& element )
{
    auto sink_pad = gst_helper::make_ptr( gst_element_get_static_pad( const_cast<GstElement*>( &element ), "sink" ) );
    if( sink_pad == nullptr ) {
        return false;
    }

    auto src_pad = gst_helper::make_ptr( gst_pad_get_peer( sink_pad.get() ) );

    bool is_connected = src_pad != nullptr;

    return is_connected;
}

std::string gst_helper::get_plugin_version_from_gst_element( const GstElement& element )
{
    // [transfer:none]
    GstElementFactory* camera_factory = gst_element_get_factory( const_cast<GstElement*>(&element) );
    if( camera_factory == nullptr ) {
        return {};
    }
    auto plugin = gst_helper::make_consume_ptr( gst_plugin_feature_get_plugin( GST_PLUGIN_FEATURE( camera_factory ) ) );
    if( plugin == nullptr ) {
        return {};
    }
    const gchar* name = gst_plugin_get_version( plugin.get() );
    if( name == nullptr ) {
        return {};
    }
    std::string rval = name;
    return rval;
}
