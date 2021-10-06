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

#include "tcamprop.h"

G_DEFINE_INTERFACE(TcamProp, tcam_prop, G_TYPE_OBJECT)


//
// error stuff
//

    static const GEnumValue _tcam_error_values[] =
{
    { TCAM_ERROR_UNKNOWN, "TCAM_ERROR_UNKNOWN", "unknown-error" },
    { TCAM_ERROR_DEVICE_LOST, "TCAM_ERROR_DEVICE_LOST", "device-lost" },
    { TCAM_ERROR_NO_SUCH_PROPERTY, "TCAM_ERROR_NO_SUCH_PROPERTY", "unknown-property" },
    { TCAM_ERROR_PROPERTY_VALUE, "TCAM_ERROR_PROPERTY_VALUE", "illegal-value" },
    { TCAM_ERROR_PROPERTY_NOT_SETTABLE, "TCAM_ERROR_PROPERTY_NOT_SETTABLE", "not-settable" },
    { TCAM_ERROR_WRONG_TYPE, "TCAM_ERROR_WRONG_TYPE", "wrong-type" },
    { 0, NULL, NULL }
};

GType tcam_error_get_type(void)
{
    static GType type = 0;

    if (!type)
    {
        type = g_enum_register_static("TcamError", _tcam_error_values);
    }
    return type;
}

GQuark tcam_error_quark(void)
{
    return g_quark_from_static_string("tcam-error-quark");
}


static const GFlagsValue _tcamprop_property_flags_values[] = {
    { TCAM_READ_ONLY, "TCAM_READ_ONLY", "read-only" },
    { TCAM_WRITE_ONLY, "TCAM_WRITE_ONLY", "write-only" },
    { TCAM_LOCKED, "TCAM_LOCKED", "locked" },
    { TCAM_EXTERNAL, "TCAM_EXTERNAL", "external" },
    { 0, NULL, NULL }
};

GType tcam_property_flags_get_type (void)
{
    static GType type = 0;

    if (!type)
    {
        type = g_flags_register_static ("TcamPropertyflags", _tcamprop_property_flags_values);
    }
    return type;
}


static const GFlagsValue _tcamprop_property_visibility_values[] = {
    {TCAM_VISIBILITY_BEGINNER, "TCAM_BEGINNER", "beginner"},
    {TCAM_VISIBILITY_EXPERT, "TCAM_EXPERT", "expert"},
    {TCAM_VISIBILITY_GURU, "TCAM_GURU", "guru"},
    {TCAM_VISIBILITY_INVISIBLE, "TCAM_INVISIBLE", "invisible" },
    { 0, NULL, NULL }
};


GType tcam_property_visibility_get_type (void)
{
    static GType type = 0;

    if (!type)
    {
        type = g_flags_register_static ("TcamPropertyVisibility", _tcamprop_property_visibility_values);
    }
    return type;
}


static const GEnumValue _tcam_property_type_values[] = {
    {TCAM_PROPERTY_UNKNOWN, "TCAM_PROPERTY_UNKNOWN", "unknown"},
    {TCAM_PROPERTY_INT   , "TCAM_PROPERTY_INT", "int"},
    {TCAM_PROPERTY_DOUBLE, "TCAM_PROPERTY_DOUBLE", "double"},
    {TCAM_PROPERTY_ENUM  , "TCAM_PROPERTY_ENUM", "enum"},
    {TCAM_PROPERTY_BOOL  , "TCAM_PROPERTY_BOOL", "bool"},
    {TCAM_PROPERTY_BUTTON, "TCAM_PROPERTY_BUTTON", "button"},
    { 0, NULL, NULL}
};

GType tcam_property_type_get_type (void)
{
    static GType type = 0;

    if (!type)
    {
        type = g_enum_register_static ("TcamPropertyType", _tcam_property_type_values);
    }
    return type;
}



/**
 * tcam_property_info_new: (constructor)
 * @name: string to be referenced for .name
 * @displayname: string to referenced for .displayname
 * @description: string to referenced for .description
 * @unit: string to referenced for .unit
 * @category: string to referenced for .category
 *
 * Returns: (transfer full): new #TcamPropertyInfo*
 */
