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

#include "tcam-property-1.0.h"

 //---------------------------------
 // Enumerations

static const GEnumValue _tcam_error_values[] =
{
    { TCAM_ERROR_SUCCESS,                       "TCAM_ERROR_SUCCESS", "no-error" },
    { TCAM_ERROR_UNKNOWN,                       "TCAM_ERROR_UNKNOWN", "unknown-error" },
    { TCAM_ERROR_TIMEOUT,                       "TCAM_ERROR_TIMEOUT", "timeout" },
    { TCAM_ERROR_NOT_IMPLEMENTED,               "TCAM_ERROR_NOT_IMPLEMENTED", "not-implemented" },
    { TCAM_ERROR_PARAMETER_INVALID,             "TCAM_ERROR_PARAMETER_INVALID", "parameter-invalid" },

    { TCAM_ERROR_PROPERTY_NOT_IMPLEMENTED,      "TCAM_ERROR_PROPERTY_NOT_IMPLEMENTED", "property-not-found" },
    { TCAM_ERROR_PROPERTY_NOT_AVAILABLE,        "TCAM_ERROR_PROPERTY_NOT_AVAILABLE", "not-available" },
    { TCAM_ERROR_PROPERTY_NOT_WRITEABLE,        "TCAM_ERROR_PROPERTY_NOT_WRITEABLE", "not-write-able" },
    { TCAM_ERROR_PROPERTY_VALUE_OUT_OF_RANGE,   "TCAM_ERROR_PROPERTY_VALUE_OUT_OF_RANGE", "property-value-out-of-range" },
    { TCAM_ERROR_PROPERTY_DEFAULT_NOT_AVAILABLE,"TCAM_ERROR_PROPERTY_DEFAULT_NOT_AVAILABLE", "property-default-not-available" },
    { TCAM_ERROR_PROPERTY_TYPE_INCOMPATIBLE,    "TCAM_ERROR_PROPERTY_TYPE_INCOMPATIBLE", "incompatible-type" },
    
    { TCAM_ERROR_DEVICE_NOT_OPENED,             "TCAM_ERROR_DEVICE_NOT_OPENED", "device-not-opened" },
    { TCAM_ERROR_DEVICE_LOST,                   "TCAM_ERROR_DEVICE_LOST", "device-lost" },
    { TCAM_ERROR_DEVICE_NOT_ACCESSIBLE,         "TCAM_ERROR_DEVICE_NOT_ACCESSIBLE", "device-not-accessible" },

    { 0, NULL, NULL }
};

GType tcam_error_get_type( void )
{
    static GType type = 0;
    if (g_once_init_enter(&type))
    {
        GType tmp = g_enum_register_static( "TcamError", _tcam_error_values );
        g_once_init_leave(&type, tmp);
    }
    return type;
}

GQuark tcam_error_quark( void )
{
    return g_quark_from_static_string( "tcam-error-quark" );
}

static const GEnumValue _tcamprop_property_visibility_values[] = {
     { TCAM_PROPERTY_VISIBILITY_BEGINNER,    "TCAM_VISIBILITY_BEGINNER", "beginner" },
     { TCAM_PROPERTY_VISIBILITY_EXPERT,      "TCAM_VISIBILITY_EXPERT", "expert" },
     { TCAM_PROPERTY_VISIBILITY_GURU,        "TCAM_VISIBILITY_GURU", "guru" },
     { TCAM_PROPERTY_VISIBILITY_INVISIBLE,   "TCAM_VISIBILITY_INVISIBLE", "invisible" },
     { 0, NULL, NULL }
};


GType tcam_property_visibility_get_type( void )
{
    static GType type = 0;
    if (g_once_init_enter(&type))
    {
        GType tmp = g_enum_register_static( "TcamPropertyVisibility", _tcamprop_property_visibility_values );
        g_once_init_leave(&type, tmp);
    }
    return type;
}

static const GEnumValue _tcamprop_property_access_values[] = {
    { TCAM_PROPERTY_ACCESS_RW, "TCAM_PROPERTY_ACCESS_RW", "rw" },
    { TCAM_PROPERTY_ACCESS_RO, "TCAM_PROPERTY_ACCESS_RO", "ro" },
    { TCAM_PROPERTY_ACCESS_WO, "TCAM_PROPERTY_ACCESS_WO", "wo" },
    { 0, NULL, NULL }
};


GType tcam_property_access_get_type(void)
{
    static GType type = 0;
    if (g_once_init_enter(&type))
    {
        GType tmp = g_enum_register_static("TcamPropertyAccess", _tcamprop_property_access_values);
        g_once_init_leave(&type, tmp);
    }
    return type;
}

static const GEnumValue _tcamprop_property_intrepresentation_values[] = {
    { TCAM_PROPERTY_INTREPRESENTATION_LINEAR, "TCAM_INTREPRESENTATION_LINEAR", "linear" },
    { TCAM_PROPERTY_INTREPRESENTATION_LOGARITHMIC, "TCAM_INTREPRESENTATION_LOGARITHMIC", "logarithmic" },
    { TCAM_PROPERTY_INTREPRESENTATION_PURENUMBER, "TCAM_INTREPRESENTATION_PURENUMBER", "purenumber" },
    { TCAM_PROPERTY_INTREPRESENTATION_HEXNUMBER, "TCAM_INTREPRESENTATION_HEXNUMBER", "hexnumber" },
    { 0, NULL, NULL }
};


