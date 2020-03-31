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

G_DEFINE_INTERFACE (TcamProp, tcam_prop, G_TYPE_OBJECT)


static void tcam_prop_default_init (__attribute__((unused))TcamPropInterface* klass)
{
    /* GObjectClass *object_class = G_OBJECT_CLASS (klass); */

    /* object_class->get_property = tcam_prop_get_property; */
    /* object_class->set_property = tcam_prop_set_property; */
    // object_class->dispose = tcam_prop_dispose;
    // object_class->finalize = tcam_prop_finalize;
}


/**
 * tcam_prop_get_tcam_property_names:
 * @self: a #TcamProp
 *
 * Return a list of property names
 *
 * Returns: (element-type utf8) (transfer full): list of property names
 */
GSList* tcam_prop_get_tcam_property_names (TcamProp* self)
{
    GSList* ret = NULL;
    TcamPropInterface* iface;

    g_return_val_if_fail (self != NULL, NULL);
    g_return_val_if_fail (TCAM_IS_PROP (self), NULL);

    iface = TCAM_PROP_GET_IFACE (self);

    if (iface->get_property_names)
    {
	    ret = iface->get_property_names (self);
    }

    return ret;
}


/**
 * tcam_prop_get_tcam_property_type:
 * @self: a #TcamProp
 * @name: a #char* identifying the property to query
 *
 * Return the type of a property
 *
 * Returns: (transfer full): A string describing the property type
 */
const gchar* tcam_prop_get_tcam_property_type (TcamProp* self, const gchar* name)
{
    TcamPropInterface* iface;
    gchar* ret = NULL;

    g_return_val_if_fail (self != NULL, NULL);
    g_return_val_if_fail (TCAM_IS_PROP (self), NULL);
    g_return_val_if_fail (name != NULL, NULL);

    iface = TCAM_PROP_GET_IFACE (self);

    if (iface->get_property_names)
    {
	    ret = iface->get_property_type (self, name);
    }
    return ret;
}


/**
 * tcam_prop_get_tcam_property:
 * @self: a #TcamProp
 * @name: a #char* identifying the property to query
 * @value: (out) (optional): a #GValue
 * @min: (out) (optional):a #GValue
 * @max: (out) (optional): a #GValue
 * @def: (out) (optional): a #GValue
 * @step: (out) (optional): a #GValue
 * @type: (out) (optional): a #GValue
 * @flags: (out) (optional): a #GValue
 * @category: (out) (optional): a #GValue
 * @group: (out) (optional): a #GValue
 *
 * Read a property
 *
 * Returns: True on success
 */
gboolean tcam_prop_get_tcam_property (TcamProp* self,
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
    TcamPropInterface* iface;
    gboolean ret = FALSE;

    g_return_val_if_fail (self != NULL, FALSE);
    g_return_val_if_fail (TCAM_IS_PROP (self), FALSE);
    g_return_val_if_fail (name != NULL, FALSE);

    iface = TCAM_PROP_GET_IFACE (self);

    if (iface->get_property)
    {
        ret = iface->get_property (self, name,
                                   value,
                                   min, max,
                                   def, step,
                                   type,
                                   flags,
                                   category, group);
    }

    return ret;
}



/**
 * tcam_prop_get_tcam_menu_entries:
 * @self: a #TcamProp
 * @name: a #char* identifying the menu name
 *
 * Returns: (element-type utf8) (transfer full): a #GSList
 */
GSList* tcam_prop_get_tcam_menu_entries (TcamProp* self,
                                         const char* name)
{
    GSList* ret = NULL;
    TcamPropInterface* iface;

    g_return_val_if_fail (self != NULL, NULL);
    g_return_val_if_fail (TCAM_IS_PROP (self), NULL);

    iface = TCAM_PROP_GET_IFACE (self);

    if (iface->get_menu_entries)
    {
	    ret = iface->get_menu_entries (self, name);
    }

    return ret;
}


/**
 * tcam_prop_set_tcam_property:
 * @self: a #TcamProp
 * @name: a #char* identifying the property to query
 * @value: (in): a #GValue
 *
 * Set a property
 *
 * Returns: true on success
 */
gboolean tcam_prop_set_tcam_property (TcamProp* self,
                                      const gchar* name,
                                      const GValue* value)
{
    TcamPropInterface *iface;
    gboolean ret = FALSE;

    g_return_val_if_fail (self != NULL, FALSE);
    g_return_val_if_fail (TCAM_IS_PROP (self), FALSE);
    g_return_val_if_fail (name != NULL, FALSE);
    g_return_val_if_fail (value != NULL, FALSE);

    iface = TCAM_PROP_GET_IFACE (self);

    if (iface->set_property)
    {
        ret = iface->set_property (self, name, value);
    }

    return ret;
}


/**
 * tcam_prop_get_device_serials:
 * @self: a #TcamProp
 *
 * Retrieve a list of all connected device serial numbers
 *
 * Returns: (element-type utf8) (transfer full): a #GSList
 */
GSList* tcam_prop_get_device_serials (TcamProp* self)
{
    TcamPropInterface* iface;
    GSList* ret = NULL;

    g_return_val_if_fail (self != NULL, NULL);
    g_return_val_if_fail (TCAM_IS_PROP (self), NULL);

    iface = TCAM_PROP_GET_IFACE (self);

    if (iface->get_device_serials)
    {
        ret = iface->get_device_serials (self);
    }

    return ret;
}


/**
 * tcam_prop_get_device_serials_backend:
 * @self: a #TcamProp
 *
 * Retrieve a list of all connected device serial numbers
 * Appended to the serial number is the backend,
 * serparated by a hyphen "-".
 *
 * Returns: (element-type utf8) (transfer full): a #GSList
 */
GSList* tcam_prop_get_device_serials_backend (TcamProp* self)
{
    TcamPropInterface* iface;
    GSList* ret = NULL;

    g_return_val_if_fail (self != NULL, NULL);
    g_return_val_if_fail (TCAM_IS_PROP (self), NULL);

    iface = TCAM_PROP_GET_IFACE (self);

    if (iface->get_device_serials_backend)
    {
        ret = iface->get_device_serials_backend (self);
    }

    return ret;
}


/**
 * tcam_prop_get_device_info:
 * @self: a #TcamProp
 * @serial: (in): serial number of camera to query
 * @name: (out) (optional): location to store an allocated string.
 *               Use g_free() to free the returned string
 * @identifier: (out) (optional): location to store an allocated string.
 *                     Use g_free() to free the returned string
 * @connection_type: (out) (optional): location to store an allocated string.
 *                          Use g_free() to free the returned string
 *
 * Get details of a given camera.
 *
 * Returns: True on success
 */
gboolean tcam_prop_get_device_info (TcamProp* self,
                                    const char* serial,
                                    char** name,
                                    char** identifier,
                                    char** connection_type)
{
    TcamPropInterface* iface;
    gboolean ret = FALSE;

    g_return_val_if_fail (self != NULL, FALSE);
    g_return_val_if_fail (TCAM_IS_PROP (self), FALSE);
    g_return_val_if_fail (serial != NULL, FALSE);

    iface = TCAM_PROP_GET_IFACE (self);

    if (iface->get_device_info)
    {
        ret = iface->get_device_info (self,
                                      serial,
                                      name,
                                      identifier,
                                      connection_type);
    }

    return ret;
}