TcamPropertyInfo* tcam_property_info_new (const gchar* name,
                                          TcamPropertyType type,
                                          const gchar* displayname,
                                          const gchar* description,
                                          const gchar* unit,
                                          const gchar* category,
                                          TcamPropertyVisibility visibility)
{
    TcamPropertyInfo* t = g_try_new(TcamPropertyInfo, 1);

    if (!t)
    {
        return NULL;
    }

    t->name = name;
    t->type = type;
    t->display_name = displayname;
    t->description = description;
    t->unit = unit;
    t->category = category;


    /* t->name = g_strdup(name); */
    /* t->display_name = g_strdup("displayname"); */
    /* t->description = g_strdup(description); */
    /* t->unit = g_strdup(unit); */
    /* t->category = g_strdup(category); */

    t->visibility = visibility;

    return t;

}

TcamPropertyInfo* tcam_property_info_copy (TcamPropertyInfo* data)
{
    TcamPropertyInfo* t = g_malloc(sizeof(TcamPropertyInfo));

    t->name = data->name;
    t->display_name = data->display_name;
    t->description = data->description;
    t->unit = data->unit;
    t->category = data->category;

    t->type = data->type;
    t->visibility = data->visibility;

    /* t->name = g_strdup(data->name); */
    /* t->display_name = g_strdup( "data->display_name"); */
    /* t->display_name = g_strdup(data->display_name); */
    /* t->description = g_strdup(data->description); */
    /* t->unit = g_strdup(data->unit); */
    /* t->category = g_strdup(data->category); */

    /* t->visibility = data->visibility; */

    return t;

}

void tcam_property_info_free (TcamPropertyInfo* t)
{
    /* g_free(t->display_name); */
    /* g_free(t->description); */
    /* g_free(t->unit); */
    /* g_free(t->category); */


    g_free((gpointer)t);
}



GType tcam_property_info_get_type (void) {
    static GType type = 0;

    if (G_UNLIKELY (!type))
        type = g_boxed_type_register_static ("TcamPropertyInfo",
                                             (GBoxedCopyFunc) tcam_property_info_copy,
                                             (GBoxedFreeFunc) tcam_property_info_free);

    return type;
}

//
// actual tcamprop
//

static void tcam_prop_default_init(__attribute__((unused)) TcamPropInterface* klass)
{
    /* GObjectClass *object_class = G_OBJECT_CLASS (klass); */

    /* object_class->get_property = tcam_prop_get_property; */
    /* object_class->set_property = tcam_prop_set_property; */
    // object_class->dispose = tcam_prop_dispose;
    // object_class->finalize = tcam_prop_finalize;

    tcam_error_quark();

    tcam_error_get_type();

    tcam_property_flags_get_type();

    tcam_property_visibility_get_type();

    tcam_property_type_get_type();

}


/**
 * tcam_prop_get_tcam_property_names:
 * @self: a #TcamProp
 *
 * Return a list of property names
 *
 * Returns: (element-type utf8) (transfer full):  a #GSList list of property names
 */
GSList* tcam_prop_get_tcam_property_names(TcamProp* self, GError** err)
{
    GSList* ret = NULL;
    TcamPropInterface* iface;

    g_return_val_if_fail(self != NULL, NULL);
    g_return_val_if_fail(TCAM_IS_PROP(self), NULL);

    iface = TCAM_PROP_GET_IFACE(self);

    if (iface->get_tcam_property_names)
    {
        ret = iface->get_tcam_property_names(self, err);
    }

    return ret;
}

/**
 * tcam_prop_get_tcam_property_info:
 * @self: a #TcamProp
 * @name: a #char* identifying the property name
 * @err: (nullable): a #GError** , may be NULL
 *
 * Returns: (transfer full): a #TcamPropertyInfo
 */
TcamPropertyInfo* tcam_prop_get_property_info(TcamProp* self, const char* name, GError** err)
{
    g_return_val_if_fail(self != NULL, NULL);
    g_return_val_if_fail(TCAM_IS_PROP(self), NULL);

    TcamPropInterface* iface = TCAM_PROP_GET_IFACE(self);

    if (iface->get_tcam_info)
    {
        return iface->get_tcam_info(self, name, err);
    }

    return NULL;
}


