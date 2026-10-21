#pragma once


#define ERSLIB_IDE_COMPAT_SEEN 1


#if defined(__RESHARPER__) || defined(__INTELLISENSE__)
#if !defined(__cpp_concepts) || __cpp_concepts < 202002L
#undef __cpp_concepts
#define __cpp_concepts 202002L
#endif
#endif
