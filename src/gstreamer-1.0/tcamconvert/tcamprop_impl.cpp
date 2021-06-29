/*
 * Copyright 2021 The Imaging Source Europe GmbH
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

#include "../../gobject/tcamprop.h"
#include "../../logging.h"
#include "property_handler_base.h"

namespace tcamconvert
{
prop_system::property_interface* get_property_interface(TcamProp* iface);
}

namespace
{

static char* g_strdup_string_view(std::string_view str) noexcept
{
    if (str.size() == 0)
    {
        return nullptr;
    }
    char* rval = (char*)g_malloc(str.size() + 1);
    if (rval == nullptr)
    {
        return nullptr;
    }
    memcpy(rval, str.data(), str.size());
    rval[str.size()] = '\0';
    return rval;
}

static auto find_property_desc_entry(prop_system::property_interface* self,
                                     std::string_view str) noexcept -> prop_system::property_desc
{
    for (const auto& prop : self->get_property_list())
    {
        if (prop.prop_name == str)
        {
            return prop;
        }
    }
    return {};
}

static void write_gvalue(GValue* dst, bool val)
{
    if (!dst)
        return;

    g_value_init(dst, G_TYPE_BOOLEAN);
    g_value_set_boolean(dst, val ? TRUE : FALSE);
}

static void write_gvalue(GValue* dst, int val)
{
    if (!dst)
        return;

    g_value_init(dst, G_TYPE_INT);
    g_value_set_int(dst, val);
}

static void write_gvalue(GValue* dst, int64_t val)
{
    if (!dst)
        return;

    g_value_init(dst, G_TYPE_INT);
    g_value_set_int(dst, val);
}

static void write_gvalue(GValue* dst, double val)
{
    if (!dst)
        return;

    g_value_init(dst, G_TYPE_DOUBLE);
    g_value_set_double(dst, val);
}
//
//static void write_gvalue(GValue* dst, const std::string& val)
//{
//    if (!dst)
//        return;
//
//    g_value_init(dst, G_TYPE_STRING);
//    g_value_set_string(dst, val.c_str());
//}
//
static void write_gvalue(GValue* dst, const char* val)
{
    if (!dst)
        return;

    g_value_init(dst, G_TYPE_STRING);
    g_value_set_string(dst, val);
}

static void write_gvalue(GValue* dst, const std::string_view& val)
{
    if (!dst)
        return;

    g_value_init(dst, G_TYPE_STRING);
    g_value_take_string(dst, g_strdup_string_view(val));
}

//
//static void write_gvalue(GValue* dst, const std::string_view& val)
//{
//    if (!dst)
//        return;
//
//    g_value_init(dst, G_TYPE_STRING);
//    g_value_take_string(dst, g_strdup_string_view(val));
//}

} // namespace

static GSList* gst_tcamconvert_get_tcam_property_names(TcamProp* iface)
{
    auto* self = tcamconvert::get_property_interface(iface);

    GSList* names = nullptr;
    for (const auto& prop : self->get_property_list())
    {
        auto flags_opt = self->get_property_flags(prop.prop_name);
        if (!flags_opt.has_value())
        {
            continue;
        }
        auto flags = flags_opt.value();
        if (!!(flags & prop_system::prop_flags::hide_from_get_property_names))
        {
            continue;
        }
        if (!(flags & prop_system::prop_flags::implemented))
        {
            continue;
        }

        names = g_slist_append(names, g_strdup_string_view(prop.prop_name));
    }
    return names;
}

static gchar* gst_tcamconvert_get_property_type(TcamProp* iface, const gchar* name)
{
    auto* self = tcamconvert::get_property_interface(iface);

    auto prop_opt = self->get_property_desc(name);
    if (!prop_opt)
    {
        return nullptr;
    }
    auto prop = prop_opt.value();

    return g_strdup(to_string(prop.type));
}


static gboolean gst_tcamconvert_get_tcam_property(TcamProp* iface,
                                                  const gchar* name_,
                                                  GValue* value,
                                                  GValue* min,
                                                  GValue* max,
                                                  GValue* def,
                                                  GValue* stp,
                                                  GValue* type,
                                                  GValue* flags,
                                                  GValue* category,
                                                  GValue* group)
{
    auto* self = tcamconvert::get_property_interface(iface);
    if (name_ == nullptr)
    {
        return FALSE;
    }

    std::string_view name = name_;

    auto prop_opt = self->get_property_desc(name);
    if (!prop_opt)
    {
        return FALSE;
    }
    auto prop = prop_opt.value();

    auto prop_flags_opt = self->get_property_flags(name);
    if (!prop_flags_opt)
    {
        return FALSE;
    }
    auto prop_flags = prop_flags_opt.value();

    if (!(prop_flags & prop_system::prop_flags::implemented))
    {
        return FALSE;
    }
    write_gvalue(flags, (int)(prop_flags));
    write_gvalue(type, to_string(prop.type));
    write_gvalue(category, prop.prop_category);
    write_gvalue(group, prop.prop_group);

    if (prop.type == prop_system::prop_type::button)
    {
        write_gvalue(value, false);
        write_gvalue(min, false);
        write_gvalue(max, true);
        write_gvalue(def, false);
        write_gvalue(stp, true);
    }
    else if (prop.type == prop_system::prop_type::boolean)
    {
        if (value)
        {
            auto tmp = self->get_property(name);
            if (tmp.has_value())
            {
                write_gvalue(value, tmp.value().integer != 0);
            }
        }
        auto range_opt = self->get_property_range(name);
        if (range_opt.has_error())
        {
            return FALSE;
        }

        write_gvalue(min, false);
        write_gvalue(max, true);
        write_gvalue(def, range_opt.value().val_def.integer != 0);
        write_gvalue(stp, true);
    }
    else if (prop.type == prop_system::prop_type::integer)
    {
        if (value)
        {
            auto tmp = self->get_property(name);
            if (tmp.has_value())
            {
                write_gvalue(value, tmp.value().integer);
            }
        }
        auto range_opt = self->get_property_range(name);
        if (range_opt.has_error())
        {
            return FALSE;
        }
        auto range = range_opt.value().to_integer();

        write_gvalue(min, range.min);
        write_gvalue(max, range.max);
        write_gvalue(def, range.def);
        write_gvalue(stp, range.stp);
    }
    else if (prop.type == prop_system::prop_type::real)
    {
        if (value)
        {
            auto tmp = self->get_property(name);
            if (tmp.has_value())
            {
                write_gvalue(value, tmp.value().real);
            }
        }

        auto range_opt = self->get_property_range(name);
        if (range_opt.has_error())
        {
            return FALSE;
        }
        auto range = range_opt.value().to_real();

        write_gvalue(min, range.min);
        write_gvalue(max, range.max);
        write_gvalue(def, range.def);
        write_gvalue(stp, range.stp);
    }
    else if (prop.type == prop_system::prop_type::menu)
    {
        auto find_string = [&](int idx) -> const char* {
            if (idx < 0 && idx >= (int)prop.menu_entries.size())
            {
                return nullptr;
            }
            return prop.menu_entries[idx];
        };
        if (value)
        {
            auto tmp = self->get_property(name);
            if (tmp.has_value())
            {
                write_gvalue(value, find_string(tmp.value().integer));
            }
        }

        auto range_opt = self->get_property_range(name);
        if (range_opt.has_error())
        {
            return FALSE;
        }
        auto range = range_opt.value();

        write_gvalue(min, "");
        write_gvalue(max, "");
        write_gvalue(def, find_string(range.val_def.integer));
        write_gvalue(stp, "");
    }
    else
    {
        return FALSE;
    }
    return TRUE;
}

static gboolean gst_tcamconvert_set_tcam_property_(prop_system::property_interface* self,
                                                   const gchar* name_,
                                                   const GValue* value)
{
    if (name_ == nullptr || value == nullptr)
    {
        return FALSE;
    }

    std::string_view name = name_;
    auto prop_opt = self->get_property_desc(name);
    if (!prop_opt)
    {
        return FALSE;
    }
    auto prop = prop_opt.value();

    auto report_type_mismatch = [&](const char* /*type_expected*/) {
        //auto str_recv = G_VALUE_TYPE_NAME( value );
        //tcamdutils::report_warning( self, fmt::format( "Property={}, Expected GValue with type {}, received {}", name, type_expected, str_recv ) );
    };

    switch (prop.type)
    {
        case prop_system::prop_type::button:
        {
            auto err = self->set_property(name, prop_system::prop_values { true });
            if (err)
            {
                return FALSE;
            }
            return TRUE;
        }
        case prop_system::prop_type::boolean:
        {
            if (!G_VALUE_HOLDS_BOOLEAN(value))
            {
                report_type_mismatch("G_TYPE_BOOLEAN");
                return FALSE;
            }

            auto val = g_value_get_boolean(value) == TRUE;
            auto err = self->set_property(name, prop_system::prop_values { val });
            if (err)
            {
                return FALSE;
            }
            return TRUE;
        }
        case prop_system::prop_type::integer:
        {
            if (!G_VALUE_HOLDS_INT(value))
            {
                report_type_mismatch("G_TYPE_INT");
                return FALSE;
            }
            auto val = (int64_t)g_value_get_int(value);
            auto err = self->set_property(name, prop_system::prop_values { val });
            if (err)
            {
                return FALSE;
            }
            return TRUE;
        }
        case prop_system::prop_type::real:
        {
            if (!G_VALUE_HOLDS_DOUBLE(value))
            {
                report_type_mismatch("G_TYPE_DOUBLE");
                return FALSE;
            }
            auto val = g_value_get_double(value);
            auto err = self->set_property(name, prop_system::prop_values { val });
            if (err)
            {
                return FALSE;
            }
            return TRUE;
        }
        case prop_system::prop_type::menu:
        {
            int index = 0;
            if (G_VALUE_HOLDS_STRING(value))
            {
                std::string str = g_value_get_string(value);

                auto& menu_entries = prop.menu_entries;
                auto it = std::find(menu_entries.begin(), menu_entries.end(), str);
                if (it == menu_entries.end())
                {
                    return FALSE;
                }
                index = std::distance(menu_entries.begin(), it);
            }
            else if (G_VALUE_HOLDS_INT(value))
            {
                index = g_value_get_int(value);
            }
            else
            {
                return FALSE;
            }

            if (index < 0 || index >= (int)prop.menu_entries.size())
            {
                return FALSE;
            }
            auto err = self->set_property(name, prop_system::prop_values { index });
            if (err)
            {
                return FALSE;
            }
            return TRUE;
        }
        default:
            return FALSE;
    }
}

