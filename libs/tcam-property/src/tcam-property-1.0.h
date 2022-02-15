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

#ifndef TCAMPROP_1_0_IMPL_H
#define TCAMPROP_1_0_IMPL_H

#include "Tcam-1.0.h"

G_BEGIN_DECLS

GType   tcam_error_get_type(void);
GQuark  tcam_error_quark(void);

GType   tcam_property_visibility_get_type( void );
GType   tcam_property_intrepresentation_get_type( void );
GType   tcam_property_floatrepresentation_get_type( void );
GType   tcam_property_type_get_type( void );
GType	tcam_property_access_get_type(void);

const gchar*            tcam_property_base_get_name( TcamPropertyBase* self );
TcamPropertyType        tcam_property_base_get_property_type( TcamPropertyBase* self );
const gchar*            tcam_property_base_get_display_name( TcamPropertyBase* self );
const gchar*            tcam_property_base_get_description( TcamPropertyBase* self );
const gchar*            tcam_property_base_get_category( TcamPropertyBase* self );
TcamPropertyVisibility  tcam_property_base_get_visibility( TcamPropertyBase* self );
TcamPropertyAccess		tcam_property_base_get_access(TcamPropertyBase* self);
gboolean                tcam_property_base_is_available( TcamPropertyBase* self, GError** err );
gboolean                tcam_property_base_is_locked( TcamPropertyBase* self, GError** err );

gboolean                        tcam_property_boolean_get_value( TcamPropertyBoolean* self, GError** err );
void                            tcam_property_boolean_set_value( TcamPropertyBoolean* self, gboolean value, GError** err );
gboolean                        tcam_property_boolean_get_default( TcamPropertyBoolean* self, GError** err );

gint64                          tcam_property_integer_get_value( TcamPropertyInteger* self, GError** err );
void                            tcam_property_integer_set_value( TcamPropertyInteger* self, gint64 value, GError** err );
void                            tcam_property_integer_get_range( TcamPropertyInteger* self, gint64* min_value, gint64* max_value, gint64* step_value, GError** err );
gint64                          tcam_property_integer_get_default( TcamPropertyInteger* self, GError** err );
const gchar*                    tcam_property_integer_get_unit( TcamPropertyInteger* self );
TcamPropertyIntRepresentation   tcam_property_integer_get_representation( TcamPropertyInteger* self );

gdouble                         tcam_property_float_get_value( TcamPropertyFloat* self, GError** err );
void                            tcam_property_float_set_value( TcamPropertyFloat* self, gdouble value, GError** err );
void                            tcam_property_float_get_range( TcamPropertyFloat* self, gdouble* min_value, gdouble* max_value, gdouble* step_value, GError** err );
gdouble                         tcam_property_float_get_default( TcamPropertyFloat* self,GError** err );
const gchar*                    tcam_property_float_get_unit( TcamPropertyFloat* self );
TcamPropertyFloatRepresentation tcam_property_float_get_representation( TcamPropertyFloat* self );

const gchar*    tcam_property_enumeration_get_value( TcamPropertyEnumeration* self, GError** err );
void            tcam_property_enumeration_set_value( TcamPropertyEnumeration* self, const gchar* value, GError** err );
GSList*         tcam_property_enumeration_get_enum_entries( TcamPropertyEnumeration* self, GError** err );
const gchar*    tcam_property_enumeration_get_default( TcamPropertyEnumeration* self, GError** err );

void            tcam_property_command_set_command( TcamPropertyCommand* self, GError** err );

char*                       tcam_property_string_get_value( TcamPropertyString* self, GError** err );
void                        tcam_property_string_set_value( TcamPropertyString* self, const char* value, GError** err );

GSList*             tcam_property_provider_get_tcam_property_names( TcamPropertyProvider* self, GError** err );
TcamPropertyBase*   tcam_property_provider_get_tcam_property( TcamPropertyProvider* self, const gchar* name, GError** err );

void            tcam_property_provider_set_tcam_boolean( TcamPropertyProvider* self, const gchar* name, gboolean value, GError** err );
void            tcam_property_provider_set_tcam_integer( TcamPropertyProvider* self, const gchar* name, gint64 value, GError** err );
void            tcam_property_provider_set_tcam_float( TcamPropertyProvider* self, const gchar* name, gdouble value, GError** err );
void            tcam_property_provider_set_tcam_enumeration( TcamPropertyProvider* self, const gchar* name, const gchar* value, GError** err );
void            tcam_property_provider_set_tcam_command( TcamPropertyProvider* self, const gchar* name, GError** err );

gboolean        tcam_property_provider_get_tcam_boolean( TcamPropertyProvider* self, const gchar* name, GError** err );
gint64          tcam_property_provider_get_tcam_integer( TcamPropertyProvider* self, const gchar* name, GError** err );
gdouble         tcam_property_provider_get_tcam_float( TcamPropertyProvider* self, const gchar* name, GError** err );
const gchar*    tcam_property_provider_get_tcam_enumeration( TcamPropertyProvider* self, const gchar* name, GError** err );

G_END_DECLS

#endif /* TCAMPROP_1_0_IMPL_H */
