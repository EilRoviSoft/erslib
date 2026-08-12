#pragma once

// std
#include <chrono>

// boost
#include <boost/unordered/unordered_flat_map.hpp>

// ers
#include <erslib/core/adaptor.hpp>
#include <erslib/core/hashing/rapid.hpp>


namespace dbio::impl {
    using kv_container_type = boost::unordered_flat_map<
        std::string, std::string,
        ers::adaptor::string_hash<ers::RapidHash>,
        ers::adaptor::equal<std::string>
    >;

    using ms_t = std::chrono::milliseconds;
}


// Exports

namespace dbio {
    using impl::ms_t;
}