GType tcam_property_intrepresentation_get_type( void )
{
    static GType type = 0;
    if (g_once_init_enter(&type))
    {
        GType tmp = g_enum_register_static( "TcamPropertyIntRepresentation", _tcamprop_property_intrepresentation_values );
        g_once_init_leave(&type, tmp);
    }
    return type;
}

static const GEnumValue _tcamprop_property_floatrepresentation_values[] = {
    { TCAM_PROPERTY_FLOATREPRESENTATION_LINEAR,      "TCAM_FLOATREPRESENTATION_LINEAR", "linear" },
    { TCAM_PROPERTY_FLOATREPRESENTATION_LOGARITHMIC, "TCAM_FLOATREPRESENTATION_LOGARITHMIC", "logarithmic" },
    { TCAM_PROPERTY_FLOATREPRESENTATION_PURENUMBER,  "TCAM_FLOATREPRESENTATION_PURENUMBER", "purenumber" },
    { 0, NULL, NULL }
};


GType tcam_property_floatrepresentation_get_type( void )
{
    static GType type = 0;
    if (g_once_init_enter(&type))
    {
        GType tmp = g_enum_register_static( "TcamPropertyFloatRepresentation", _tcamprop_property_floatrepresentation_values );
        g_once_init_leave(&type, tmp);
    }
    return type;
}

static const GEnumValue _tcam_property_type_values[] = {
    { TCAM_PROPERTY_TYPE_INTEGER   , "TCAM_PROPERTY_TYPE_INTEGER", "integer" },
    { TCAM_PROPERTY_TYPE_FLOAT, "TCAM_PROPERTY_TYPE_FLOAT", "float" },
    { TCAM_PROPERTY_TYPE_ENUMERATION  , "TCAM_PROPERTY_TYPE_ENUMERATION", "enumeration" },
    { TCAM_PROPERTY_TYPE_BOOLEAN  , "TCAM_PROPERTY_TYPE_BOOLEAN", "boolean" },
    { TCAM_PROPERTY_TYPE_COMMAND, "TCAM_PROPERTY_TYPE_COMMAND", "command" },
    { TCAM_PROPERTY_TYPE_STRING, "TCAM_PROPERTY_TYPE_STRING", "string" },
    { 0, NULL, NULL }
};

GType tcam_property_type_get_type (void)
{
    static GType type = 0;
    if (g_once_init_enter(&type))
    {
        GType tmp = g_enum_register_static("TcamPropertyType", _tcam_property_type_values);
        g_once_init_leave(&type, tmp);
    }
    return type;
}

//-------------------------------------------
// Prop-Nodes:

G_DEFINE_INTERFACE( TcamPropertyBase, tcam_property_base, G_TYPE_OBJECT )

static void tcam_property_base_default_init( __attribute__ ((unused)) TcamPropertyBaseInterface* instance )
{

}

/**
 * tcam_property_base_get_name:
 * @self: A #TcamPropertyBase
 *
 * Returns: (type utf8) (transfer none): The name of this property.
 */
const gchar* tcam_property_base_get_name( TcamPropertyBase* self )
{
    g_return_val_if_fail( self != NULL, NULL );
    g_return_val_if_fail( TCAM_IS_PROPERTY_BASE( self ), NULL );

    TcamPropertyBaseInterface* iface = TCAM_PROPERTY_BASE_GET_IFACE( self );
    if( iface->get_name )
    {
        return iface->get_name( self );
    }
    return NULL;
}

/**
 * tcam_property_base_get_display_name:
 * @self: A #TcamPropertyBase
 *
 * Returns: (type utf8) (transfer none): Return the display name for this property.
 */
const gchar* tcam_property_base_get_display_name( TcamPropertyBase* self )
{
    g_return_val_if_fail( self != NULL, NULL );
    g_return_val_if_fail( TCAM_IS_PROPERTY_BASE( self ), NULL );

    TcamPropertyBaseInterface* iface = TCAM_PROPERTY_BASE_GET_IFACE( self );
    if( iface->get_display_name )
    {
        return iface->get_display_name( self );
    }
    return NULL;
}

/**
 * tcam_property_base_get_description:
 * @self: A #TcamPropertyBase
 *
 * Returns: (type utf8) (transfer none): Return the description for this property.
 */
const gchar* tcam_property_base_get_description( TcamPropertyBase* self )
{
    g_return_val_if_fail( self != NULL, NULL );
    g_return_val_if_fail( TCAM_IS_PROPERTY_BASE( self ), NULL );

    TcamPropertyBaseInterface* iface = TCAM_PROPERTY_BASE_GET_IFACE( self );
    if( iface->get_description )
    {
        return iface->get_description( self );
    }
    return NULL;
}

/**
 * tcam_property_base_get_category:
 * @self: A #TcamPropertyBase
 *
 * Returns: (type utf8) (transfer none): Return the category for this property.
 */
const gchar* tcam_property_base_get_category( TcamPropertyBase* self )
{
    g_return_val_if_fail( self != NULL, NULL );
    g_return_val_if_fail( TCAM_IS_PROPERTY_BASE( self ), NULL );

    TcamPropertyBaseInterface* iface = TCAM_PROPERTY_BASE_GET_IFACE( self );
    if( iface->get_category )
    {
        return iface->get_category( self );
    }
    return NULL;
}

