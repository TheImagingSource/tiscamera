
// #include "stdafx.h"

#include "memcpy_opt.h"

#include "cpu_features.h"
#include "../sse_helper/sse_base_utils.h"

FORCEINLINE
static void*	memcpy_opt_( void* dest, void* src, size_t cbcount, unsigned int cpu_features )
{
    return memcpy( dest, src, cbcount );
}

void*	memcpy_opt( void* dest, void* src, size_t cbcount )
{
	return memcpy_opt_( dest, src, cbcount, 0);
}

void* memcpy_opt_test( void* dest, void* src, size_t cbcount, unsigned int cpu_features )
{
	return memcpy_opt_( dest, src, cbcount, cpu_features );
}
