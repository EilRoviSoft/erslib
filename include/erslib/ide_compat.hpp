#pragma once


#define ERSLIB_IDE_COMPAT_SEEN 1


#if defined(__RESHARPER__) || defined(__INTELLISENSE__)

#if !defined(__cpp_concepts) || __cpp_concepts < 202002L
#undef __cpp_concepts
#define __cpp_concepts 202002L
#endif

#if !defined(__cpp_lib_ranges_concat) || __cpp_lib_ranges_concat < 202403L
#undef __cpp_lib_ranges_concat
#define __cpp_lib_ranges_concat 202403L
#endif


#if !defined(__linux__)
#define __linux__ 1
#endif

#if !defined(_GNU_SOURCE)
#define _GNU_SOURCE 1
#endif

#endif
