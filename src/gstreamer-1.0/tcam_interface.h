

#ifndef TCAM_GST_TCAM_INTERFACE_H
#define TCAM_GST_TCAM_INTERFACE_H

#include "base_types.h"
#include "utils.h"
#include "logging.h"
#include "tcamprop.h"

#include <string>
#include <memory>
#include <cstring>

#include <glib-object.h>

std::shared_ptr<std::vector<tcam::Property>> tcam_interface_properties;


void tcam_interface_init (std::shared_ptr<std::vector<tcam::Property>> properties)
{
    tcam_interface_properties = properties;
}


struct tcam_device_property* tcam_interface_get_property (uint32_t id)
{
    if (tcam_interface_properties == nullptr)
    {
        return nullptr;
    }

}


GSList* tcam_interface_get_property_names (TcamProp* self)
{
    GSList* names = nullptr;

    if (tcam_interface_properties == nullptr || tcam_interface_properties->empty())
    {
        return nullptr;
    }

    for (const auto& p : *(tcam_interface_properties.get()))
    {
        names = g_slist_append(names, g_strdup(p.get_name().c_str()));
    }
    return names;
}


gchar* tcam_interface_get_property_type (TcamProp* self, const gchar* name)
{
    for (const auto& p : *(tcam_interface_properties.get()))
    {
        if (strcmp(p.get_name().c_str(), name) == 0)
        {
            return g_strdup(tcam::property_type_to_string(p.get_type()).c_str());
        }
    }
    return nullptr;
}


gboolean tcam_interface_get_tcam_property (TcamProp* prop,
                                           const gchar* name,
                                           GValue* value,
                                           GValue* min,
                                           GValue* max,
                                           GValue* def,
                                           GValue* step,
                                           GValue* type,
                                           GValue* flags,
                                           GValue* category,
                                           GValue* group)
{
    auto props = *(tcam_interface_properties.get());
    for (const auto& pr : props)
    {
        auto p = pr.get_struct();
        if (strcmp(p.name, name) == 0)
        {
            // tcam_log(TCAM_LOG_ERROR, "bla %s", name);

            if (flags)
            {
                g_value_init(flags, G_TYPE_INT);
                g_value_set_int(flags, 0);
            }

            if (type)
            {
                g_value_init(type, G_TYPE_STRING);
                g_value_set_string(type, tcam::property_type_to_string(p.type).c_str());
            }

            if (category)
            {
                g_value_init(category, G_TYPE_STRING);
                g_value_set_string(category, tcam::category2string(p.group.property_category).c_str());
            }

            if (group)
            {
                g_value_init(group, G_TYPE_STRING);
                g_value_set_string(group, tcam::property_id_to_string(p.group.property_group).c_str());
            }

            if (p.type == TCAM_PROPERTY_TYPE_INTEGER)
            {
                if (value)
                {
                    g_value_init(value, G_TYPE_INT);
                    g_value_set_int(value, p.value.i.value);
                }
                if (min)
                {
                    g_value_init(min, G_TYPE_INT);
                    g_value_set_int(min, p.value.i.min);
                }
                if (max)
                {
                    g_value_init(max, G_TYPE_INT);
                    g_value_set_int(max, p.value.i.max);
                }
                if (def)
                {
                    g_value_init(def, G_TYPE_INT);
                    g_value_set_int(def, p.value.i.default_value);
                }
                if (step)
                {
                    g_value_init(step, G_TYPE_INT);
                    g_value_set_int(step, p.value.i.step);
                }
            }
            else if (p.type == TCAM_PROPERTY_TYPE_BOOLEAN || p.type == TCAM_PROPERTY_TYPE_BUTTON)
            {
                if (value)
                {
                    g_value_init(value, G_TYPE_BOOLEAN);
                    g_value_set_boolean(value, p.value.b.value);
                }
                if (min)
                {
                    g_value_init(min, G_TYPE_BOOLEAN);
                    g_value_set_boolean(min, FALSE);
                }
                if (max)
                {
                    g_value_init(max, G_TYPE_BOOLEAN);
                    g_value_set_boolean(max, TRUE);
                }
                if (def)
                {
                    g_value_init(def, G_TYPE_BOOLEAN);
                    g_value_set_boolean(def, p.value.b.default_value);
                }
                if (step)
                {
                    g_value_init(step, G_TYPE_INT);
                    g_value_set_int(step, 1);
                }
            }
            else if (p.type == TCAM_PROPERTY_TYPE_DOUBLE)
            {
                if (value)
                {
                    g_value_init(value, G_TYPE_DOUBLE);
                    g_value_set_double(value, p.value.d.value);
                }
                if (min)
                {
                    g_value_init(min, G_TYPE_DOUBLE);
                    g_value_set_double(min, p.value.d.min);
                }
                if (max)
                {
                    g_value_init(max, G_TYPE_DOUBLE);
                    g_value_set_double(max, p.value.d.max);
                }
                if (def)
                {
                    g_value_init(def, G_TYPE_DOUBLE);
                    g_value_set_double(def, p.value.d.default_value);
                }
                if (step)
                {
                    g_value_init(step, G_TYPE_DOUBLE);
                    g_value_set_double(step, p.value.d.step);
                }
            }
            else
            {
                return FALSE;
            }


            return TRUE;
        }

    }
    return FALSE;
}


GSList* tcam_interface_get_menu_entries (TcamProp* self,
                                         const gchar* name)
{
    GSList* ret = nullptr;

    for (auto& p : *(tcam_interface_properties.get()))
    {
        if (strcmp(name, p.get_name().c_str()) == 0)
        {
            if (p.get_type() != TCAM_PROPERTY_TYPE_ENUMERATION)
            {
                return nullptr;
            }

            auto mapping = ((tcam::PropertyEnumeration&)p).get_values();

            for (const auto& m : mapping)
            {
                ret = g_slist_append(ret, g_strdup(m.c_str()));
            }

            break;
        }
    }

    return ret;
}

gboolean tcam_interface_set_tcam_property (TcamProp* self,
                                           const gchar* name,
                                           const GValue* value)
{
    for (auto& p : *(tcam_interface_properties.get()))
    {
        if (strcmp(name, p.get_name().c_str()) == 0)
        {
            switch (p.get_type())
            {
                case TCAM_PROPERTY_TYPE_BOOLEAN:
                {
                    return p.set_value((bool)g_value_get_boolean(value));
                }
                case TCAM_PROPERTY_TYPE_INTEGER:
                {
                    auto i = (int64_t)g_value_get_int(value);
                    return p.set_value(i);
                }
                case TCAM_PROPERTY_TYPE_DOUBLE:
                {
                    return p.set_value((double)g_value_get_double(value));
                }
                case TCAM_PROPERTY_TYPE_BUTTON:
                {
                    return p.set_value();
                }
                case TCAM_PROPERTY_TYPE_ENUMERATION:
                {
                    return p.set_value(std::string(g_value_get_string(value)));
                }
                case TCAM_PROPERTY_TYPE_UNKNOWN:
                default:
                {
                    break;
                }
            }
        }
    }

    return FALSE;
}

GSList* tcam_interface_get_device_serials (TcamProp*);
gboolean tcam_interface_get_device_info (TcamProp* self,
                                         const char* serial,
                                         char** name,
                                         char** identifier,
                                         char** connection_type);


#endif /* TCAM_GST_TCAM_INTERFACE_H */