/**
 * tcam_property_base_get_visibility:
 * @self: A #TcamPropertyBase
 *
 * Returns: Return the #TcamPropertyVisibility for this property.
 */
TcamPropertyVisibility tcam_property_base_get_visibility( TcamPropertyBase* self )
{
    g_return_val_if_fail( self != NULL, 0 );
    g_return_val_if_fail( TCAM_IS_PROPERTY_BASE( self ), 0 );

    TcamPropertyBaseInterface* iface = TCAM_PROPERTY_BASE_GET_IFACE( self );
    if( iface->get_visibility )
    {
        return iface->get_visibility( self );
    }
    return TCAM_PROPERTY_VISIBILITY_INVISIBLE;
}

/**
 * tcam_property_base_get_access:
 * @self: A #TcamPropertyBase
 *
 * Returns: Return the #TcamPropertyAccess for this property.
 */
TcamPropertyAccess tcam_property_base_get_access(TcamPropertyBase* self) 
{
    g_return_val_if_fail(self != NULL, 0);
    g_return_val_if_fail(TCAM_IS_PROPERTY_BASE(self), 0);

    TcamPropertyBaseInterface* iface = TCAM_PROPERTY_BASE_GET_IFACE(self);
    if (iface->get_access)
    {
        return iface->get_access(self);
    }
    return TCAM_PROPERTY_ACCESS_RW;
}

/**
 * tcam_property_base_get_property_type:
 * @self: A #TcamPropertyBase
 *
 * Returns: Return the #TcamPropertyType for this property.
 */
TcamPropertyType tcam_property_base_get_property_type( TcamPropertyBase* self )
{
    g_return_val_if_fail( self != NULL, 0 );
    g_return_val_if_fail( TCAM_IS_PROPERTY_BASE( self ), 0 );

    TcamPropertyBaseInterface* iface = TCAM_PROPERTY_BASE_GET_IFACE( self );
    if( iface->get_property_type )
    {
        return iface->get_property_type( self );
    }
    return 0;
}

/**
 * tcam_property_base_is_available:
 * @self: A #TcamPropertyBase
 * @err: return location for a GError, or NULL
 *
 * Returns: Return if the property is 'available'.
 */
gboolean        tcam_property_base_is_available( TcamPropertyBase* self, GError** err )
{
    g_return_val_if_fail( self != NULL, FALSE );
	g_return_val_if_fail( err == NULL || *err == NULL, FALSE);
    g_return_val_if_fail( TCAM_IS_PROPERTY_BASE( self ), FALSE );

    TcamPropertyBaseInterface* iface = TCAM_PROPERTY_BASE_GET_IFACE( self );
    if( iface->is_available )
    {
        return iface->is_available( self, err );
    }
    return FALSE;
}

/**
 * tcam_property_base_is_locked:
 * @self: A #TcamPropertyBase
 * @err: return location for a GError, or NULL
 *
 * Returns: Return if the property is 'locked'.
 */
gboolean        tcam_property_base_is_locked( TcamPropertyBase* self, GError** err )
{
    g_return_val_if_fail( self != NULL, FALSE );
    g_return_val_if_fail( err == NULL || *err == NULL, FALSE );
    g_return_val_if_fail( TCAM_IS_PROPERTY_BASE( self ), FALSE );

    TcamPropertyBaseInterface* iface = TCAM_PROPERTY_BASE_GET_IFACE( self );
    if( iface->is_locked )
    {
        return iface->is_locked( self, err );
    }
    return FALSE;
}

G_DEFINE_INTERFACE( TcamPropertyBoolean, tcam_property_boolean, TCAM_TYPE_PROPERTY_BASE )

static void tcam_property_boolean_default_init( __attribute__ ((unused)) TcamPropertyBooleanInterface* instance )
{}

/**
 * tcam_property_boolean_get_value:
 * @self: A #TcamPropertyBoolean
 * @err: return location for a GError, or NULL
 *
 * Returns: The current value of the property.
 */
gboolean tcam_property_boolean_get_value( TcamPropertyBoolean* self, GError** err )
{
    g_return_val_if_fail( self != NULL, FALSE );
    g_return_val_if_fail( err == NULL || *err == NULL, FALSE );
    g_return_val_if_fail( TCAM_IS_PROPERTY_BOOLEAN( self ), FALSE );

    TcamPropertyBooleanInterface* iface = TCAM_PROPERTY_BOOLEAN_GET_IFACE( self );
    if( iface->get_value )
    {
        return iface->get_value( self, err );
    }
    return FALSE;
}

/**
 * tcam_property_boolean_set_value:
 * @self: A #TcamPropertyBoolean
 * @value: The new value to set
 * @err: return location for a GError, or NULL
 */
void tcam_property_boolean_set_value( TcamPropertyBoolean* self, gboolean value, GError** err )
{
    g_return_if_fail( self != NULL );
    g_return_if_fail( err == NULL || *err == NULL );
    g_return_if_fail( TCAM_IS_PROPERTY_BOOLEAN( self ) );

    TcamPropertyBooleanInterface* iface = TCAM_PROPERTY_BOOLEAN_GET_IFACE( self );
    if( iface->set_value )
    {
        iface->set_value( self, value, err );
    }
}

