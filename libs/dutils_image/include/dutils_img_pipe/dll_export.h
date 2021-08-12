
#pragma once

#if 0   // disable dll export
#define DUTILS_IMG_PIPE_EXPORT_FUNC

#ifndef DUTILS_IMG_PIPE_EXPORT_FUNC
#  if defined (_MSC_VER )
#    ifdef DUTILS_IMG_PIPE_EXPORT
#       define DUTILS_IMG_PIPE_EXPORT_FUNC  __declspec(dllexport)
#    else
#      define DUTILS_IMG_PIPE_EXPORT_FUNC  __declspec(dllimport)
#    endif
#  else
#    ifdef DUTILS_IMG_PIPE_EXPORT
#      define DUTILS_IMG_PIPE_EXPORT_FUNC __attribute__((visibility("default")))
#    else
#      define DUTILS_IMG_PIPE_EXPORT_FUNC 
#    endif
#  endif
#endif
#endif