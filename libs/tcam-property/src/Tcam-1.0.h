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

#ifndef TCAMPROP_1_0_H
#define TCAMPROP_1_0_H

#pragma once

#include <glib-object.h>

G_BEGIN_DECLS

typedef enum {
    TCAM_ERROR_SUCCESS                      = 0,
    TCAM_ERROR_UNKNOWN                      = 1,
    TCAM_ERROR_TIMEOUT                      = 2,
    TCAM_ERROR_NOT_IMPLEMENTED              = 3,
    TCAM_ERROR_PARAMETER_INVALID            = 4,

    TCAM_ERROR_PROPERTY_NOT_IMPLEMENTED     = 10,
    TCAM_ERROR_PROPERTY_NOT_AVAILABLE       = 11,
    TCAM_ERROR_PROPERTY_NOT_WRITEABLE       = 12,
    TCAM_ERROR_PROPERTY_VALUE_OUT_OF_RANGE  = 13,
    TCAM_ERROR_PROPERTY_DEFAULT_NOT_AVAILABLE = 14,
    TCAM_ERROR_PROPERTY_TYPE_INCOMPATIBLE   = 15,
    
    TCAM_ERROR_DEVICE_NOT_OPENED            = 20,
    TCAM_ERROR_DEVICE_LOST                  = 21,
    TCAM_ERROR_DEVICE_NOT_ACCESSIBLE        = 22,
} TcamError;

typedef enum {
    TCAM_PROPERTY_TYPE_BOOLEAN      = 0,
    TCAM_PROPERTY_TYPE_INTEGER      = 1,
    TCAM_PROPERTY_TYPE_FLOAT        = 2,
    TCAM_PROPERTY_TYPE_ENUMERATION  = 3,
    TCAM_PROPERTY_TYPE_COMMAND      = 4,
    TCAM_PROPERTY_TYPE_STRING       = 5,
} TcamPropertyType;

typedef enum {
    TCAM_PROPERTY_VISIBILITY_BEGINNER = 0,
    TCAM_PROPERTY_VISIBILITY_EXPERT = 1,
    TCAM_PROPERTY_VISIBILITY_GURU = 2,
    TCAM_PROPERTY_VISIBILITY_INVISIBLE = 3,
} TcamPropertyVisibility;

typedef enum
{
    TCAM_PROPERTY_ACCESS_RW = 0,
    TCAM_PROPERTY_ACCESS_RO = 1,
    TCAM_PROPERTY_ACCESS_WO = 2,
} TcamPropertyAccess;

typedef enum  {
    TCAM_PROPERTY_INTREPRESENTATION_LINEAR = 0,
    TCAM_PROPERTY_INTREPRESENTATION_LOGARITHMIC = 1,
    TCAM_PROPERTY_INTREPRESENTATION_PURENUMBER = 2,
    TCAM_PROPERTY_INTREPRESENTATION_HEXNUMBER = 3,
} TcamPropertyIntRepresentation;

typedef enum {
    TCAM_PROPERTY_FLOATREPRESENTATION_LINEAR = 0,
    TCAM_PROPERTY_FLOATREPRESENTATION_LOGARITHMIC = 1,
    TCAM_PROPERTY_FLOATREPRESENTATION_PURENUMBER = 2,
} TcamPropertyFloatRepresentation;

#define TCAM_TYPE_PROPERTY_BASE tcam_property_base_get_type()
G_DECLARE_INTERFACE( TcamPropertyBase, tcam_property_base, TCAM, PROPERTY_BASE, GObject )

struct _TcamPropertyBaseInterface
{
    GTypeInterface parent_interface;

    const gchar* (*get_name)(TcamPropertyBase* self);
    const gchar* (*get_display_name)(TcamPropertyBase* self);
    const gchar* (*get_description)(TcamPropertyBase* self);
    const gchar* (*get_category)(TcamPropertyBase* self);

    TcamPropertyVisibility (*get_visibility)(TcamPropertyBase* self);

    TcamPropertyType (*get_property_type)(TcamPropertyBase* self);

    gboolean (*is_available)(TcamPropertyBase* self, GError** err );
    gboolean (*is_locked)(TcamPropertyBase* self, GError** err);

    TcamPropertyAccess (*get_access)(TcamPropertyBase* self);
    
    gpointer    padding[11];
};

#define TCAM_TYPE_PROPERTY_BOOLEAN tcam_property_boolean_get_type()
G_DECLARE_INTERFACE( TcamPropertyBoolean, tcam_property_boolean, TCAM, PROPERTY_BOOLEAN, TcamPropertyBase )

struct _TcamPropertyBooleanInterface
{
    GTypeInterface parent_interface;

    gboolean    (*get_value)(TcamPropertyBoolean* self, GError** err);
    void        (*set_value)(TcamPropertyBoolean* self, gboolean value, GError** err);

    gboolean    (*get_default)(TcamPropertyBoolean* self, GError** err);

    gpointer    padding[3];
};


#define TCAM_TYPE_PROPERTY_INTEGER tcam_property_integer_get_type()
G_DECLARE_INTERFACE( TcamPropertyInteger, tcam_property_integer, TCAM, PROPERTY_INTEGER, TcamPropertyBase )

