/*
 * Copyright 2015 The Imaging Source Europe GmbH
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

#include "tcam-gobject.h"
#include <tcam.h>

struct _TcamProp
{
    GObject parent_instance;
    std::shared_ptr<tcam::CaptureDevice> device;
};

G_DEFINE_TYPE (TcamProp, tcam_prop, G_TYPE_OBJECT)


static void tcam_prop_get_property (GObject    *object,
                                    guint       property_id,
                                    GValue     *value,
                                    GParamSpec *pspec)
{
    switch (property_id)
    {
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}


static void tcam_prop_set_property (GObject      *object,
                                    guint         property_id,
                                    const GValue *value,
                                    GParamSpec   *pspec)
{
    switch (property_id)
    {
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}


static void tcam_prop_dispose (GObject *object)
{
    G_OBJECT_CLASS (tcam_prop_parent_class)->dispose (object);
}


static void tcam_prop_finalize (GObject *object)
{
    G_OBJECT_CLASS (tcam_prop_parent_class)->finalize (object);
}


static void tcam_prop_class_init (TcamPropClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);

    object_class->get_property = tcam_prop_get_property;
    object_class->set_property = tcam_prop_set_property;
    object_class->dispose = tcam_prop_dispose;
    object_class->finalize = tcam_prop_finalize;
}


static void tcam_prop_init (TcamProp *self)
{
    self->device = tcam::open_device("123");
    g_assert (self->device);
}


/**
 * tcam_prop_test:
 * @self: a #TcamProp
 *
 * Prints something
 *
 * Returns: (element-type utf8) (transfer full): list of strings.
 */
GSList* tcam_prop_test (TcamProp *self)
{
    GSList *list = NULL;
    list = g_slist_append (list, g_strdup ("One"));
    list = g_slist_append (list, g_strdup ("Two"));
    return list;
}


static GVariant* variant_from_property (tcam::Property* p)
{
    GVariantBuilder ppty_builder;
    g_variant_builder_init (&ppty_builder, G_VARIANT_TYPE_ARRAY);
    g_variant_builder_add (&ppty_builder,
                           "{sv}", "name",
                           g_variant_new_string (
                               p->get_name().c_str()));
    switch (p->get_type()){
        case TCAM_PROPERTY_TYPE_INTEGER:
        {
            tcam::PropertyInteger* i = (tcam::PropertyInteger*)p;
            g_variant_builder_add (&ppty_builder,
                                   "{sv}", "type",
                                   g_variant_new_string(
                                       "int32"));
            g_variant_builder_add (&ppty_builder,
                                   "{sv}", "min",
                                   g_variant_new_int32 (
                                       i->get_min()));
            g_variant_builder_add (&ppty_builder,
                                   "{sv}", "max",
                                   g_variant_new_int32 (
                                       i->get_max()));
            g_variant_builder_add (&ppty_builder,
                                   "{sv}", "step",
                                   g_variant_new_int32 (
                                       i->get_step()));
            g_variant_builder_add (&ppty_builder,
                                   "{sv}", "default",
                                   g_variant_new_int32 (
                                       i->get_default()));
            g_variant_builder_add (&ppty_builder,
                                   "{sv}", "value",
                                   g_variant_new_int32 (
                                       i->get_value()));
            break;
        }
        default:
        {
            break;
        }
    }
    return g_variant_builder_end (&ppty_builder);
}


/**
 * tcam_prop_enumerate:
 * @self: a #TcamProp
 *
 * Enumerate all device properties
 *
 * Return value: List of device properties as #GVariant
 */
GVariant* tcam_prop_enumerate (TcamProp* self)
{
    GVariantBuilder list_builder;

    g_variant_builder_init (&list_builder, G_VARIANT_TYPE_ARRAY);

    for (const auto& p : self->device->get_available_properties())
    {
        g_variant_builder_add (&list_builder,
                               "{sv}", p->get_name().c_str(),
                               variant_from_property(p));
    }
    return g_variant_builder_end (&list_builder);
}


