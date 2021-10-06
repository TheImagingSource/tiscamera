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

#ifndef TCAMPROP_H
#define TCAMPROP_H

#include "Tcam-1.0.h"

G_BEGIN_DECLS

// error stuff

#define TCAM_ERROR tcam_error_quark()

GQuark tcam_error_quark(void);


GType tcam_error_get_type(void);

// helper types

GType tcam_property_flags_get_type (void);

GType tcam_property_visibility_get_type(void);

GType tcam_property_type_get_type (void);

// static data description

TcamPropertyInfo* tcam_property_info_new (const gchar* name,
                                          TcamPropertyType type,
                                          const gchar* displayname,
                                          const gchar* description,
                                          const gchar* unit,
                                          const gchar* category,
                                          TcamPropertyVisibility visibility);

/**
 * tcam_property_info_copy: (skip)
 *
 * Creates a copy of @data.
 */

TcamPropertyInfo* tcam_property_info_copy (TcamPropertyInfo* data);

/**
 * tcam_property_description_free: (skip)
 *
 * Free's @data.
 */
void tcam_property_info_free (TcamPropertyInfo* data);


// actual interface

GSList* tcam_prop_get_tcam_property_names(TcamProp* self, GError** err);

TcamPropertyInfo* tcam_prop_get_property_info(TcamProp* self, const char* name, GError** err);

GSList* tcam_prop_get_tcam_enum_entries(TcamProp* self, const char* name, GError** err);


TcamPropertyType tcam_prop_get_tcam_property_type(TcamProp* self, const gchar* name, GError** err);


gboolean tcam_prop_get_tcam_range(TcamProp* self, const char* name, GValue* min, GValue* max, GValue* step, GError** err);

gboolean tcam_prop_get_tcam_default(TcamProp* self, const char* name, GValue* default_value, GError** err);

//
// getter
//

gboolean tcam_prop_get_tcam_property(TcamProp* self, const gchar* name,
                              GValue* value, GValue* flags, GError** err);

const char* tcam_prop_get_tcam_string(TcamProp* self,
                               const char* name,
                               GError** err);

gint64 tcam_prop_get_tcam_int(TcamProp* self,
                       const char* name,
                       GError** err);

gdouble tcam_prop_get_tcam_double(TcamProp* self,
                           const char* name,
                           GError** err);

gboolean tcam_prop_get_tcam_bool(TcamProp* self,
                          const char* name,
                          GError** err);

//
// setter
//

gboolean tcam_prop_set_tcam_property(TcamProp* self, const gchar* name, const GValue* value, GError** err);

gboolean tcam_prop_set_tcam_string(TcamProp* self,
                                   const char* name,
                                   const char* value,
                                   GError** err);

gboolean tcam_prop_set_tcam_int(TcamProp* self,
                                const char* name,
                                gint64 value,
                                GError** err);

gboolean tcam_prop_set_tcam_double(TcamProp* self,
                                   const char* name,
                                   gdouble value,
                                   GError** err);

gboolean tcam_prop_set_tcam_bool(TcamProp* self,
                                 const char* name,
                                 gboolean value,
                                 GError** err);

gboolean tcam_prop_set_tcam_execute(TcamProp* self,
                                    const char* name,
                                    GError** err);



G_END_DECLS

#endif /* TCAMPROP_H */
