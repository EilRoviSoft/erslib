#pragma once

// std
#include <memory>


// Fwd -------------------------------------------------------------------------

namespace ers::impl {
    struct locale_config_t;
    using locale_config_ptr = std::shared_ptr<const locale_config_t>;

    struct locale_index_t;
    using locale_index_ptr = std::shared_ptr<const locale_index_t>;

    class LocaleView;
    class LocaleTable;


    struct fingerprint_t;

    class Folder;
    using FolderPtr = std::shared_ptr<const Folder>;
    class FolderView;
}


// Exports ---------------------------------------------------------------------

namespace ers {
    using impl::locale_config_t;
    using impl::locale_config_ptr;

    using impl::locale_index_t;
    using impl::locale_index_ptr;

    using impl::LocaleView;
    using impl::LocaleTable;


    using impl::fingerprint_t;

    using impl::Folder;
    using impl::FolderPtr;
    using impl::FolderView;
}
