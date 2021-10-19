
#pragma once

#include <Tcam-1.0.h>

namespace tcamprop1_gobj
{

void            provider_set_tcam_boolean( TcamPropertyProvider* self, const gchar* name, gboolean value, GError** err );
void            provider_set_tcam_integer( TcamPropertyProvider* self, const gchar* name, gint64 value, GError** err );
void            provider_set_tcam_float( TcamPropertyProvider* self, const gchar* name, gdouble value, GError** err );
void            provider_set_tcam_enumeration( TcamPropertyProvider* self, const gchar* name, const gchar* value, GError** err );
void            provider_set_tcam_command( TcamPropertyProvider* self, const gchar* name, GError** err );

gboolean        provider_get_tcam_boolean( TcamPropertyProvider* self, const gchar* name, GError** err );
gint64          provider_get_tcam_integer( TcamPropertyProvider* self, const gchar* name, GError** err );
gdouble         provider_get_tcam_float( TcamPropertyProvider* self, const gchar* name, GError** err );
const gchar*    provider_get_tcam_enumeration( TcamPropertyProvider* self, const gchar* name, GError** err );

}