#pragma once


#ifdef __clang__
#define ERS_RESTRICT __restrict
#elifdef __GNUC__
#define ERS_RESTRICT __restrict__
#elifdef _MSC_VER
#define ERS_RESTRICT __restrict__
#else
#error Unsupported compiler
#endif
