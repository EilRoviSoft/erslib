#pragma once

// std
#include <string>

// ers
#include <erslib/hashing/base.hpp>
#include <erslib/trait/transparent.hpp>


// Hash

namespace ers::adaptor {
    template<typename Policy>
    struct string_hash {
        using is_transparent = void;


        size_t operator()(const std::string_view sv) const noexcept {
            return ers::THashBase<std::string_view, Policy> {}(sv);
        }
        size_t operator()(const std::string& str) const noexcept {
            return ers::THashBase<std::string, Policy> {}(str);
        }
        size_t operator()(const char* cstr) const noexcept {
            return ers::THashBase<const char*, Policy> {}(cstr);
        }
    };
}


template<typename Policy>
struct ers::transparent_hash_for<std::string, Policy> {
    using type = adaptor::string_hash<Policy>;
};


// Equal

namespace ers::adaptor {
    
}
