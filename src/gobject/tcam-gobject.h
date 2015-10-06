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


#ifndef TCAM_GOBJECT_H
#define TCAM_GOBJECT_H

#include <glib-object.h>

G_BEGIN_DECLS

#define TCAM_TYPE_PROP tcam_prop_get_type()
G_DECLARE_FINAL_TYPE (TcamProp, tcam_prop, TCAM, PROP, GObject)

    TcamProp *tcam_prop_new (void);
    GSList *tcam_prop_test (TcamProp *self);
    GVariant *tcam_prop_enumerate (TcamProp *self);
    GSList *tcam_prop_get_property_names (TcamProp *self);
    gboolean tcam_prop_set (TcamProp *self, gchar *cname, GVariant *value);
    GVariant *tcam_prop_get (TcamProp *self, gchar *cname);
    gchar *tcam_prop_get_property_type (TcamProp *self, gchar *name);
    gdouble tcam_prop_get_property_double(TcamProp *self, gchar *name);
    gdouble tcam_prop_get_property_min(TcamProp *self, gchar *name);
    gdouble tcam_prop_get_property_max(TcamProp *self, gchar *name);
    gdouble tcam_prop_get_property_default_double(TcamProp *self,
                                                  gchar *name);
    gboolean tcam_prop_set_property_double(TcamProp *self,
                                           gchar *name,
                                           gdouble value);




    G_END_DECLS

#endif /* TCAM_GOBJECT_H */


/* /\* */
/*  * Copyright 2015 The Imaging Source Europe GmbH */
/*  * */
/*  * Licensed under the Apache License, Version 2.0 (the "License"); */
/*  * you may not use this file except in compliance with the License. */
/*  * You may obtain a copy of the License at */
/*  * */
/*  * http://www.apache.org/licenses/LICENSE-2.0 */
/*  * */
/*  * Unless required by applicable law or agreed to in writing, software */
/*  * distributed under the License is distributed on an "AS IS" BASIS, */
/*  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. */
/*  * See the License for the specific language governing permissions and */
/*  * limitations under the License. */
/*  *\/ */

/* #ifndef TCAM_GOBJECT_H */
/* #define TCAM_GOBJECT_H */

/* #include <glib-object.h> */

/* #ifdef __cplusplus */
/* extern "C" */
/* { */
/* #endif */


/* G_BEGIN_DECLS */

/* #define TCAM_TYPE tcam_get_type() */
/* G_DECLARE_FINAL_TYPE (Tcam, tcam, TCAM, TC, GObject) */

/* Tcam* tcam_init (void); */

/*     GVariant* tcam_get (Tcam* self, gchar* name); */
/*     gboolean tcam_set (Tcam* self, gchar* name, GVariant* value); */

/*     gchar* tcam_get_property_type (Tcam* self, gchar* name); */



/* G_END_DECLS */



/* #ifdef __cplusplus */
/*     } */
/* #endif */

/* #endif /\* TCAM_GOBJECT_H *\/ */
