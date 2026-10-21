#include "erslib/logging/factory.hpp"

// std
#include <memory>
#include <mutex>
#include <utility>
#include <vector>

// quill
#include <quill/Backend.h>
#include <quill/Frontend.h>
#include <quill/sinks/ConsoleSink.h>
#include <quill/sinks/RotatingFileSink.h>


quill::Logger* ers::logging::impl::make_logger(const logger_options_t& options) {
    static std::once_flag backend_started;
    std::call_once(backend_started, [] {
        quill::BackendOptions backend_options;

        backend_options.check_printable_char = [](char c) {
            return static_cast<unsigned char>(c) >= 0x80
                || (c >= ' ' && c <= '~') || c == '\n' || c == '\t' || c == '\r';
        };

        quill::Backend::start(backend_options);
    });

    std::vector<std::shared_ptr<quill::Sink>> sinks;

    sinks.emplace_back(quill::Frontend::create_or_get_sink<quill::RotatingFileSink>(
        options.file.string(),
        [&options] {
            quill::RotatingFileSinkConfig cfg;

            cfg.set_open_mode('w');
            cfg.set_filename_append_option(quill::FilenameAppendOption::StartDateTime);
            cfg.set_overwrite_rolled_files(true);
            cfg.set_remove_old_files(false);

            cfg.set_rotation_max_file_size(options.max_file_mib * 1024 * 1024);
            cfg.set_max_backup_files(options.max_files);

            return cfg;
        }()
    ));

    if (options.mirror_to_console)
        sinks.emplace_back(quill::Frontend::create_or_get_sink<quill::ConsoleSink>(options.name + "_console"));

    auto* logger = quill::Frontend::create_or_get_logger(
        options.name,
        std::move(sinks),
        quill::PatternFormatterOptions {
            options.pattern,
            options.timestamp,
            quill::Timezone::GmtTime,
            false
        }
    );

    logger->set_log_level(options.level);

    return logger;
}
