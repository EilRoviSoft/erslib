// doctest
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>


// Sanitizers ------------------------------------------------------------------

extern "C" [[gnu::used, gnu::visibility("default")]]
const char* __asan_default_options() {
    return "abort_on_error=1:log_path=stdout";
}

extern "C" [[gnu::used, gnu::visibility("default")]]
const char* __ubsan_default_options() {
    return "print_stacktrace=1";
}
