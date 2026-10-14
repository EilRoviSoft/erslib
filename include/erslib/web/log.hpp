#pragma once

#ifdef ERSLIB_WEB_HAS_LOGGING

// std
#include <string>

// crow
#include <crow/logging.h>

// quill
#include <quill/Logger.h>

// export
#include <erslib/export.hpp>


namespace ers::web::impl {
    class ERSLIB_EXPORT LogHandler : public crow::ILogHandler {
    public:
        // Member functions

        explicit LogHandler(quill::Logger* logger);


        // Handlers

        void log(const std::string& message, crow::LogLevel level) override;


    private:
        quill::Logger* _logger;
    };
}

#endif
