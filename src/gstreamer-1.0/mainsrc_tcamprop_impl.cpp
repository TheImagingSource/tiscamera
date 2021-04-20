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

#include "mainsrc_tcamprop_impl.h"

#include "mainsrc_device_state.h"

using namespace tcam::property;


const char* prop_type_to_string(TCAM_PROPERTY_TYPE type)
{
    switch (type)
    {
        case TCAM_PROPERTY_TYPE_BOOLEAN:
            return "boolean";
        case TCAM_PROPERTY_TYPE_INTEGER:
            return "integer";
        case TCAM_PROPERTY_TYPE_DOUBLE:
            return "double";
        case TCAM_PROPERTY_TYPE_STRING:
            return "string";
        case TCAM_PROPERTY_TYPE_ENUMERATION:
            return "enum";
        case TCAM_PROPERTY_TYPE_BUTTON:
            return "button";
        default:
            return nullptr;
    }
}


/**
 * gst_tcam_get_property_type:
 * @self: a #GstTcamMainSrcProp
 * @name: a #char* identifying the property to query
 *
 * Return the type of a property
 *
 * Returns: (transfer full): A string describing the property type
 */
gchar* gst_tcam_mainsrc_get_property_type(TcamProp* iface, const gchar* name)
{
    GstTcamMainSrc* self = GST_TCAM_MAINSRC(iface);

    if (!self->device || !self->device->dev)
    {
        return nullptr;
    }

    //     g_return_val_if_fail (self->device != NULL, NULL);
    // prefer this so that no gobject error appear
    // this method is also used to check for property existence
    // so unneccessary errors should be avoided
    if (self->device->dev == nullptr)
    {
        return nullptr;
    }

    auto prop = self->device->dev->get_property(name);

    if (!prop)
    {
        return nullptr;
    }

    const char* prop_type_str = g_strdup(prop_type_to_string(prop->get_type()));
    if (prop_type_str == nullptr)
    {
        return g_strdup("unknown");
    }

    return g_strdup(prop_type_str);
}

/**
 * gst_tcam_mainsrc_get_property_names:
 * @self: a #GstTcamMainSrc
 *
 * Return a list of property names
 *
 * Returns: (element-type utf8) (transfer full): list of property names
 */
GSList* gst_tcam_mainsrc_get_property_names(TcamProp* iface)
{
    GSList* ret = NULL;
    GstTcamMainSrc* self = GST_TCAM_MAINSRC(iface);

    g_return_val_if_fail(self->device != NULL, NULL);
    g_return_val_if_fail(self->device->dev != NULL, NULL);

    std::vector<std::shared_ptr<tcam::property::IPropertyBase>> vec =
        self->device->dev->get_properties();

    for (const auto& v : vec) { ret = g_slist_append(ret, g_strdup(v->get_name().c_str())); }

    return ret;
}