/**
 * tcam_property_boolean_get_default:
 * @self: A #TcamPropertyBoolean
 * @err: return location for a GError, or NULL
 * Returns: The default-value for this property.
 */
gboolean tcam_property_boolean_get_default( TcamPropertyBoolean* self, GError** err )
{
    g_return_val_if_fail( self != NULL, FALSE );
    g_return_val_if_fail( err == NULL || *err == NULL, FALSE );
    g_return_val_if_fail( TCAM_IS_PROPERTY_BOOLEAN( self ), FALSE );

    TcamPropertyBooleanInterface* iface = TCAM_PROPERTY_BOOLEAN_GET_IFACE( self );
    if( iface->get_default )
    {
        return iface->get_default( self, err );
    }
    return FALSE;
}

G_DEFINE_INTERFACE( TcamPropertyInteger, tcam_property_integer, TCAM_TYPE_PROPERTY_BASE )

static void tcam_property_integer_default_init( __attribute__ ((unused)) TcamPropertyIntegerInterface* instance )
{}

/**
 * tcam_property_integer_get_value:
 * @self: A #TcamPropertyInteger
 * @err: return location for a GError, or NULL
 *
 * Returns: The current value of the property.
 */
gint64 tcam_property_integer_get_value( TcamPropertyInteger* self, GError** err )
{
    g_return_val_if_fail( self != NULL, 0 );
    g_return_val_if_fail( err == NULL || *err == NULL, 0 );
    g_return_val_if_fail( TCAM_IS_PROPERTY_INTEGER( self ), 0 );

    TcamPropertyIntegerInterface* iface = TCAM_PROPERTY_INTEGER_GET_IFACE( self );
    if( iface->get_value )
    {
        return iface->get_value( self, err );
    }
    return 0;
}

/**
 * tcam_property_integer_set_value:
 * @self: A #TcamPropertyInteger
 * @value: The new value to set
 * @err: return location for a GError, or NULL
 */
void tcam_property_integer_set_value( TcamPropertyInteger* self, gint64 value, GError** err )
{
    g_return_if_fail( self != NULL );
    g_return_if_fail( err == NULL || *err == NULL );
    g_return_if_fail( TCAM_IS_PROPERTY_INTEGER( self ) );

    TcamPropertyIntegerInterface* iface = TCAM_PROPERTY_INTEGER_GET_IFACE( self );
    if( iface->set_value )
    {
        iface->set_value( self, value, err );
    }
}

/**
 * tcam_property_integer_get_range:
 * @self: A #TcamPropertyInteger
 * @min_value: (out) (optional): The minimum for this property.
 * @max_value: (out) (optional): The maximum for this property.
 * @step_value: (out) (optional): The step delta for this property.
 * @err: return location for a GError, or NULL
 */
void tcam_property_integer_get_range( TcamPropertyInteger* self, gint64* min_value, gint64* max_value, gint64* step_value, GError** err )
{
    g_return_if_fail( self != NULL );
    g_return_if_fail( err == NULL || *err == NULL );
    g_return_if_fail( TCAM_IS_PROPERTY_INTEGER( self ) );

    TcamPropertyIntegerInterface* iface = TCAM_PROPERTY_INTEGER_GET_IFACE( self );
    if( iface->get_range )
    {
        iface->get_range( self, min_value, max_value, step_value, err );
    }
}

/**
 * tcam_property_integer_get_default:
 * @self: A #TcamPropertyInteger
 * @err: return location for a GError, or NULL
 * Returns: The default-value for this property.
 */
gint64  tcam_property_integer_get_default( TcamPropertyInteger* self, GError** err )
{
    g_return_val_if_fail( self != NULL, 0 );
    g_return_val_if_fail( err == NULL || *err == NULL, 0 );
    g_return_val_if_fail( TCAM_IS_PROPERTY_INTEGER( self ), 0 );

    TcamPropertyIntegerInterface* iface = TCAM_PROPERTY_INTEGER_GET_IFACE( self );
    if( iface->get_default )
    {
        return iface->get_default( self, err );
    }
    return 0;
}

/**
 * tcam_property_integer_get_unit:
 * @self: A #TcamPropertyInteger
 * Returns: (transfer none) (nullable) (type utf8): The unit of this property or NULL if this property does not have a unit.
 */
const gchar* tcam_property_integer_get_unit( TcamPropertyInteger* self )
{
    g_return_val_if_fail( self != NULL, NULL );
    g_return_val_if_fail( TCAM_IS_PROPERTY_INTEGER( self ), NULL );

    TcamPropertyIntegerInterface* iface = TCAM_PROPERTY_INTEGER_GET_IFACE( self );
    if( iface->get_unit )
    {
        return iface->get_unit( self );
    }
    return NULL;
}

/**
 * tcam_property_integer_get_representation:
 * @self: A #TcamPropertyInteger
 * Returns: The #TcamPropertyIntRepresentation of this property.
 */
TcamPropertyIntRepresentation tcam_property_integer_get_representation( TcamPropertyInteger* self )
{
    g_return_val_if_fail( self != NULL, TCAM_PROPERTY_INTREPRESENTATION_LINEAR );
    g_return_val_if_fail( TCAM_IS_PROPERTY_INTEGER( self ), TCAM_PROPERTY_INTREPRESENTATION_LINEAR );

    TcamPropertyIntegerInterface* iface = TCAM_PROPERTY_INTEGER_GET_IFACE( self );
    if( iface->get_representation )
    {
        return iface->get_representation( self );
    }
    return TCAM_PROPERTY_INTREPRESENTATION_LINEAR;
}


