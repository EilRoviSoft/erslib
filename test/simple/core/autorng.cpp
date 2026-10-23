// catch2
#include <catch2/catch_get_random_seed.hpp>
#include <catch2/catch_test_macros.hpp>

// std
#include <array>
#include <chrono>
#include <random>
#include <ranges>
#include <string>
#include <string_view>
#include <thread>

// boost
#include <boost/unordered/unordered_flat_map.hpp>

// ers
#include <erslib/core/adaptor.hpp>
#include <erslib/core/thread_safe/map.hpp>
#include <erslib/core/hashing/rapid.hpp>
#include <erslib/core/type/time.hpp>
#include <erslib/core/type/timed_object.hpp>

// test
#include "support/test_case.hpp"


namespace {
    using namespace std::chrono_literals;
    constexpr ers::duration_t lifetime = 20ms;


    std::mt19937_64& gen() {
        static std::mt19937_64 instance(Catch::getSeed());
        return instance;
    }

    template<typename T>
    T random(T min, T max) {
        std::uniform_int_distribution distribution(min, max);
        return distribution(gen());
    }


    class AutoRng : public ers::ITimedObject<size_t> {
    public:
        AutoRng() :
            ITimedObject(lifetime) {
        }


    protected:
        [[nodiscard]] ers::Status load() const override {
            *m_expiring ^= random<size_t>(0, 10);
            return ers::ok;
        }
    };


    using Map = ers::thread_safe::Map<boost::unordered_flat_map<std::string, AutoRng,
        ers::adaptor::string_hash<ers::RapidHash>,
        ers::adaptor::equal<std::string>>>;
}


ERS_TEST_CASE("map", "[core]") {
    Map c;
    std::array<std::string_view, 4> vals = { "player", "enemy:knight", "enemy:mage", "enemy:rogue" };
    std::array<std::chrono::milliseconds, 10> intervals;

    for (auto& it : intervals)
        it = std::chrono::milliseconds(random(5, 15));

    for (auto it : vals)
        c.set(it);

    SECTION("intervals") {
        auto print = [&c, &vals]([[maybe_unused]] size_t i) mutable {
            for (auto it : vals) {
                [[maybe_unused]] size_t v = *c[it].get();
            }
        };

#if __cpp_lib_ranges_enumerate >= 202302L
        for (auto [i, d] : intervals | std::views::enumerate) {
#else
        size_t i = -1;
        for (auto d : intervals) {
            i++;
#endif
            print(i);
            std::this_thread::sleep_for(d);
        }
        print(intervals.size());
    }
}