/**
 * tcam_prop_get_property_names:
 * @self: a #TcamProp
 *
 * Return a list of property names
 *
 * Returns: (element-type utf8) (transfer full): list of property names
 */
GSList* tcam_prop_get_property_names (TcamProp* self)
{
    GSList *ret = NULL;
    for (const auto& p : self->device->get_available_properties())
    {
        ret = g_slist_append (ret, g_strdup(p->get_name().c_str()));
    }

    return ret;
}


static tcam::Property* find_property_by_name (TcamProp* self, std::string name)
{
    for (const auto& p : self->device->get_available_properties())
    {
        if (p->get_name() == name)
            return p;
    }
    return NULL;
}


/**
 * tcam_prop_set:
 * @self: a #TcamProp
 * @cname: a #gchar
 * @value: a #GVariant
 *
 * Sets a property
 *
 * Returns: TRUE on success, FALSE otherwise
 */
gboolean tcam_prop_set (TcamProp* self, gchar* cname, GVariant* value)
{
    GVariantDict dict;
    gboolean ret = FALSE;
    tcam::Property *tcam_ppty;
    std::string name = std::string(cname);
    tcam_ppty = find_property_by_name (self,name);
    if (tcam_ppty)
    {
        switch (tcam_ppty->get_type())
        {
            case TCAM_PROPERTY_TYPE_INTEGER:
            {
                tcam::PropertyInteger* prop_i =
                    (tcam::PropertyInteger*) tcam_ppty;
                ret = prop_i->
                    set_value(g_variant_get_int32(value));
                break;
            }
            default:
                break;
        }
    }
    return FALSE;
}


/**
 * tcam_prop_get:
 * @self: a #TcamProp
 * @cname: a #gchar*
 *
 * Get a property
 *
 * Returns: a #GVariant
 */
GVariant* tcam_prop_get (TcamProp* self, gchar* cname)
{
    tcam::Property* tcam_ppty;
    std::string name = std::string(cname);
    GVariant* ret = NULL;

    tcam_ppty = find_property_by_name (self,name);
    if (tcam_ppty)
    {
        ret = variant_from_property (tcam_ppty);
    }
    return ret;
}


/**
 * tcam_prop_get_property_type:
 * @self: a #TcamProp
 * @name: a #char* identifying the property to query
 *
 * Return the type of a property
 *
 * Returns: (transfer full): A string describing the property type
 */
gchar* tcam_prop_get_property_type (TcamProp* self, gchar* name)
{
    tcam::Property* tcam_ppty;
    gchar *ret = NULL;

    tcam_ppty = find_property_by_name (self, std::string(name));
    if (!tcam_ppty)
    {
        return g_strdup("invalid");
    }
    switch (tcam_ppty->get_type())
    {
        case TCAM_PROPERTY_TYPE_INTEGER:
        case TCAM_PROPERTY_TYPE_DOUBLE:
            ret = g_strdup("double");
            break;
        case TCAM_PROPERTY_TYPE_STRING:
            ret = g_strdup("utf8");
            break;
        case TCAM_PROPERTY_TYPE_BOOLEAN:
            ret = g_strdup("boolean");
            break;
        default:
            break;
    }

    return ret ? ret : g_strdup ("invalid");
}


/**
 * tcam_prop_set_property_double:
 * @self: a #TcamProp
 * @name: a #gchar* with the name of the property to set
 * @value: a #gdouble
 *
 * Set a property of type "double"
 *
 * Returns: TRUE on success, FALSE otherwise
 */
