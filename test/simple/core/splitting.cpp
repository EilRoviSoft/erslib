// catch2
#include <catch2/catch_test_macros.hpp>

// std
#include <ranges>
#include <string_view>
#include <utility>
#include <vector>

// ers
#include <erslib/core/algorithm.hpp>

// test
#include "support/test_case.hpp"


namespace {
    template<typename TSplit>
    std::vector<std::string_view> process(std::string_view input) {
        std::vector<std::string_view> result;

        for (auto it : TSplit(input))
            result.emplace_back(it);

        return result;
    }


    template<typename... TArgs>
    std::vector<std::string_view> make_vector(TArgs&&... args) {
        std::vector<std::string_view> result;

        result.reserve(sizeof...(args));
        (result.emplace_back(std::forward<TArgs>(args)), ...);

        return result;
    }
}


ERS_TEST_CASE("regular", "[core]") {
    REQUIRE(process<ers::algo::RegularSplitter>("hello world") == make_vector("hello", "world"));
    REQUIRE(process<ers::algo::RegularSplitter>("I love Isaac Iwasaki") == make_vector("I", "love", "Isaac", "Iwasaki"));
    REQUIRE(process<ers::algo::RegularSplitter>("I hate \"Sir Isaac Westcott\"") == make_vector("I", "hate", "\"Sir", "Isaac", "Westcott\""));
}

ERS_TEST_CASE("smart", "[core]") {
    REQUIRE(process<ers::algo::SmartSplitter>("hello world") == make_vector("hello", "world"));
    REQUIRE(process<ers::algo::SmartSplitter>("I love Isaac Iwasaki") == make_vector("I", "love", "Isaac", "Iwasaki"));
    REQUIRE(process<ers::algo::SmartSplitter>("I hate \"Sir Isaac Westcott\"") == make_vector("I", "hate", "Sir Isaac Westcott"));
}
