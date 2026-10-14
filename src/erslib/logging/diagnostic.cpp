#include "erslib/logging/diagnostic.hpp"

// quill
#include <quill/LogMacros.h>


void ers::logging::impl::log(quill::Logger* logger, const Diagnostic& what) {
    switch (what.severity()) {
#define X(SEVERITY, LOG_LEVEL) \
    case ESeverity::SEVERITY: \
        LOG_##LOG_LEVEL(logger, "{}", what.to_string(true)); \
        break

        X(Debug, DEBUG);
        X(Info, INFO);
        X(Warning, WARNING);
        X(Error, ERROR);
        X(Crit, CRITICAL);

#undef X
    }
}
