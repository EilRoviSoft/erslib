#include "erslib/web/log.hpp"

// quill
#include <quill/LogMacros.h>


// Member functions

ers::web::impl::LogHandler::LogHandler(quill::Logger* logger) :
    _logger(logger) {
}


// Handlers

void ers::web::impl::LogHandler::log(const std::string& message, crow::LogLevel level) {
    switch (level) {
#define X(CROW, QUILL) \
    case crow::LogLevel::CROW: \
        LOG_##QUILL(_logger, "{}", message); \
        break

        X(Debug, DEBUG);
        X(Info, INFO);
        X(Warning, WARNING);
        X(Error, ERROR);
        X(Critical, CRITICAL);

#undef X
    }
}
