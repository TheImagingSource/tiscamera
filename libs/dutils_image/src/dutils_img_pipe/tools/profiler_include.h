
//#define SCOPE_PROFILER_ENABLED_

#include "../../dutils_img_filter/utils/scope_profiler.h"

#define DUTIL_PROFILE_SECTION( name )   PROFILER_START_SCOPED( name )
#define DUTIL_PROFILE_FUNCTION()        PROFILER_START_SCOPED( __FUNCTION__ )
#define DUTIL_PROFILE_SECTION_FUNC( func )   PROFILER_START_SCOPED( func )
