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

#include "Tcam-0.1.h"

G_BEGIN_DECLS

GSList* tcam_prop_get_tcam_property_names (TcamProp* self);

const gchar *tcam_prop_get_tcam_property_type (TcamProp* self,
                                               const gchar* name);

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
                                      GValue* group);

GSList* tcam_prop_get_tcam_menu_entries (TcamProp* self,
                                         const char* name);

gboolean tcam_prop_set_tcam_property (TcamProp* self,
                                      const gchar* name,
                                      const GValue* value);

GSList* tcam_prop_get_device_serials (TcamProp* self);

GSList* tcam_prop_get_device_serials_backend (TcamProp* self);

gboolean tcam_prop_get_device_info (TcamProp* self,
                                    const char* serial,
                                    char** name,
                                    char** identifier,
                                    char** connection_type);

G_END_DECLS

#endif /* TCAMPROP_H */
