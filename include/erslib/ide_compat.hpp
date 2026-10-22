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

#endif