gboolean tcam_prop_set_property_double(TcamProp* self,
                                       gchar* name,
                                       gdouble value)
{
    tcam::Property* p;
    gboolean ret = TRUE;
    p = find_property_by_name (self, std::string(name));
    if (!p)
    {
        return FALSE;
    }

    switch (p->get_type())
    {
        case TCAM_PROPERTY_TYPE_INTEGER:
        {
            tcam::PropertyInteger* prop_i = (tcam::PropertyInteger*) p;
            prop_i->set_value(value);
            break;
        }
        case TCAM_PROPERTY_TYPE_DOUBLE:
        {
            tcam::PropertyDouble* prop_d = (tcam::PropertyDouble*) p;
            prop_d->set_value(value);
            break;
        }
        default:
            ret = FALSE;
            break;
    }

    return ret;
}


/**
 * tcam_prop_get_property_double
 * @self: a #TcamProp
 * @name: name of the property to query
 *
 * Get the current value of a "double" type property
 *
 * Returns: a #gdouble
 */
gdouble tcam_prop_get_property_double(TcamProp* self,
                                      gchar* name)
{
    tcam::Property* p;
    gdouble ret = 0.0;
    p = find_property_by_name (self, std::string(name));
    if (!p)
    {
        return ret;
    }

    switch (p->get_type())
    {
        case TCAM_PROPERTY_TYPE_INTEGER:
        {
            tcam::PropertyInteger* prop_i = (tcam::PropertyInteger*) p;
            ret = prop_i->get_value();
            break;
        }
        case TCAM_PROPERTY_TYPE_DOUBLE:
        {
            tcam::PropertyDouble* prop_d = (tcam::PropertyDouble*) p;
            ret = prop_d->get_value();
            break;
        }
        default:
            break;
    }

    return ret;
}


/**
 * tcam_prop_get_property_min
 * @self: a #TcamProp
 * @name: name of the property to query
 *
 * Get the minimum value of a "double" type property
 *
 * Returns: a #gdouble
 */
gdouble tcam_prop_get_property_min (TcamProp* self,
                                    gchar* name)
{
    tcam::Property* p;
    gdouble ret = 0.0;
    p = find_property_by_name (self, std::string(name));
    if (!p)
    {
        return ret;
    }

    switch (p->get_type())
    {
        case TCAM_PROPERTY_TYPE_INTEGER:
        {
            tcam::PropertyInteger* prop_i = (tcam::PropertyInteger*) p;
            ret = prop_i->get_min();
            break;
        }
        case TCAM_PROPERTY_TYPE_DOUBLE:
        {
            tcam::PropertyDouble* prop_d = (tcam::PropertyDouble*) p;
            ret = prop_d->get_min();
            break;
        }
        default:
            break;
    }

    return ret;
}


/**
 * tcam_prop_get_property_max
 * @self: a #TcamProp
 * @name: name of the property to query
 *
 * Get the maximum value of a "double" type property
 *
 * Returns: a #gdouble
 */
gdouble tcam_prop_get_property_max (TcamProp* self,
                                    gchar* name)
{
    tcam::Property* p;
    gdouble ret = 0.0;
    p = find_property_by_name (self, std::string(name));
    if (!p)
    {
        return ret;
    }

    switch (p->get_type())
    {
        case TCAM_PROPERTY_TYPE_INTEGER:
        {
            tcam::PropertyInteger* prop_i = (tcam::PropertyInteger*) p;
            ret = prop_i->get_max();
            break;
        }
        case TCAM_PROPERTY_TYPE_DOUBLE:
        {
            tcam::PropertyDouble* prop_d = (tcam::PropertyDouble*) p;
            ret = prop_d->get_max();
            break;
        }
        default:
            break;
    }

    return ret;
}


/**
 * tcam_prop_get_property_default_double
 * @self: a #TcamProp
 * @name: name of the property to query
 *
 * Get the default value of a "double" type property
 *
 * Returns: a #gdouble
 */
