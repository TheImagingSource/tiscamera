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

#include "tcamprop_helper.h"

#include <gst-helper/gst_gvalue_helper.h>
#include <tcamprop1.0_consumer/tcamprop1_consumer.h>
#include <tcamprop1.0_gobject/tcam_gerror.h>
#include <tcamprop1.0_gobject/tcam_property_provider.h>


void tcamprop_helper_set_tcam_boolean( TcamPropertyProvider* self, const gchar* name, gboolean value, GError** err )
{
    auto ptr_base = tcam_property_provider_get_tcam_property( self, name, err );
    if( *err || ptr_base == nullptr )
    {
        return;
    }
    if (!TCAM_IS_PROPERTY_BOOLEAN(ptr_base))
    {
        tcamprop1_gobj::set_gerror( err, TCAM_ERROR_PROPERTY_TYPE_INCOMPATIBLE );
        return;
    }

    tcam_property_boolean_set_value(TCAM_PROPERTY_BOOLEAN(ptr_base), value, err);

    g_object_unref(ptr_base);
}


void tcamprop_helper_set_tcam_integer( TcamPropertyProvider* self, const gchar* name, gint64 value, GError** err )
{
    auto ptr_base = tcam_property_provider_get_tcam_property( self, name, err );
    if( *err || ptr_base == nullptr )
    {
        return;
    }
    if (!TCAM_IS_PROPERTY_INTEGER(ptr_base))
    {
        tcamprop1_gobj::set_gerror( err, TCAM_ERROR_PROPERTY_TYPE_INCOMPATIBLE );
        return;
    }

    tcam_property_integer_set_value(TCAM_PROPERTY_INTEGER(ptr_base), value, err);

    g_object_unref(ptr_base);
}


void tcamprop_helper_set_tcam_float( TcamPropertyProvider* self, const gchar* name, gdouble value, GError** err )
{
    auto ptr_base = tcam_property_provider_get_tcam_property( self, name, err );
    if( *err || ptr_base == nullptr )
    {
        return;
    }
    if (!TCAM_IS_PROPERTY_FLOAT(ptr_base))
    {
        tcamprop1_gobj::set_gerror( err, TCAM_ERROR_PROPERTY_TYPE_INCOMPATIBLE );
        return;
    }

    tcam_property_float_set_value(TCAM_PROPERTY_FLOAT(ptr_base), value, err);

    g_object_unref(ptr_base);
}


void tcamprop_helper_set_tcam_enumeration( TcamPropertyProvider* self, const gchar* name, const gchar* value, GError** err )
{
    auto ptr_base = tcam_property_provider_get_tcam_property( self, name, err );
    if( *err || ptr_base == nullptr )
    {
        return;
    }
    if (!TCAM_IS_PROPERTY_ENUMERATION(ptr_base))
    {
        tcamprop1_gobj::set_gerror( err, TCAM_ERROR_PROPERTY_TYPE_INCOMPATIBLE );
        return;
    }

    tcam_property_enumeration_set_value( TCAM_PROPERTY_ENUMERATION( ptr_base ), value, err );

    g_object_unref(ptr_base);
}


void tcamprop_helper_set_tcam_command( TcamPropertyProvider* self, const gchar* name, GError** err )
{
    auto ptr_base = tcam_property_provider_get_tcam_property( self, name, err );
    if( *err || ptr_base == nullptr )
    {
        return;
    }
    if (!TCAM_IS_PROPERTY_COMMAND(ptr_base))
    {
        tcamprop1_gobj::set_gerror( err, TCAM_ERROR_PROPERTY_TYPE_INCOMPATIBLE );
        return;
    }

    tcam_property_command_set_command(TCAM_PROPERTY_COMMAND(ptr_base), err);

    g_object_unref(ptr_base);
}


gboolean tcamprop_helper_get_tcam_boolean( TcamPropertyProvider* self, const gchar* name, GError** err )
{
    auto ptr_base = tcam_property_provider_get_tcam_property( self, name, err );
    if( *err || ptr_base == nullptr )
    {
        return false;
    }
    if (!TCAM_IS_PROPERTY_BOOLEAN(ptr_base))
    {
        tcamprop1_gobj::set_gerror( err, TCAM_ERROR_PROPERTY_TYPE_INCOMPATIBLE );
        return false;
    }

    auto rval = tcam_property_boolean_get_value(TCAM_PROPERTY_BOOLEAN(ptr_base), err);

    g_object_unref(ptr_base);
    return rval;
}


gint64 tcamprop_helper_get_tcam_integer( TcamPropertyProvider* self, const gchar* name, GError** err )
{
    auto ptr_base = tcam_property_provider_get_tcam_property( self, name, err );
    if( *err || ptr_base == nullptr )
    {
        return 0;
    }
    if (!TCAM_IS_PROPERTY_INTEGER(ptr_base))
    {
        tcamprop1_gobj::set_gerror( err, TCAM_ERROR_PROPERTY_TYPE_INCOMPATIBLE );
        return 0;
    }

    auto rval = tcam_property_integer_get_value(TCAM_PROPERTY_INTEGER(ptr_base), err);

    g_object_unref(ptr_base);
    return rval;
}


gdouble tcamprop_helper_get_tcam_float( TcamPropertyProvider* self, const gchar* name, GError** err )
{
    auto ptr_base = tcam_property_provider_get_tcam_property( self, name, err );
    if( *err || ptr_base == nullptr )
    {
        return 0;
    }
    if (!TCAM_IS_PROPERTY_FLOAT(ptr_base))
    {
        tcamprop1_gobj::set_gerror( err, TCAM_ERROR_PROPERTY_TYPE_INCOMPATIBLE );
        return 0;
    }

    auto rval = tcam_property_float_get_value(TCAM_PROPERTY_FLOAT(ptr_base), err);

    g_object_unref(ptr_base);
    return rval;
}


gchar* tcamprop_helper_get_tcam_enumeration( TcamPropertyProvider* self, const gchar* name, GError** err )
{
    auto ptr_base = tcam_property_provider_get_tcam_property( self, name, err );
    if( *err || ptr_base == nullptr )
    {
        return nullptr;
    }
    if (!TCAM_IS_PROPERTY_ENUMERATION(ptr_base))
    {
        tcamprop1_gobj::set_gerror( err, TCAM_ERROR_PROPERTY_TYPE_INCOMPATIBLE );
        return nullptr;
    }

    auto rval = tcam_property_enumeration_get_value( TCAM_PROPERTY_ENUMERATION( ptr_base ), err );

    g_object_unref(ptr_base);
    return rval;
}