static gboolean gst_tcamconvert_set_tcam_property(TcamProp* iface,
                                                  const gchar* name,
                                                  const GValue* value)
{
    auto* self = tcamconvert::get_property_interface(iface);

    return gst_tcamconvert_set_tcam_property_(self, name, value);
}

static GSList* gst_tcamconvert_get_menu_entries(TcamProp* iface, const char* name_)
{
    auto* self = tcamconvert::get_property_interface(iface);

    if (name_ == nullptr)
    {
        return FALSE;
    }

    std::string_view name = name_;

    auto prop = find_property_desc_entry(self, name);
    if (prop.prop_name.empty())
    {
        return nullptr;
    }

    if (prop.type != prop_system::prop_type::menu)
    {
        return nullptr;
    }

    GSList* ret = nullptr;
    for (const auto& m : prop.menu_entries) { ret = g_slist_append(ret, g_strdup(m)); }
    return ret;
}

void gst_tcamconvert_prop_init(TcamPropInterface* iface)
{
    iface->get_tcam_property_names = gst_tcamconvert_get_tcam_property_names;
    iface->get_tcam_property_type = gst_tcamconvert_get_property_type;
    iface->get_tcam_menu_entries = gst_tcamconvert_get_menu_entries;
    iface->get_tcam_property = gst_tcamconvert_get_tcam_property;
    iface->set_tcam_property = gst_tcamconvert_set_tcam_property;
}
