/*
 * Copyright 2020 The Imaging Source Europe GmbH
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

#include <glib-object.h>

G_BEGIN_DECLS

#define TCAM_TYPE_PROP tcam_prop_get_type()

#ifdef G_DECLARE_INTERFACE

G_DECLARE_INTERFACE(TcamProp, tcam_prop, TCAM, PROP, GObject)

#else

#define TCAM_PROP(obj)    (G_TYPE_CHECK_INSTANCE_CAST((obj), TCAM_TYPE_PROP, TcamProp))
#define TCAM_IS_PROP(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), TCAM_TYPE_PROP))
#define TCAM_PROP_GET_IFACE(inst) \
    (G_TYPE_INSTANCE_GET_INTERFACE((inst), TCAM_TYPE_PROP, TcamPropInterface))

GType tcam_prop_get_type(void);

typedef struct _TcamProp TcamProp; /* dummy object */
typedef struct _TcamPropInterface TcamPropInterface;

#endif


typedef enum {

    TCAM_ERROR_UNKNOWN = 1 << 0,
    TCAM_ERROR_DEVICE_LOST = 1 << 1,
    TCAM_ERROR_NO_SUCH_PROPERTY = 1 << 2,
    TCAM_ERROR_PROPERTY_VALUE = 1 << 3,
    TCAM_ERROR_PROPERTY_NOT_SETTABLE = 1 << 4,
    TCAM_ERROR_WRONG_TYPE = 1 << 5,

} TcamError;



typedef enum {
    TCAM_READ_ONLY  = 1 << 0,
    TCAM_WRITE_ONLY   = 1 << 1,
    TCAM_LOCKED  = 1 << 2,
    TCAM_EXTERNAL  = 1 << 3
} TcamPropertyFlags;


typedef enum {
    TCAM_PROPERTY_UNKNOWN = 1 << 0,
    TCAM_PROPERTY_INT     = 1 << 1,
    TCAM_PROPERTY_DOUBLE  = 1 << 2,
    TCAM_PROPERTY_ENUM    = 1 << 3,
    TCAM_PROPERTY_BOOL    = 1 << 4,
    TCAM_PROPERTY_BUTTON  = 1 << 5,
} TcamPropertyType;



typedef enum {
    TCAM_VISIBILITY_BEGINNER  = 1 << 0,
    TCAM_VISIBILITY_EXPERT    = 1 << 1,
    TCAM_VISIBILITY_GURU      = 1 << 2,
    TCAM_VISIBILITY_INVISIBLE = 1 << 3,
} TcamPropertyVisibility;


// static property parts

#define TCAM_PROPERTY_INFO (tcam_property_info_get_type())
GType tcam_property_info_get_type (void);

typedef struct _TcamPropertyInformation TcamPropertyInfo;

struct _TcamPropertyInformation {

    const gchar* name; // used to ensure information is associatable
    TcamPropertyType type;
    const gchar* display_name;
    const gchar* description;
    const gchar* unit;
    const gchar* category;
    TcamPropertyVisibility visibility;

    // TODO  add placeholder for expansions
};


struct _TcamPropInterface
{
    GTypeInterface parent_interface;

    // query list of all available properties
    // from v1
    GSList* (*get_tcam_property_names)(TcamProp* self, GError** err);

    TcamPropertyInfo* (*get_tcam_info)(TcamProp* self, const char* name, GError** err);

    // query available menu entries for property
    // from v1
    GSList* (*get_tcam_enum_entries)(TcamProp* self,
                                     const char* menu_name,
                                     GError** err);



    TcamPropertyType (*get_tcam_type)(TcamProp* self,
                                      const char* name,
                                      GError** err);

    gboolean (*get_tcam_range)(TcamProp* self, const char* name, GValue* min, GValue* max, GValue* step, GError** err);

    gboolean (*get_tcam_default)(TcamProp* self, const char* name, GValue* default_value, GError** err);

    //
    // getter
    //

    gboolean (*get_tcam_property)(TcamProp* self, const gchar* name,
                                  GValue* value, GValue* flags, GError** err);

    const char* (*get_tcam_string)(TcamProp* self,
                                   const char* name,
                                   GError** err);

    gint64 (*get_tcam_int)(TcamProp* self,
                           const char* name,
                           GError** err);

    gdouble (*get_tcam_double)(TcamProp* self,
                               const char* name,
                               GError** err);

    gboolean (*get_tcam_bool)(TcamProp* self,
                              const char* name,
                              GError** err);

    //
    // setter
    //

    gboolean (*set_tcam_property)(TcamProp* self, const gchar* name, const GValue* value, GError** err);

    gboolean (*set_tcam_string)(TcamProp* self,
                                const char* name,
                                const char* value,
                                GError** err);

    gboolean (*set_tcam_int)(TcamProp* self,
                             const char* name,
                             gint64 value,
                             GError** err);

    gboolean (*set_tcam_double)(TcamProp* self,
                                const char* name,
                                gdouble value,
                                GError** err);

    gboolean (*set_tcam_bool)(TcamProp* self,
                              const char* name,
                              gboolean value,
                              GError** err);

    gboolean (*set_tcam_execute)(TcamProp* self,
                                 const char* name,
                                 GError** err);


};

G_END_DECLS
