
#pragma once

#include "gst_ptr.h"

#include <memory>
#include <string>
#include <vector>
#include <cassert>

namespace gst_helper
{
std::string get_type_name(GstElement* element);
std::string to_string(const GstCaps* caps);
std::string to_string( const gst_ptr<GstCaps>& caps );

gst_ptr<GstPad> get_static_pad( GstElement* elem, std::string name );
gst_ptr<GstPad> get_static_pad( const gst_ptr<GstElement>& elem, std::string name );
gst_ptr<GstPad> get_peer_pad( GstPad* pad ) noexcept;
gst_ptr<GstPad> get_peer_pad( const gst_ptr<GstPad>& pad ) noexcept;
gst_ptr<GstCaps> query_caps( GstPad* pad ) noexcept;
gst_ptr<GstCaps> query_caps(const gst_ptr<GstPad>& pad) noexcept;
bool caps_empty_or_any( const gst_ptr<GstCaps>& caps ) noexcept;

std::vector<std::string> gst_string_list_to_vector( const GValue* gst_list );

inline gst_ptr<GstPad> get_static_pad(GstElement* elem, std::string name)
{
    return gst_ptr<GstPad>::wrap(gst_element_get_static_pad(elem, name.c_str()));
}
inline gst_ptr<GstPad> get_static_pad(const gst_ptr<GstElement>& elem, std::string name)
{
    return get_static_pad(elem.get(), name);
}

inline gst_ptr<GstPad> get_peer_pad(GstPad* pad) noexcept
{
    return gst_ptr<GstPad>::wrap(::gst_pad_get_peer(pad));
}

inline gst_ptr<GstPad> get_peer_pad(const gst_ptr<GstPad>& pad) noexcept
{
    return get_peer_pad(pad.get());
}

inline gst_ptr<GstCaps> query_caps(GstPad* pad) noexcept
{
    return gst_ptr<GstCaps>::wrap( gst_pad_query_caps(pad, NULL) );
}
inline gst_ptr<GstCaps> query_caps(const gst_ptr<GstPad>& pad) noexcept
{
    return query_caps(pad.get());
}


inline std::string to_string(const GstCaps* caps)
{
    auto tmp = gst_caps_to_string(caps);
    if (tmp == nullptr)
    {
        return {};
    }

    std::string rval = tmp;
    g_free(tmp);
    return rval;
}
inline std::string to_string(const gst_ptr<GstCaps>& caps)
{
    return to_string(caps.get());
}

inline bool caps_empty_or_any(const gst_ptr<GstCaps>& caps) noexcept
{
    return gst_caps_is_any(caps.get()) || gst_caps_is_empty(caps.get());
}

inline std::string get_type_name(GstElement* element)
{
    // this does not leak memory!!
    const char* name =
        g_type_name(gst_element_factory_get_element_type(gst_element_get_factory(element)));
    return name;
}

inline std::vector<std::string> gst_string_list_to_vector(const GValue* gst_list)
{
    if (!GST_VALUE_HOLDS_LIST(gst_list))
    {
        GST_ERROR("Given GValue is not a list.");
        return {};
    }

    unsigned int gst_list_size = gst_value_list_get_size(gst_list);

    std::vector<std::string> ret;
    ret.reserve(gst_list_size);
    for (unsigned int i = 0; i < gst_list_size; ++i)
    {
        const GValue* val = gst_value_list_get_value(gst_list, i);
        if (G_VALUE_TYPE(val) == G_TYPE_STRING)
        {
            ret.push_back(g_value_get_string(val));
        }
        else
        {
            assert( G_VALUE_TYPE( val ) == G_TYPE_STRING );
        }
    }
    return ret;
}
} // namespace gst_helper