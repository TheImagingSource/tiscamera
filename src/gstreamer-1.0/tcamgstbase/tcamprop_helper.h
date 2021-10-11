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

#include "tcam-property-1.0.h"


void tcamprop_helper_set_tcam_boolean( TcamPropertyProvider* self, const gchar* name, gboolean value, GError** err );


void tcamprop_helper_set_tcam_integer( TcamPropertyProvider* self, const gchar* name, gint64 value, GError** err );


void tcamprop_helper_set_tcam_float( TcamPropertyProvider* self, const gchar* name, gdouble value, GError** err );


void tcamprop_helper_set_tcam_enumeration( TcamPropertyProvider* self, const gchar* name, const gchar* value, GError** err );


void tcamprop_helper_set_tcam_command( TcamPropertyProvider* self, const gchar* name, GError** err );


gboolean tcamprop_helper_get_tcam_boolean( TcamPropertyProvider* self, const gchar* name, GError** err );


gint64 tcamprop_helper_get_tcam_integer( TcamPropertyProvider* self, const gchar* name, GError** err );


gdouble tcamprop_helper_get_tcam_float( TcamPropertyProvider* self, const gchar* name, GError** err );


gchar* tcamprop_helper_get_tcam_enumeration( TcamPropertyProvider* self, const gchar* name, GError** err );