/**
 * tcam_prop_get_tcam_enum_entries:
 * @self: a #TcamProp
 * @name: a #char* identifying the property name
 * @err: (nullable): a #GError** , may be NULL
 *
 * Returns: (element-type utf8) (transfer full): a #GSList
 *          free the list with g_slist_free and the elements with g_free when done.
 */
GSList* tcam_prop_get_tcam_enum_entries(TcamProp* self, const char* name, GError** err)
{
    GSList* ret = NULL;
    TcamPropInterface* iface;

    g_return_val_if_fail(self != NULL, NULL);
    g_return_val_if_fail(TCAM_IS_PROP(self), NULL);

    iface = TCAM_PROP_GET_IFACE(self);

    if (iface->get_tcam_enum_entries)
    {
        ret = iface->get_tcam_enum_entries(self, name, err);
    }

    return ret;
}


/**
 * tcam_prop_get_tcam_property_type:
 * @self: a #TcamProp
 * @name: a #char* identifying the property
 * @err: (nullable): a #GError** , may be NULL
 *
 * Returns: a #TcamPropertyType
 *          In case of an error TCAM_PROPERTY_UNKNOWN will be returned
 */
TcamPropertyType tcam_prop_get_tcam_property_type(TcamProp* self,
                                                  const gchar* name,
                                                  GError** err)
{
    TcamPropertyType ret = TCAM_PROPERTY_UNKNOWN;
    TcamPropInterface* iface;

    g_return_val_if_fail(self != NULL, TCAM_PROPERTY_UNKNOWN);
    g_return_val_if_fail(TCAM_IS_PROP(self), TCAM_PROPERTY_UNKNOWN);
    g_return_val_if_fail(name != NULL, TCAM_PROPERTY_UNKNOWN);

    iface = TCAM_PROP_GET_IFACE(self);

    if (iface->get_tcam_type)
    {
        ret = iface->get_tcam_type(self, name, err);
    }

    return ret;
}



/**
 * tcam_prop_get_tcam_range:
 * @self: a #TcamProp
 * @name: a #char* identifying the property
 * @min: (out)(optional): a #GValue will contain the lowest possible value
 * @max: (out)(optional):a #GValue will contain the highest possible value
 * @step: (out)(optional): a #GValue will contain the step size
 * @err: (nullable): a #GError** , may be NULL
 *
 *  Returns: a #gboolean, TRUE ion success
 */
gboolean tcam_prop_get_tcam_range(TcamProp* self,
                              const char* name,
                              GValue* min, GValue* max,
                              GValue* step,
                              GError** err)
{
    TcamPropInterface* iface;

    g_return_val_if_fail(self != NULL, FALSE);
    g_return_val_if_fail(TCAM_IS_PROP(self), FALSE);
    g_return_val_if_fail(name != NULL, FALSE);

    iface = TCAM_PROP_GET_IFACE(self);

    if (iface->get_tcam_range)
    {
        return iface->get_tcam_range(self, name, min, max, step, err);
    }
    return FALSE;
}


/**
 * tcam_prop_get_tcam_default:
 * @self: a #TcamProp
 * @name: a #char* identifying the property
 * @default_value: (out)(optional): a #GValue will contain the default value
 * @err: (nullable): a #GError** , may be NULL
 *
 * Returns: A #gboolean indicating the success of the operation
 */
gboolean tcam_prop_get_tcam_default(TcamProp* self,
                                    const char* name, GValue* default_value,
                                    GError** err)
{
    TcamPropInterface* iface;

    g_return_val_if_fail(self != NULL, FALSE);
    g_return_val_if_fail(TCAM_IS_PROP(self), FALSE);
    g_return_val_if_fail(name != NULL, FALSE);

    iface = TCAM_PROP_GET_IFACE(self);

    if (iface->get_tcam_default)
    {
        return iface->get_tcam_default(self, name, default_value, err);
    }

    return FALSE;
}


