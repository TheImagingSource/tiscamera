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

#pragma once

#include "../../gobject/tcamprop.h"
#include "gsttcammainsrc.h"


gboolean get_property_by_name(GstTcamMainSrc* self,
                              const gchar* name,
                              struct tcam_device_property* prop);


gchar* gst_tcam_mainsrc_get_property_type(TcamProp* iface, const gchar* name);


GSList* gst_tcam_mainsrc_get_property_names(TcamProp* iface);


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
                                            GValue* group);


GSList* gst_tcam_mainsrc_get_menu_entries(TcamProp* iface, const char* menu_name);


gboolean gst_tcam_mainsrc_set_tcam_property(TcamProp* iface,
                                            const gchar* name,
                                            const GValue* value);


GSList* gst_tcam_mainsrc_get_device_serials(TcamProp* self);


GSList* gst_tcam_mainsrc_get_device_serials_backend(TcamProp* self);

gboolean gst_tcam_mainsrc_get_device_info(TcamProp* self,
                                          const char* serial,
                                          char** name,
                                          char** identifier,
                                          char** connection_type);
