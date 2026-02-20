// std
#include <array>
#include <chrono>
#include <print>
#include <random>
#include <ranges>
#include <string>
#include <thread>

// catch2
#include <catch2/catch_get_random_seed.hpp>
#include <catch2/catch_test_macros.hpp>

// ers
#include <erslib/container/unordered_table.hpp>
#include <erslib/hashing/rapid.hpp>
#include <erslib/thread_safe/map.hpp>
#include <erslib/type/time.hpp>
#include <erslib/type/timed_object.hpp>
#include <erslib/util/string.hpp>


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


using Map = ers::thread_safe::Map<
    std::string,
    AutoRng,
    ers::UnorderedMap<
        std::string,
        AutoRng,
        ers::util::string_hash_adaptor<ers::RapidHash>,
        ers::util::string_equal
    >
>;


TEST_CASE("testing thread_safe map", "[ts_map]") {
    Map c;
    std::array<std::string_view, 4> vals = { "player", "enemy:knight", "enemy:mage", "enemy:rogue" };
    std::array<std::chrono::milliseconds, 10> intervals;

    for (auto& it : intervals)
        it = std::chrono::milliseconds(random(5, 15));

    for (auto it : vals)
        c.set(it);

    //SECTION("intervaled output") {
    //    auto print = [&c, &vals](size_t i) mutable {
    //        //std::print("\niteration {}:\n", i);
    //        for (auto it : vals) {
    //            size_t v = *c[it].get();
    //            //std::print("{:3}: {}\n", it, v);
    //        }
    //    };

    //    for (auto [i, d] : intervals | std::views::enumerate) {
    //        print(i);
    //        //std::print("waiting for {}\n", d);
    //        std::this_thread::sleep_for(d);
    //    }
    //    print(intervals.size());
    //}
}