G_DEFINE_INTERFACE( TcamPropertyFloat, tcam_property_float, TCAM_TYPE_PROPERTY_BASE )

static void tcam_property_float_default_init( __attribute__ ((unused)) TcamPropertyFloatInterface* instance )
{}

/**
 * tcam_property_float_get_value:
 * @self: A #TcamPropertyFloat
 * @err: return location for a GError, or NULL
 *
 * Returns: The current value of the property.
 */
gdouble tcam_property_float_get_value( TcamPropertyFloat* self, GError** err )
{
    g_return_val_if_fail( self != NULL, 0 );
    g_return_val_if_fail( err == NULL || *err == NULL, 0 );
    g_return_val_if_fail( TCAM_IS_PROPERTY_FLOAT( self ), 0 );

    TcamPropertyFloatInterface* iface = TCAM_PROPERTY_FLOAT_GET_IFACE( self );
    if( iface->get_value )
    {
        return iface->get_value( self, err );
    }
    return 0;
}

/**
 * tcam_property_float_set_value:
 * @self: A #TcamPropertyFloat
 * @value: The new value to set
 * @err: return location for a GError, or NULL
 */
void tcam_property_float_set_value( TcamPropertyFloat* self, gdouble value, GError** err )
{
    g_return_if_fail( self != NULL );
    g_return_if_fail( err == NULL || *err == NULL );
    g_return_if_fail( TCAM_IS_PROPERTY_FLOAT( self ) );

    TcamPropertyFloatInterface* iface = TCAM_PROPERTY_FLOAT_GET_IFACE( self );
    if( iface->set_value )
    {
        iface->set_value( self, value, err );
    }
}

/**
 * tcam_property_float_get_range:
 * @self: A #TcamPropertyFloat
 * @min_value: (out) (optional): The minimum for this property.
 * @max_value: (out) (optional): The maximum for this property.
 * @step_value: (out) (optional): The step delta for this property.
 * @err: return location for a GError, or NULL
 */
void tcam_property_float_get_range( TcamPropertyFloat* self, gdouble* min_value, gdouble* max_value, gdouble* step_value, GError** err )
{
    g_return_if_fail( self != NULL );
    g_return_if_fail( err == NULL || *err == NULL );
    g_return_if_fail( TCAM_IS_PROPERTY_FLOAT( self ) );

    TcamPropertyFloatInterface* iface = TCAM_PROPERTY_FLOAT_GET_IFACE( self );
    if( iface->get_range )
    {
        iface->get_range( self, min_value, max_value, step_value, err );
    }
}

/**
 * tcam_property_float_get_default:
 * @self: A #TcamPropertyFloat
 * @err: return location for a GError, or NULL
 * Returns: The default-value for this property.
 */
gdouble tcam_property_float_get_default( TcamPropertyFloat* self, GError** err )
{
    g_return_val_if_fail( self != NULL, 0 );
    g_return_val_if_fail( err == NULL || *err == NULL, 0 );
    g_return_val_if_fail( TCAM_IS_PROPERTY_FLOAT( self ), 0 );

    TcamPropertyFloatInterface* iface = TCAM_PROPERTY_FLOAT_GET_IFACE( self );
    if( iface->get_default )
    {
        return iface->get_default( self, err );
    }
    return 0;
}

/**
 * tcam_property_float_get_unit:
 * @self: A #TcamPropertyFloat
 * Returns: (transfer none) (nullable) (type utf8): The unit of this property or NULL if this property does not have a unit.
 */
const gchar* tcam_property_float_get_unit( TcamPropertyFloat* self )
{
    g_return_val_if_fail( self != NULL, NULL );
    g_return_val_if_fail( TCAM_IS_PROPERTY_FLOAT( self ), NULL );

    TcamPropertyFloatInterface* iface = TCAM_PROPERTY_FLOAT_GET_IFACE( self );
    if( iface->get_unit )
    {
        return iface->get_unit( self );
    }
    return NULL;
}

/**
 * tcam_property_float_get_representation:
 * @self: A #TcamPropertyFloat
 * Returns: The #TcamPropertyFloatRepresentation of this property.
 */
TcamPropertyFloatRepresentation tcam_property_float_get_representation( TcamPropertyFloat* self )
{
    g_return_val_if_fail( self != NULL, TCAM_PROPERTY_FLOATREPRESENTATION_LINEAR );
    g_return_val_if_fail( TCAM_IS_PROPERTY_FLOAT( self ), TCAM_PROPERTY_FLOATREPRESENTATION_LINEAR );

    TcamPropertyFloatInterface* iface = TCAM_PROPERTY_FLOAT_GET_IFACE( self );
    if( iface->get_representation )
    {
        return iface->get_representation( self );
    }
    return TCAM_PROPERTY_FLOATREPRESENTATION_LINEAR;
}

G_DEFINE_INTERFACE( TcamPropertyEnumeration, tcam_property_enumeration, TCAM_TYPE_PROPERTY_BASE )

