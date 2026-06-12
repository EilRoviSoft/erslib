// std
#include <array>
#include <chrono>
#include <random>
#include <ranges>
#include <string>
#include <thread>

// boost
#include <boost/unordered/unordered_flat_map.hpp>

// doctest
#include <doctest/doctest.h>

// ers
#include <erslib/adaptor/transparent/string.hpp>
#include <erslib/hashing/rapid.hpp>
#include <erslib/thread_safe/map.hpp>
#include <erslib/type/time.hpp>
#include <erslib/type/timed_object.hpp>
#include <erslib/util/string.hpp>


namespace {
    using namespace std::chrono_literals;
    constexpr ers::duration_t lifetime = 20ms;


    std::mt19937_64& gen() {
        static std::mt19937_64 instance(doctest::getContextOptions()->rand_seed);
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
        ers::string_hash_adaptor<ers::RapidHash>,
        ers::equal_adaptor<std::string>>>;
}


TEST_CASE("testing thread_safe map") {
    Map c;
    std::array<std::string_view, 4> vals = { "player", "enemy:knight", "enemy:mage", "enemy:rogue" };
    std::array<std::chrono::milliseconds, 10> intervals;

    for (auto& it : intervals)
        it = std::chrono::milliseconds(random(5, 15));

    for (auto it : vals)
        c.set(it);

    SUBCASE("intervaled output") {
        auto print = [&c, &vals](size_t i) mutable {
            //MESSAGE("\niteration {}:\n", i);
            for (auto it : vals) {
                size_t v = *c[it].get();
                //MESSAGE("{:3}: {}\n", it, v);
            }
        };

        for (auto [i, d] : intervals | std::views::enumerate) {
            print(i);
            //MESSAGE("waiting for {}\n", d);
            std::this_thread::sleep_for(d);
        }
        print(intervals.size());
    }
}