gdouble tcam_prop_get_property_default_double (TcamProp* self,
                                               gchar* name)
{
    tcam::Property* p;
    gdouble ret = 0.0;
    p = find_property_by_name (self, std::string(name));
    if (!p)
    {
        return ret;
    }

    switch (p->get_type())
    {
        case TCAM_PROPERTY_TYPE_INTEGER:
        {
            tcam::PropertyInteger* prop_i = (tcam::PropertyInteger*) p;
            ret = prop_i->get_default();
            break;
        }
        case TCAM_PROPERTY_TYPE_DOUBLE:
        {
            tcam::PropertyDouble* prop_d = (tcam::PropertyDouble*) p;
            ret = prop_d->get_default();
            break;
        }
        default:
            break;
    }

    return ret;
}


TcamProp* tcam_prop_new (void)
{
    return (TcamProp*)g_object_new (TCAM_TYPE_PROP, NULL);
}









// /*
//  * Copyright 2015 The Imaging Source Europe GmbH
//  *
//  * Licensed under the Apache License, Version 2.0 (the "License");
//  * you may not use this file except in compliance with the License.
//  * You may obtain a copy of the License at
//  *
//  * http://www.apache.org/licenses/LICENSE-2.0
//  *
//  * Unless required by applicable law or agreed to in writing, software
//  * distributed under the License is distributed on an "AS IS" BASIS,
//  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//  * See the License for the specific language governing permissions and
//  * limitations under the License.
//  */

// #include "tcam-gobject.h"

// #include "tcam.h"

// struct _Tcam
// {
//     GObject parent_instance;
//     std::shared_ptr<tcam::CaptureDevice> device;
// };

// G_DEFINE_TYPE (Tcam, tcam, G_TYPE_OBJECT)


// static void tcam_get_property (GObject*    object,
//                                guint       property_id,
//                                GValue*     value,
//                                GParamSpec* pspec)
// {
//     switch (property_id)
//     {
//         default:
//             G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
//     }
// }


// static void tcam_set_property (GObject*      object,
//                                guint         property_id,
//                                const GValue* value,
//                                GParamSpec*   pspec)
// {
//     switch (property_id)
//     {
//         default:
//             G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
//     }
// }


// static void tcam_dispose (GObject* object)
// {
//     G_OBJECT_CLASS (tcam_parent_class)->dispose (object);
// }


// static void tcam_finalize (GObject* object)
// {
//     G_OBJECT_CLASS (tcam_parent_class)->finalize (object);
// }


// static void tcam_class_init (TcamClass* klass)
// {
//     GObjectClass *object_class = G_OBJECT_CLASS (klass);

//     object_class->get_property = tcam_get_property;
//     object_class->set_property = tcam_set_property;
//     object_class->dispose = tcam_dispose;
//     object_class->finalize = tcam_finalize;
// }


// static void tcam_init (Tcam* self, gchar* serial)
// {
//     self->device = tcam::open_device(serial);
//     g_assert (self->device);
// }

// Tcam* tcam_new (void)
// {
//     return (Tcam*)g_object_new (TCAM_TYPE, NULL);
// }


// /**
//  * tcam_prop_get:
//  * @self: a #Tcam
//  * @cname: a #gchar*
//  *
//  * Get a property
//  *
//  * Returns: a #GVariant
//  */
// GVariant* tcam_get (Tcam* self, gchar* name)
// {}


// /**
//  * tcam_prop_set:
//  * @self: a #Tcam
//  * @cname: a #gchar
//  * @value: a #GVariant
//  *
//  * Sets a property
//  *
//  * Returns: TRUE on success, FALSE otherwise
//  */
// gboolean tcam_set (Tcam* self, gchar* name, GVariant* value)
// {}


// /**
//  * tcam_get_type:
//  * @self: a #Tcam
//  * @name: a #char* identifying the property to query
//  *
//  * Return the type of a property
//  *
//  * Returns: (transfer full): A string describing the property type
//  */
// gchar* tcam_get_property_type (Tcam* self, gchar* name)
// {
//     gchar* ret = NULL;

//     tcam::Property* prop = self->device->get_property_by_name(name);

//     if (prop != NULL)
//     {

//     }

//     return ret;
// }