static void tcam_property_enumeration_default_init( __attribute__ ((unused)) TcamPropertyEnumerationInterface* instance )
{}

/**
 * tcam_property_enumeration_get_value:
 * @self: A #TcamPropertyEnumeration
 * @err: return location for a GError, or NULL
 * Returns: (transfer none)(type utf8): The current value of the property
 */
const gchar* tcam_property_enumeration_get_value( TcamPropertyEnumeration* self, GError** err )
{
    g_return_val_if_fail( self != NULL, NULL );
    g_return_val_if_fail( err == NULL || *err == NULL, NULL );
    g_return_val_if_fail( TCAM_IS_PROPERTY_ENUMERATION( self ), NULL );

    TcamPropertyEnumerationInterface* iface = TCAM_PROPERTY_ENUMERATION_GET_IFACE( self );
    if( iface->get_value )
    {
        return iface->get_value( self, err );
    }
    return NULL;
}

/**
 * tcam_property_enumeration_set_value:
 * @self: A #TcamPropertyEnumeration
 * @value: (in) (not nullable): The new value to set
 * @err: return location for a GError, or NULL
 */
void tcam_property_enumeration_set_value( TcamPropertyEnumeration* self, const gchar* value, GError** err )
{
    g_return_if_fail( self != NULL );
    g_return_if_fail( err == NULL || *err == NULL );
    g_return_if_fail( TCAM_IS_PROPERTY_ENUMERATION( self ) );

    TcamPropertyEnumerationInterface* iface = TCAM_PROPERTY_ENUMERATION_GET_IFACE( self );
    if( iface->set_value )
    {
        iface->set_value( self, value, err );
    }
}

/**
 * tcam_property_enumeration_get_enum_entries:
 * @self: A #TcamPropertyEnumeration
 * @err: return location for a GError, or NULL
 * Returns: (transfer full)(element-type utf8): The list of enum entries.
 */
GSList* tcam_property_enumeration_get_enum_entries( TcamPropertyEnumeration* self, GError** err )
{
    g_return_val_if_fail( self != NULL, NULL );
    g_return_val_if_fail( err == NULL || *err == NULL, NULL );
    g_return_val_if_fail( TCAM_IS_PROPERTY_ENUMERATION( self ), NULL );

    TcamPropertyEnumerationInterface* iface = TCAM_PROPERTY_ENUMERATION_GET_IFACE( self );
    if( iface->get_enum_entries )
    {
        return iface->get_enum_entries( self, err );
    }
    return NULL;
}

/**
 * tcam_property_enumeration_get_default:
 * @self: A #TcamPropertyEnumeration
 * @err: return location for a GError, or NULL
 * Returns: (transfer none)(type utf8): The default value of the property
 */
const gchar* tcam_property_enumeration_get_default( TcamPropertyEnumeration* self, GError** err )
{
    g_return_val_if_fail( self != NULL, NULL );
    g_return_val_if_fail( err == NULL || *err == NULL, NULL );
    g_return_val_if_fail( TCAM_IS_PROPERTY_ENUMERATION( self ), NULL );

    TcamPropertyEnumerationInterface* iface = TCAM_PROPERTY_ENUMERATION_GET_IFACE( self );
    if( iface->get_default )
    {
        return iface->get_default( self, err );
    }
    return NULL;
}

G_DEFINE_INTERFACE( TcamPropertyCommand, tcam_property_command, TCAM_TYPE_PROPERTY_BASE )

static void tcam_property_command_default_init( __attribute__ ((unused)) TcamPropertyCommandInterface* instance )
{}

/**
 * tcam_property_command_set_command:
 * @self: A #TcamPropertyCommand
 * @err: return location for a GError, or NULL
 */
void tcam_property_command_set_command( TcamPropertyCommand* self, GError** err )
{
    g_return_if_fail( self != NULL );
    g_return_if_fail( err == NULL || *err == NULL );
    g_return_if_fail( TCAM_IS_PROPERTY_COMMAND( self ) );

    TcamPropertyCommandInterface* iface = TCAM_PROPERTY_COMMAND_GET_IFACE( self );
    if( iface->set_command )
    {
        iface->set_command( self, err );
    }
}


G_DEFINE_INTERFACE(TcamPropertyString, tcam_property_string, TCAM_TYPE_PROPERTY_BASE)

static void tcam_property_string_default_init(__attribute__((unused))
                                               TcamPropertyStringInterface* instance)
{
}

/**
 * tcam_property_string_get_value:
 * @self: A #TcamPropertyString
 * @err: return location for a GError, or NULL
 *
 * Returns: (transfer full)(type utf8): The current value of the property
 */
char* tcam_property_string_get_value(TcamPropertyString* self, GError** err)
{
    g_return_val_if_fail(self != NULL, NULL);
    g_return_val_if_fail(err == NULL || *err == NULL, NULL);
    g_return_val_if_fail(TCAM_IS_PROPERTY_STRING(self), NULL);

    TcamPropertyStringInterface* iface = TCAM_PROPERTY_STRING_GET_IFACE(self);
    if (iface->get_value)
    {
        return iface->get_value(self, err);
    }
    return NULL;
}

/**
 * tcam_property_string_set_value:
 * @self: A #TcamPropertyString
 * @value: The new value to set
 * @err: return location for a GError, or NULL
 */
