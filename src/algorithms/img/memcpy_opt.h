
#ifndef OPT_MEMCPY_H_INC_
#define OPT_MEMCPY_H_INC_

#include <cstddef>

void*	memcpy_opt( void* dest, void* src, size_t cbcount );
void*	memcpy_opt_test( void* dest, void* src, size_t cbcount, unsigned int cpu_features );

#endif // OPT_MEMCPY_H_INC_
