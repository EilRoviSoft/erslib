#pragma once


// Includes --------------------------------------------------------------------

#include <erslib/core/localization/config.hpp>
#include <erslib/core/localization/domain.hpp>
#include <erslib/core/localization/domain_view.hpp>
#include <erslib/core/localization/fingerprint.hpp>
#include <erslib/core/localization/fwd.hpp>
#include <erslib/core/localization/index.hpp>
#include <erslib/core/localization/key.hpp>
#include <erslib/core/localization/locale_table.hpp>
#include <erslib/core/localization/locale_view.hpp>
#include <erslib/core/localization/options.hpp>


// Exports ---------------------------------------------------------------------

namespace ers {
    using impl::locale_config_t;
    using impl::locale_config_ptr;
    using impl::common_locale_id;
    using impl::make_locale_config;

    using impl::locale_index_t;
    using impl::locale_index_ptr;

    using impl::EInvalidDomain;
    using impl::bundle_entry_t;
    using impl::bundle_loader_t;
    using impl::locale_table_options_t;
    using impl::domain_validator_t;

    using impl::fingerprint_t;
    using impl::make_localization_key;

    using impl::Domain;
    using impl::DomainPtr;
    using impl::DomainView;

    using impl::DomainAdapter;
    using impl::LocaleView;
    using impl::LocaleTable;
}
