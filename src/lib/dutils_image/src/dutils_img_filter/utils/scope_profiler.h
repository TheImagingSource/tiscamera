
#pragma once

#ifdef SCOPE_PROFILER_ENABLED_

#include "scope_profiler_profiling.h"

#	define PROFILER_STRING_MERGE_(a,b)  a##b
#   define PROFILER_STRING_MERGE_TMP(a,b)   PROFILER_STRING_MERGE_(a,b)

#	define PROFILER_START_SCOPED(name)	\
            auto PROFILER_STRING_MERGE_TMP(sec_profiler_,__COUNTER__) = scope_profiler::profiler_threaded::from_string( name )

#	define PROFILER_START_SCOPED_FUNC(func)	\
            auto PROFILER_STRING_MERGE_TMP(sec_profiler_,__COUNTER__) = scope_profiler::profiler_threaded::from_func( func )

#else
#	define PROFILER_START_SCOPED(name)
#   define PROFILER_START_SCOPED_FUNC(func)
#endif  // SCOPE_PROFILER_ENABLED_