gboolean gst_tcam_mainsrc_get_tcam_property(TcamProp* iface,
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
    gboolean ret = TRUE;
    GstTcamMainSrc* self = GST_TCAM_MAINSRC(iface);

    g_return_val_if_fail(self->device != NULL, FALSE);
    g_return_val_if_fail(self->device->dev != NULL, FALSE);

    auto property = self->device->dev->get_property(name);

    if (property == nullptr)
    {
        GST_DEBUG_OBJECT(GST_TCAM_MAINSRC(iface), "no property with name: '%s'", name);
        return FALSE;
    }

    //property->update();

    //struct tcam_device_property prop = property->get_struct();

    if (flags)
    {
        g_value_init(flags, G_TYPE_INT);
        g_value_set_int(flags, static_cast<int>(property->get_flags()));
    }

    if (category)
    {
        g_value_init(category, G_TYPE_STRING);

        g_value_set_string(category, "Unknown");
        //tcam::category2string(prop.group.property_category).c_str());
    }
    if (group)
    {
        g_value_init(group, G_TYPE_STRING);
        g_value_set_string(group, "");
        //tcam::get_control_reference(prop.group.property_group).name.c_str());
    }

    auto prop_type = property->get_type();

    if (type)
    {
        // g_value_set_gtype (type, G_TYPE_INT);
        g_value_init(type, G_TYPE_STRING);
        g_value_set_string(type, prop_type_to_string(prop_type));
    }

    switch (prop_type)
    {
        case TCAM_PROPERTY_TYPE_INTEGER:
        {
            auto prop_int = dynamic_cast<tcam::property::IPropertyInteger*>(property.get());
            if (value)
            {
                g_value_init(value, G_TYPE_INT);
                g_value_set_int(value, prop_int->get_value());
            }
            if (min)
            {
                g_value_init(min, G_TYPE_INT);
                g_value_set_int(min, prop_int->get_min());
            }
            if (max)
            {
                g_value_init(max, G_TYPE_INT);
                g_value_set_int(max, prop_int->get_max());
            }
            if (def)
            {
                g_value_init(def, G_TYPE_INT);
                g_value_set_int(def, prop_int->get_default());
            }
            if (step)
            {
                g_value_init(step, G_TYPE_INT);
                g_value_set_int(step, prop_int->get_step());
            }
            break;
        }
        case TCAM_PROPERTY_TYPE_ENUMERATION:
        {
            auto prop_enum = dynamic_cast<tcam::property::IPropertyEnum*>(property.get());

            if (value)
            {
                g_value_init(value, G_TYPE_STRING);
                g_value_set_string(value, prop_enum->get_value().c_str());
            }
            if (min)
            {
                g_value_init(min, G_TYPE_STRING);
                g_value_set_string(min, "");
            }
            if (max)
            {
                g_value_init(max, G_TYPE_STRING);
                g_value_set_string(max, "");
            }
            if (def)
            {
                g_value_init(def, G_TYPE_STRING);
                g_value_set_string(def, prop_enum->get_default().c_str());
            }
            if (step)
            {
                g_value_init(step, G_TYPE_STRING);
                g_value_set_string(step, "");
            }
            break;
        }
        case TCAM_PROPERTY_TYPE_DOUBLE:
        {
            auto prop_float = dynamic_cast<tcam::property::IPropertyFloat*>(property.get());
            if (value)
            {
                g_value_init(value, G_TYPE_DOUBLE);
                g_value_set_double(value, prop_float->get_value());
            }
            if (min)
            {
                g_value_init(min, G_TYPE_DOUBLE);
                g_value_set_double(min, prop_float->get_min());
            }
            if (max)
            {
                g_value_init(max, G_TYPE_DOUBLE);
                g_value_set_double(max, prop_float->get_max());
            }
            if (def)
            {
                g_value_init(def, G_TYPE_DOUBLE);
                g_value_set_double(def, prop_float->get_default());
            }
            if (step)
            {
                g_value_init(step, G_TYPE_DOUBLE);
                g_value_set_double(step, prop_float->get_step());
            }
            break;
        }
        case TCAM_PROPERTY_TYPE_STRING:
        // {
        //     if (value)
        //     {
        //         g_value_init(value, G_TYPE_STRING);
        //         g_value_set_string(value, prop.value.s.value);
        //     }
        //     if (min)
        //     {
        //         g_value_init(min, G_TYPE_STRING);
        //     }
        //     if (max)
        //     {
        //         g_value_init(max, G_TYPE_STRING);
        //     }
        //     if (def)
        //     {
        //         g_value_init(def, G_TYPE_STRING);
        //         g_value_set_string(def, prop.value.s.default_value);
        //     }
        //     if (step)
        //     {
        //         g_value_init(def, G_TYPE_STRING);
        //     }
        //     break;
        // }
        case TCAM_PROPERTY_TYPE_BUTTON:
        {
            if (value)
            {
                g_value_init(value, G_TYPE_BOOLEAN);
                g_value_set_boolean(value, FALSE);
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
                g_value_set_boolean(def, FALSE);
            }
            if (step)
            {
                g_value_init(step, G_TYPE_BOOLEAN);
                g_value_set_boolean(step, TRUE);
            }
            break;
        }
        case TCAM_PROPERTY_TYPE_BOOLEAN:
        {
            auto prop_bool = dynamic_cast<tcam::property::IPropertyBool*>(property.get());
            if (value)
            {
                g_value_init(value, G_TYPE_BOOLEAN);
                g_value_set_boolean(value, prop_bool->get_value());
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
                g_value_set_boolean(def, prop_bool->get_default());
            }
            if (step)
            {
                g_value_init(step, G_TYPE_BOOLEAN);
            }
            break;
        }
        default:
        {
            if (value)
            {
                g_value_init(value, G_TYPE_INT);
            }
            ret = FALSE;
            break;
        }
    }

    return ret;
}


GSList* gst_tcam_mainsrc_get_menu_entries(TcamProp* iface, const char* menu_name)
{
    GSList* ret = NULL;

    GstTcamMainSrc* self = GST_TCAM_MAINSRC(iface);

    auto property = self->device->dev->get_property(menu_name);

    if (property == nullptr)
    {
        return ret;
    }

    if (property->get_type() != TCAM_PROPERTY_TYPE_ENUMERATION)
    {
        return ret;
    }

    auto prop_enum = dynamic_cast<tcam::property::IPropertyEnum*>(property.get());

    auto mapping = prop_enum->get_entries();

    for (const auto& m : mapping) { ret = g_slist_append(ret, g_strdup(m.c_str())); }

    return ret;
}


