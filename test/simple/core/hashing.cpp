// catch2
#include <catch2/catch_test_macros.hpp>

// std
#include <utility>

// ers
#include <erslib/core/fwd.hpp>
#include <erslib/core/hashing/rapid.hpp>
#include <erslib/core/type/uuid.hpp>

// test
#include "support/test_case.hpp"


ERS_TEST_CASE("uuid and pairs", "[core]") {
    auto a = ers::uuid_from_string("42fbbcaf-bc00-11ef-a8f8-042b581c0d88");
    auto b = ers::uuid_from_string("42fbbcac-bc00-11ef-a8f8-042b581c0d88");

    CHECK(ers::RapidHash<ers::uuid> {}(a) == ers::RapidHash<ers::uuid> {}(a));
    CHECK(ers::RapidHash<ers::uuid> {}(a) != ers::RapidHash<ers::uuid> {}(b));

    using key_t = std::pair<ers::uuid, ers::uuid>;
    CHECK(ers::RapidHash<key_t> {}({ a, b }) != ers::RapidHash<key_t> {}({ b, a }));

    ers::HashMap<key_t, int> map;
    map.try_emplace({ a, b }, 1);
    CHECK(map.contains({ a, b }));
    CHECK(!map.contains({ b, a }));
}