gboolean tcam_prop_get_tcam_property(TcamProp* self,
                                     const gchar* name,
                                     GValue* value,
                                     GValue* flags,
                                     GError** err)
{
    TcamPropInterface* iface;

    g_return_val_if_fail(self != NULL, FALSE);
    g_return_val_if_fail(TCAM_IS_PROP(self), FALSE);
    g_return_val_if_fail(name != NULL, FALSE);

    iface = TCAM_PROP_GET_IFACE(self);

    if (iface->get_tcam_property)
    {
        return iface->get_tcam_property(self, name, value, flags, err);
    }

    return FALSE;
}


/**
 * tcam_prop_get_tcam_string:
 * @self: a #TcamProp
 * @name: (in): name of the property
 *
 * Returns: (transfer full): A string describing the property value
 */
const char* tcam_prop_get_tcam_string(TcamProp* self,
                                      const char* name,
                                      GError** err)
{
    g_return_val_if_fail(self, NULL);
    g_return_val_if_fail(TCAM_IS_PROP(self), NULL);
    g_return_val_if_fail(name, NULL);

    TcamPropInterface* iface = TCAM_PROP_GET_IFACE(self);

    if (iface->get_tcam_string)
    {
        return iface->get_tcam_string(self, name, err);
    }
    return NULL;
}


/**
 * tcam_prop_get_tcam_int:
 * @self: a #TcamProp
 * @name: (in): name of the property
 *
 * Returns: A #gint64 describing the property value
 */
gint64 tcam_prop_get_tcam_int(TcamProp* self,
                              const char* name,
                              GError** err)
{
    g_return_val_if_fail(self, -1);
    g_return_val_if_fail(TCAM_IS_PROP(self), -1);
    g_return_val_if_fail(name, -1);

    TcamPropInterface* iface = TCAM_PROP_GET_IFACE(self);

    if (iface->get_tcam_int)
    {
        return iface->get_tcam_int(self, name, err);
    }
    return -1;
}


/**
 * tcam_prop_get_tcam_double:
 * @self: a #TcamProp
 * @name: (in): name of the property
 *
 * Returns: A #gdouble describing the property value
 */
gdouble tcam_prop_get_tcam_double(TcamProp* self,
                                  const char* name,
                                  GError** err)
{
    g_return_val_if_fail(self, -1.0);
    g_return_val_if_fail(TCAM_IS_PROP(self), -1.0);
    g_return_val_if_fail(name, -1.0);

    TcamPropInterface* iface = TCAM_PROP_GET_IFACE(self);

    if (iface->get_tcam_double)
    {
        return iface->get_tcam_double(self, name, err);
    }
    return -1.0;
}


/**
 * tcam_prop_get_tcam_bool:
 * @self: a #TcamProp
 * @name: (in): name of the property
 *
 * Returns: A #gboolean describing the property value
 */
gboolean tcam_prop_get_tcam_bool(TcamProp* self,
                                 const char* name,
                                 GError** err)
{
    g_return_val_if_fail(self, FALSE);
    g_return_val_if_fail(TCAM_IS_PROP(self), FALSE);
    g_return_val_if_fail(name, FALSE);

    TcamPropInterface* iface = TCAM_PROP_GET_IFACE(self);

    if (iface->get_tcam_bool)
    {
        return iface->get_tcam_bool(self, name, err);
    }
    return FALSE;
}



















/**
 * tcam_prop_set_tcam_property:
 * @self: a #TcamProp
 * @name: a #char* identifying the property to set
 * @value: (in): a #GValue
 *
 * Set a property
 *
 * Returns: true on success
 */
gboolean tcam_prop_set_tcam_property(TcamProp* self, const gchar* name, const GValue* value, GError** err)
{
    TcamPropInterface* iface;
    gboolean ret = FALSE;

    g_return_val_if_fail(self != NULL, FALSE);
    g_return_val_if_fail(TCAM_IS_PROP(self), FALSE);
    g_return_val_if_fail(name != NULL, FALSE);
    g_return_val_if_fail(value != NULL, FALSE);

    iface = TCAM_PROP_GET_IFACE(self);

    if (iface->set_tcam_property)
    {
        ret = iface->set_tcam_property(self, name, value, err);
    }

    return ret;
}