gboolean gst_tcam_mainsrc_set_tcam_property(TcamProp* iface, const gchar* name, const GValue* value)
{
    GstTcamMainSrc* self = GST_TCAM_MAINSRC(iface);

    auto prop = self->device->dev->get_property(name);

    if (prop == nullptr)
    {
        return FALSE;
    }

    switch (prop->get_type())
    {
        case TCAM_PROPERTY_TYPE_INTEGER:
        {
            if (!G_VALUE_HOLDS(value, G_TYPE_INT))
            {
                return FALSE;
            }

            return static_cast<IPropertyInteger*>(prop.get())
                ->set_value((int64_t)g_value_get_int(value));
        }
        case TCAM_PROPERTY_TYPE_DOUBLE:
        {
            if (!G_VALUE_HOLDS(value, G_TYPE_DOUBLE))
            {
                return FALSE;
            }
            return static_cast<IPropertyFloat*>(prop.get())->set_value(g_value_get_double(value));
        }
        // case TCAM_PROPERTY_TYPE_STRING:
        // {
        //     if (!G_VALUE_HOLDS(value, G_TYPE_STRING))
        //     {
        //         return FALSE;
        //     }
        //     //return property->set_value(g_value_get_string(value));
        //     return static_cast<IPropertyString*>(prop.get())->set_value(g_value_get_string(value));
        // }
        case TCAM_PROPERTY_TYPE_BOOLEAN:
        {
            if (!G_VALUE_HOLDS(value, G_TYPE_BOOLEAN))
            {
                return FALSE;
            }
            return static_cast<IPropertyBool*>(prop.get())->set_value(g_value_get_boolean(value));
        }
        case TCAM_PROPERTY_TYPE_BUTTON:
        {
            return static_cast<IPropertyCommand*>(prop.get())->execute();
        }
        case TCAM_PROPERTY_TYPE_ENUMERATION:
        {
            if (!G_VALUE_HOLDS(value, G_TYPE_STRING))
            {
                return FALSE;
            }

            std::string s = g_value_get_string(value);
            return static_cast<IPropertyEnum*>(prop.get())->set_value_str(s);
        }
        default:
        {
            return FALSE;
        }
    }
}


GSList* gst_tcam_mainsrc_get_device_serials(TcamProp* self)
{

    GstTcamMainSrc* s = GST_TCAM_MAINSRC(self);

    std::vector<tcam::DeviceInfo> devices = s->device->index_.get_device_list();

    GSList* ret = NULL;

    for (const auto& d : devices)
    {
        ret = g_slist_append(ret, g_strndup(d.get_serial().c_str(), d.get_serial().size()));
    }

    return ret;
}


GSList* gst_tcam_mainsrc_get_device_serials_backend(TcamProp* self)
{
    GstTcamMainSrc* s = GST_TCAM_MAINSRC(self);

    std::vector<tcam::DeviceInfo> devices = s->device->index_.get_device_list();

    GSList* ret = NULL;

    for (const auto& d : devices)
    {
        std::string long_serial = d.get_serial() + "-" + d.get_device_type_as_string();
        ret = g_slist_append(ret, g_strndup(long_serial.c_str(), long_serial.size()));
    }

    return ret;
}


gboolean gst_tcam_mainsrc_get_device_info(TcamProp* self,
                                          const char* serial,
                                          char** name,
                                          char** identifier,
                                          char** connection_type)
{
    GstTcamMainSrc* s = GST_TCAM_MAINSRC(self);

    std::vector<tcam::DeviceInfo> devices = s->device->index_.get_device_list();

    gboolean ret = FALSE;

    if (devices.empty())
    {
        return FALSE;
    }

    std::string input = serial;
    std::string actual_serial;
    std::string type;

    TCAM_DEVICE_TYPE ty = TCAM_DEVICE_TYPE_UNKNOWN;

    auto pos = input.find("-");

    if (pos != std::string::npos)
    {
        actual_serial = input.substr(0, pos);
        type = input.substr(pos + 1);
        ty = tcam::tcam_device_from_string(type);
    }
    else
    {
        actual_serial = serial;
    }

    for (const auto& d : devices)
    {
        struct tcam_device_info info = d.get_info();

        if (!strncmp(actual_serial.c_str(), info.serial_number, sizeof(info.serial_number)))
        {
            if (ty != TCAM_DEVICE_TYPE_UNKNOWN)
            {
                if (ty != info.type)
                {
                    continue;
                }
            }

            ret = TRUE;
            if (name)
            {
                *name = g_strndup(info.name, sizeof(info.name));
            }
            if (identifier)
            {
                *identifier = g_strndup(info.identifier, sizeof(info.identifier));
            }
            if (connection_type)
            {
                auto t = tcam::tcam_device_type_to_string(info.type);
                *connection_type = g_strndup(t.c_str(), strlen(t.c_str()));
            }
            break;
        }
    }

    return ret;
}
