#pragma once

// std
#include <memory>


namespace ers::impl {
    struct locale_config_t;
    using locale_config_ptr = std::shared_ptr<const locale_config_t>;

    struct locale_index_t;
    using locale_index_ptr = std::shared_ptr<const locale_index_t>;

    struct locale_table_options_t;
    struct bundle_entry_t;
    struct bundle_loader_t;

    class LocaleView;
    class LocaleTable;


    struct fingerprint_t;

    class Domain;
    using DomainPtr = std::shared_ptr<const Domain>;
    class DomainView;
}
