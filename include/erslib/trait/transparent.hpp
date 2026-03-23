#pragma once

// ers
#include <erslib/hashing/base.hpp>


// Hash

namespace ers {
    template<typename T, typename Policy>
    struct transparent_hash_for {
        using policy_tag = Policy;
        using type = THashBase<T, policy_tag>;
    };

    template<typename T, typename Policy>
    using transparent_hash_for_t = transparent_hash_for<std::remove_cvref_t<T>, Policy>::type;
}


// Equal

namespace ers {
    template<typename T>
    struct transparent_equal_for {
        using type = std::equal_to<>;
    };

    template<typename T>
    using transparent_equal_for_t = transparent_equal_for<std::remove_cvref_t<T>>::type;
}
