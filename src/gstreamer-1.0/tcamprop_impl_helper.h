#pragma once

namespace tcamprop_impl_helper
{
enum class prop_types
{
    boolean,
    integer,
    real,
    button
};

struct prop_entry
{
    guint prop_id;
    const char* prop_name;
    prop_types type;
    const char* category;
    const char* group;
};

inline const char* to_string(prop_types t) noexcept
{
    switch (t)
    {
        case prop_types::boolean:
            return "boolean";
        case prop_types::integer:
            return "integer";
        case prop_types::real:
            return "double";
        case prop_types::button:
            return "button";
        default:
            return nullptr;
    };
}


inline void fill_bool(GValue* val, bool value_to_write)
{
    if (val)
    {
        g_value_init(val, G_TYPE_BOOLEAN);
        g_value_set_boolean(val, value_to_write ? TRUE : FALSE);
    }
}
inline void fill_int(GValue* val, int value_to_write)
{
    if (val)
    {
        g_value_init(val, G_TYPE_INT);
        g_value_set_int(val, value_to_write);
    }
}

inline void fill_string(GValue* val, const char* value_to_write)
{
    if (val)
    {
        g_value_init(val, G_TYPE_STRING);
        g_value_set_string(val, value_to_write);
    }
}

inline void fill_double(GValue* val, double value_to_write)
{
    if (val)
    {
        g_value_init(val, G_TYPE_DOUBLE);
        g_value_set_double(val, value_to_write);
    }
}

inline void fill_gvalue(GValue* val, prop_types type)
{
    if (val)
    {
        g_value_init(val, G_TYPE_STRING);
        g_value_set_string(val, to_string(type));
    }
}
} // namespace tcamprop_impl_helper