void tcam_property_string_set_value(TcamPropertyString* self, const char* value, GError** err)
{
    g_return_if_fail(self != NULL);
    g_return_if_fail(err == NULL || *err == NULL);
    g_return_if_fail(TCAM_IS_PROPERTY_STRING(self));

    TcamPropertyStringInterface* iface = TCAM_PROPERTY_STRING_GET_IFACE(self);
    if (iface->set_value)
    {
        iface->set_value(self, value, err);
    }
}

G_DEFINE_INTERFACE( TcamPropertyProvider, tcam_property_provider, G_TYPE_OBJECT )

static void tcam_property_provider_default_init( __attribute__( (unused) )TcamPropertyProviderInterface* klass )
{}

/**
 * tcam_property_provider_get_tcam_property_names:
 * @self: a #TcamPropertyProvider
 * @err: return location for a GError, or NULL
 *
 * Returns: (transfer full) (element-type utf8): Return a list of the available property names
 */
GSList* tcam_property_provider_get_tcam_property_names( TcamPropertyProvider* self, GError** err )
{
    g_return_val_if_fail( self != NULL, NULL );
    g_return_val_if_fail( err == NULL || *err == NULL, NULL );
    g_return_val_if_fail( TCAM_IS_PROPERTY_PROVIDER( self ), NULL );

    TcamPropertyProviderInterface* iface = TCAM_PROPERTY_PROVIDER_GET_IFACE( self );
    if( iface->get_tcam_property_names )
    {
        return iface->get_tcam_property_names( self, err );
    }
    return NULL;
}

/**
 * tcam_property_provider_get_tcam_property:
 * @self: a #TcamPropertyProvider
 * @name: (not nullable): name of the property to find
 * @err: return location for a GError, or NULL
 *
 * Returns: (transfer full): Returns  a #GOject implementing #TcamPropertyBase representing the property or NULL on error
 */
TcamPropertyBase* tcam_property_provider_get_tcam_property( TcamPropertyProvider* self, const gchar* name, GError** err )
{
    g_return_val_if_fail( self != NULL, NULL );
    g_return_val_if_fail( name != NULL, NULL );
    g_return_val_if_fail( err == NULL || *err == NULL, NULL );
    g_return_val_if_fail( TCAM_IS_PROPERTY_PROVIDER( self ), NULL );

    TcamPropertyProviderInterface* iface = TCAM_PROPERTY_PROVIDER_GET_IFACE( self );
    if( iface->get_tcam_property )
    {
        return iface->get_tcam_property( self, name, err );
    }
    return NULL;
}

/**
 * tcam_property_provider_set_tcam_boolean:
 * @self: a #TcamPropertyProvider
 * @name: (not nullable): name of the property on which the value should be set
 * @value: New value for the property
 * @err: return location for a GError, or NULL
 */
void        tcam_property_provider_set_tcam_boolean( TcamPropertyProvider* self, const gchar* name, gboolean value, GError** err )
{
    g_return_if_fail( self != NULL );
    g_return_if_fail( name != NULL );
    g_return_if_fail( err == NULL || *err == NULL );
    g_return_if_fail( TCAM_IS_PROPERTY_PROVIDER( self ) );

    TcamPropertyProviderInterface* iface = TCAM_PROPERTY_PROVIDER_GET_IFACE( self );
    if( iface->set_tcam_boolean )
    {
        iface->set_tcam_boolean( self, name, value, err );
    }
}

/**
 * tcam_property_provider_set_tcam_integer:
 * @self: a #TcamPropertyProvider
 * @name: (not nullable): name of the property on which the value should be set
 * @value: New value for the property
 * @err: return location for a GError, or NULL
 */
void        tcam_property_provider_set_tcam_integer( TcamPropertyProvider* self, const gchar* name, gint64 value, GError** err )
{
    g_return_if_fail( self != NULL );
    g_return_if_fail( name != NULL );
    g_return_if_fail( err == NULL || *err == NULL );
    g_return_if_fail( TCAM_IS_PROPERTY_PROVIDER( self ) );

    TcamPropertyProviderInterface* iface = TCAM_PROPERTY_PROVIDER_GET_IFACE( self );
    if( iface->set_tcam_integer )
    {
        iface->set_tcam_integer( self, name, value, err );
    }
}

/**
 * tcam_property_provider_set_tcam_float:
 * @self: a #TcamPropertyProvider
 * @name: (not nullable): name of the property on which the value should be set
 * @value: New value for the property
 * @err: return location for a GError, or NULL
 */
void        tcam_property_provider_set_tcam_float( TcamPropertyProvider* self, const gchar* name, gdouble value, GError** err )
{
    g_return_if_fail( self != NULL );
    g_return_if_fail( name != NULL );
    g_return_if_fail( err == NULL || *err == NULL );
    g_return_if_fail( TCAM_IS_PROPERTY_PROVIDER( self ) );

    TcamPropertyProviderInterface* iface = TCAM_PROPERTY_PROVIDER_GET_IFACE( self );
    if( iface->set_tcam_float )
    {
        iface->set_tcam_float( self, name, value, err );
    }
}

/**
 * tcam_property_provider_set_tcam_enumeration:
 * @self: a #TcamPropertyProvider
 * @name: (not nullable): name of the property on which the value should be set
 * @value: (not nullable): New value for the property
 * @err: return location for a GError, or NULL
 */
