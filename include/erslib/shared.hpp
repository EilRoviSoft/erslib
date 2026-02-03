#pragma once

#ifdef ERSLIB_SHARED_LIBS
#ifdef _WIN32
#ifdef ERSLIB_EXPORTS
#define ERSLIB_API __declspec(dllexport)
#else
#define ERSLIB_API __declspec(dllimport)
#endif
#else
#define ERSLIB_API __attribute__((visibility("default")))
#endif
#else
#define ERSLIB_API
#endif