/**
 * tcam_prop_set_tcam:
 * @self: a #TcamProp
 * @name: (in): a #char* indentifying to property to set
 * @value: (in): a #char*
 * @err: (out): a #GError**
 *
 * Set a string
 *
 */
gboolean tcam_prop_set_tcam_string(TcamProp* self,
                                   const char* name,
                                   const char* value,
                                   GError** err)
{
    TcamPropInterface* iface;

    g_return_val_if_fail(self != NULL, FALSE);
    g_return_val_if_fail(TCAM_IS_PROP(self), FALSE);
    g_return_val_if_fail(name != NULL, FALSE);
    g_return_val_if_fail(value != NULL, FALSE);

    iface = TCAM_PROP_GET_IFACE(self);

    if (iface->set_tcam_string)
    {
        return iface->set_tcam_string(self, name, value, err);
    }
    return FALSE;
}


/**
 * tcam_prop_set_tcam_int:
 * @self: a #TcamProp
 * @name: (in): a #char* indentifying to property to set
 * @value: (in): a #gint64
 * @err: (out): a #GError**
 *
 * Set an integer
 *
 */
gboolean tcam_prop_set_tcam_int(TcamProp* self,
                                const char* name,
                                gint64 value,
                                GError** err)
{
    TcamPropInterface* iface;

    g_return_val_if_fail(self != NULL, FALSE);
    g_return_val_if_fail(TCAM_IS_PROP(self), FALSE);
    g_return_val_if_fail(name != NULL, FALSE);

    iface = TCAM_PROP_GET_IFACE(self);

    if (iface->set_tcam_int)
    {
        return iface->set_tcam_int(self, name, value, err);
    }
    return FALSE;
}


/**
 * tcam_prop_set_tcam_double:
 * @self: a #TcamProp
 * @name: (in): a #char* indentifying to property to set
 * @value: (in): a #gdbouble
 * @err: (out): a #GError**
 *
 * Set a double
 *
 */
gboolean tcam_prop_set_tcam_double(TcamProp* self,
                                   const char* name,
                                   gdouble value,
                                   GError** err)
{
    TcamPropInterface* iface;

    g_return_val_if_fail(self != NULL, FALSE);
    g_return_val_if_fail(TCAM_IS_PROP(self), FALSE);
    g_return_val_if_fail(name != NULL, FALSE);

    iface = TCAM_PROP_GET_IFACE(self);

    if (iface->set_tcam_double)
    {
        return iface->set_tcam_double(self, name, value, err);
    }
    return FALSE;
}


/**
 * tcam_prop_set_tcam_bool:
 * @self: a #TcamProp
 * @name: (in): a #char* indentifying to property to set
 * @value: (in): a #gboolean
 * @err: (out): a #GError**
 *
 * Set a boolean
 *
 */
gboolean tcam_prop_set_tcam_bool(TcamProp* self,
                                 const char* name,
                                 gboolean value,
                                 GError** err)
{
    TcamPropInterface* iface;

    g_return_val_if_fail(self != NULL, FALSE);
    g_return_val_if_fail(TCAM_IS_PROP(self), FALSE);
    g_return_val_if_fail(name != NULL, FALSE);

    iface = TCAM_PROP_GET_IFACE(self);

    if (iface->set_tcam_bool)
    {
        return iface->set_tcam_bool(self, name, value, err);
    }
    return FALSE;
}


/**
 * tcam_prop_set_tcam_execute:
 * @self: a #TcamProp
 * @name: (in): a #char* indentifying to property to set
 * @err: (out): a #GError**
 *
 * Execute a command
 *
 */
gboolean tcam_prop_set_tcam_execute(TcamProp* self,
                                    const char* name,
                                    GError** err)
{
    g_return_val_if_fail(self != NULL, FALSE);
    g_return_val_if_fail(TCAM_IS_PROP(self), FALSE);
    g_return_val_if_fail(name != NULL, FALSE);

    TcamPropInterface* iface = TCAM_PROP_GET_IFACE(self);

    if (iface->set_tcam_execute)
    {
        return iface->set_tcam_execute(self, name, err);
    }
    return FALSE;
}