void        tcam_property_provider_set_tcam_enumeration( TcamPropertyProvider* self, const gchar* name, const gchar* value, GError** err )
{
    g_return_if_fail( self != NULL );
    g_return_if_fail( name != NULL );
    g_return_if_fail( err == NULL || *err == NULL );
    g_return_if_fail( TCAM_IS_PROPERTY_PROVIDER( self ) );

    TcamPropertyProviderInterface* iface = TCAM_PROPERTY_PROVIDER_GET_IFACE( self );
    if( iface->set_tcam_enumeration )
    {
        iface->set_tcam_enumeration( self, name, value, err );
    }
}

/**
 * tcam_property_provider_set_tcam_command:
 * @self: a #TcamPropertyProvider
 * @name: (not nullable): name of the property on where set_command should be called
 * @err: return location for a GError, or NULL
 */
void        tcam_property_provider_set_tcam_command( TcamPropertyProvider* self, const gchar* name, GError** err )
{
    g_return_if_fail( self != NULL );
    g_return_if_fail( name != NULL );
    g_return_if_fail( err == NULL || *err == NULL );
    g_return_if_fail( TCAM_IS_PROPERTY_PROVIDER( self ) );

    TcamPropertyProviderInterface* iface = TCAM_PROPERTY_PROVIDER_GET_IFACE( self );
    if( iface->set_tcam_command )
    {
        iface->set_tcam_command( self, name, err );
    }
}

/**
 * tcam_property_provider_get_tcam_boolean:
 * @self: a #TcamPropertyProvider
 * @name: (not nullable): name of the property whose value will be returned.
 * @err: return location for a GError, or NULL
 *
 * Returns: Returns the value of the property.
 */
gboolean    tcam_property_provider_get_tcam_boolean( TcamPropertyProvider* self, const gchar* name, GError** err )
{
    gboolean rval = FALSE;
    g_return_val_if_fail( self != NULL, rval );
    g_return_val_if_fail( name != NULL, rval );
    g_return_val_if_fail( err == NULL || *err == NULL, rval );
    g_return_val_if_fail( TCAM_IS_PROPERTY_PROVIDER( self ), rval );

    TcamPropertyProviderInterface* iface = TCAM_PROPERTY_PROVIDER_GET_IFACE( self );
    if( iface->get_tcam_boolean )
    {
        rval = iface->get_tcam_boolean( self, name, err );
    }
    return rval;
}

/**
 * tcam_property_provider_get_tcam_integer:
 * @self: a #TcamPropertyProvider
 * @name: (not nullable): name of the property whose value will be returned.
 * @err: return location for a GError, or NULL
 *
 * Returns: Returns the value of the property.
 */
gint64      tcam_property_provider_get_tcam_integer( TcamPropertyProvider* self, const gchar* name, GError** err )
{
    gint64 rval = 0;
    g_return_val_if_fail( self != NULL, rval );
    g_return_val_if_fail( name != NULL, rval );
    g_return_val_if_fail( err == NULL || *err == NULL, rval );
    g_return_val_if_fail( TCAM_IS_PROPERTY_PROVIDER( self ), rval );

    TcamPropertyProviderInterface* iface = TCAM_PROPERTY_PROVIDER_GET_IFACE( self );
    if( iface->get_tcam_integer )
    {
        rval = iface->get_tcam_integer( self, name, err );
    }
    return rval;
}

/**
 * tcam_property_provider_get_tcam_float:
 * @self: a #TcamPropertyProvider
 * @name: (not nullable): name of the property whose value will be returned.
 * @err: return location for a GError, or NULL
 *
 * Returns: Returns the value of the property.
 */
gdouble     tcam_property_provider_get_tcam_float( TcamPropertyProvider* self, const gchar* name, GError** err )
{
    gdouble rval = FALSE;
    g_return_val_if_fail( self != NULL, rval );
    g_return_val_if_fail( name != NULL, rval );
    g_return_val_if_fail( err == NULL || *err == NULL, 0 );
    g_return_val_if_fail( TCAM_IS_PROPERTY_PROVIDER( self ), rval );

    TcamPropertyProviderInterface* iface = TCAM_PROPERTY_PROVIDER_GET_IFACE( self );
    if( iface->get_tcam_float )
    {
        rval = iface->get_tcam_float( self, name, err );
    }
    return rval;
}

/**
 * tcam_property_provider_get_tcam_enumeration:
 * @self: a #TcamPropertyProvider
 * @name: (not nullable): name of the property whose value will be returned.
 * @err: return location for a GError, or NULL
 *
 * Returns: (transfer none)(type utf8): The current value of the property
 */
const gchar* tcam_property_provider_get_tcam_enumeration( TcamPropertyProvider* self, const gchar* name, GError** err )
{
    const gchar* rval = FALSE;
    g_return_val_if_fail( self != NULL, rval );
    g_return_val_if_fail( name != NULL, rval );
    g_return_val_if_fail( err == NULL || *err == NULL, rval );
    g_return_val_if_fail( TCAM_IS_PROPERTY_PROVIDER( self ), rval );

    TcamPropertyProviderInterface* iface = TCAM_PROPERTY_PROVIDER_GET_IFACE( self );
    if( iface->get_tcam_enumeration )
    {
        rval = iface->get_tcam_enumeration( self, name, err );
    }
    return rval;
}