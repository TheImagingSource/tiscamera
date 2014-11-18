
#ifndef TCAM_COMPILER_DEFINES_H
#define TCAM_COMPILER_DEFINES_H

#if defined (__GNUC__)

#define VISIBILITY_INTERNAL _Pragma("GCC visibility push (internal)")
#define VISIBILITY_POP      _Pragma("GCC visibility pop")

#else

#define VISIBILITY_INTERNAL
#define VISIBILITY_POP

#endif

#endif /* TCAM_COMPILER_DEFINES_H */
