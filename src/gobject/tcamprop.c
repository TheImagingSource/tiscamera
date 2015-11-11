#include "tcamprop.h"

G_DEFINE_INTERFACE (TcamProp, tcam_prop, G_TYPE_OBJECT)


static void tcam_prop_default_init (TcamPropInterface *klass)
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
    GSList *ret = NULL;
    TcamPropInterface *iface;

    g_return_val_if_fail (self != NULL, NULL);
    g_return_val_if_fail (TCAM_IS_PROP (self), NULL);

    iface = TCAM_PROP_GET_IFACE (self);

    if (iface->get_property_names){
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
gchar *
tcam_prop_get_tcam_property_type (TcamProp *self, gchar *name)
{
    TcamPropInterface *iface;
    gchar *ret;

    g_return_val_if_fail (self != NULL, NULL);
    g_return_val_if_fail (TCAM_IS_PROP (self), NULL);
    g_return_val_if_fail (name != NULL, NULL);

    iface = TCAM_PROP_GET_IFACE (self);

    if (iface->get_property_names){
	    ret = iface->get_property_type (self, name);
    }

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
 *
 * Read a property
 *
 * Returns: True on success
 */
gboolean tcam_prop_get_tcam_property (TcamProp *self,
				      gchar *name,
				      GValue *value,
				      GValue *min,
				      GValue *max,
				      GValue *def,
				      GValue *step,
				      GValue *type)
{
    TcamPropInterface *iface;
    gboolean ret;

    g_return_val_if_fail (self != NULL, FALSE);
    g_return_val_if_fail (TCAM_IS_PROP (self), FALSE);
    g_return_val_if_fail (name != NULL, FALSE);

    iface = TCAM_PROP_GET_IFACE (self);

    if (iface->get_property){
	ret = iface->get_property (self, name,
				   value, min, max, def, step, type);
    }

    return ret;
}

/**
 * tcam_prop_get_tcam_property:
 * @self: a #TcamProp
 * @name: a #char* identifying the property to query
 * @value: (in): a #GValue
 *
 * Set a property
 *
 * Returns: true on success
 */
gboolean tcam_prop_set_tcam_property (TcamProp *self,
				      gchar *name,
				      const GValue *value)
{
    TcamPropInterface *iface;
    gboolean ret;

    g_return_val_if_fail (self != NULL, FALSE);
    g_return_val_if_fail (TCAM_IS_PROP (self), FALSE);
    g_return_val_if_fail (name != NULL, FALSE);
    g_return_val_if_fail (value != NULL, FALSE);

    iface = TCAM_PROP_GET_IFACE (self);

    if (iface->set_property){
	ret = iface->set_property (self, name, value);
    }

    return ret;
}

