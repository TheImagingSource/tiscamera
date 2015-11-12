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

#ifndef __TCAM_PROP_H__
#define __TCAM_PROP_H__

#include <glib-object.h>

G_BEGIN_DECLS

#define TCAM_TYPE_PROP tcam_prop_get_type()
G_DECLARE_INTERFACE (TcamProp, tcam_prop, TCAM, PROP, GObject)

struct _TcamPropInterface
{
	GTypeInterface parent_interface;

	GSList* (*get_property_names) (TcamProp *self);
	gchar* (*get_property_type) (TcamProp *self, gchar *name);
	gboolean (*get_property) (TcamProp *self, gchar *name,
				  GValue *value,
				  GValue *min,
				  GValue *max,
				  GValue *def,
				  GValue *step,
				  GValue *type);
	gboolean (*set_property) (TcamProp *self, gchar *name,
				  const GValue *value);
	GSList *(*get_device_serials) (TcamProp *self);
	gboolean (*get_device_info) (TcamProp *self,
				     const char *serial,
				     char **name,
				     char **identifier,
				     char **connection_type);
};

GSList* tcam_prop_get_tcam_property_names (TcamProp* self);
gchar *tcam_prop_get_tcam_property_type (TcamProp *self, gchar *name);
gboolean tcam_prop_get_tcam_property (TcamProp *self,
				      gchar *name,
				      GValue *value,
				      GValue *min,
				      GValue *max,
				      GValue *def,
				      GValue *step,
				      GValue *type);
gboolean tcam_prop_set_tcam_property (TcamProp *self,
				      gchar *name,
				      const GValue *value);
GSList *tcam_prop_get_device_serials (TcamProp *self);
gboolean tcam_prop_get_device_info (TcamProp *self,
				    const char *serial,
				    char **name,
				    char **identifier,
				    char **connection_type);

G_END_DECLS

#endif//__TCAM_PROP_H__