struct _TcamPropertyIntegerInterface
{
    GTypeInterface parent_interface;

    gint64      (*get_value)(TcamPropertyInteger* self, GError** err);
    void        (*set_value)(TcamPropertyInteger* self, gint64 value, GError** err);

    void        (*get_range)(TcamPropertyInteger* self, gint64* min_value, gint64* max_value, gint64* step_value, GError** err);
    gint64      (*get_default)(TcamPropertyInteger* self, GError** err);


    const gchar* (*get_unit)(TcamPropertyInteger* self);
    TcamPropertyIntRepresentation   (*get_representation)(TcamPropertyInteger* self );

    gpointer    padding[3];
};


#define TCAM_TYPE_PROPERTY_FLOAT tcam_property_float_get_type()
G_DECLARE_INTERFACE( TcamPropertyFloat, tcam_property_float, TCAM, PROPERTY_FLOAT, TcamPropertyBase )

struct _TcamPropertyFloatInterface
{
    GTypeInterface parent_interface;

    gdouble     (*get_value)(TcamPropertyFloat* self, GError** err);
    void        (*set_value)(TcamPropertyFloat* self, gdouble value, GError** err);

    void        (*get_range)(TcamPropertyFloat* self, gdouble* min_value, gdouble* max_value, gdouble* step_value, GError** err);
    gdouble     (*get_default)(TcamPropertyFloat* self, GError** err);

    const gchar* (*get_unit)(TcamPropertyFloat* self);
    TcamPropertyFloatRepresentation( *get_representation )(TcamPropertyFloat* self);

    gpointer    padding[6];
};

#define TCAM_TYPE_PROPERTY_ENUMERATION tcam_property_enumeration_get_type()
G_DECLARE_INTERFACE( TcamPropertyEnumeration, tcam_property_enumeration, TCAM, PROPERTY_ENUMERATION, TcamPropertyBase )

struct _TcamPropertyEnumerationInterface
{
    GTypeInterface parent_interface;

    const gchar* (*get_value)(TcamPropertyEnumeration* self, GError** err);
    void        (*set_value)(TcamPropertyEnumeration* self, const gchar* value, GError** err);

    GSList*     (*get_enum_entries)(TcamPropertyEnumeration* self, GError** err );
    const gchar* (*get_default)(TcamPropertyEnumeration* self, GError** err);

    gpointer    padding[6];
};

#define TCAM_TYPE_PROPERTY_COMMAND tcam_property_command_get_type()
G_DECLARE_INTERFACE( TcamPropertyCommand, tcam_property_command, TCAM, PROPERTY_COMMAND, TcamPropertyBase )

struct _TcamPropertyCommandInterface
{
    GTypeInterface parent_interface;

    void        (*set_command)(TcamPropertyCommand* self, GError** err);

    gpointer    padding[3];
};

#define TCAM_TYPE_PROPERTY_STRING tcam_property_string_get_type()
G_DECLARE_INTERFACE( TcamPropertyString, tcam_property_string, TCAM, PROPERTY_STRING, TcamPropertyBase )

struct _TcamPropertyStringInterface
{
    GTypeInterface parent_interface;

    char*       (*get_value)(TcamPropertyString* self, GError** err);
    void        (*set_value)(TcamPropertyString* self, const char* value, GError** err);

    gpointer    padding[3];
};

#define TCAM_TYPE_PROPERTY_PROVIDER tcam_property_provider_get_type()
G_DECLARE_INTERFACE( TcamPropertyProvider, tcam_property_provider, TCAM, PROPERTY_PROVIDER, GObject )

struct _TcamPropertyProviderInterface
{
    GTypeInterface parent_interface;

    GSList*             (*get_tcam_property_names) (TcamPropertyProvider* self, GError** err);
    TcamPropertyBase*   (*get_tcam_property)(TcamPropertyProvider* self, const gchar* name, GError** err);

    void            (*set_tcam_boolean)(TcamPropertyProvider* self, const gchar* name, gboolean value, GError** err);
    void            (*set_tcam_integer)(TcamPropertyProvider* self, const gchar* name, gint64 value, GError** err);
    void            (*set_tcam_float)(TcamPropertyProvider* self, const gchar* name, gdouble value, GError** err);
    void            (*set_tcam_enumeration)(TcamPropertyProvider* self, const gchar* name, const gchar* value, GError** err);
    void            (*set_tcam_command)(TcamPropertyProvider* self, const gchar* name, GError** err);

    gboolean        (*get_tcam_boolean)(TcamPropertyProvider* self, const gchar* name, GError** err);
    gint64          (*get_tcam_integer)(TcamPropertyProvider* self, const gchar* name, GError** err);
    gdouble         (*get_tcam_float)(TcamPropertyProvider* self, const gchar* name, GError** err);
    const gchar*    (*get_tcam_enumeration)(TcamPropertyProvider* self, const gchar* name, GError** err);

    gpointer    padding[12];
};

G_END_DECLS

#endif
