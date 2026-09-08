// std
#include <string>
#include <vector>

// boost
#include <boost/container/flat_map.hpp>
#include <boost/container/flat_set.hpp>
#include <boost/container/small_vector.hpp>
#include <boost/container/static_vector.hpp>
#include <boost/container/vector.hpp>
#include <boost/multi_index/hashed_index.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/random_access_index.hpp>
#include <boost/multi_index/sequenced_index.hpp>
#include <boost/multi_index/tag.hpp>
#include <boost/multi_index_container.hpp>
#include <boost/unordered/unordered_flat_map.hpp>
#include <boost/unordered/unordered_flat_set.hpp>
#include <boost/unordered/unordered_map.hpp>
#include <boost/unordered/unordered_node_map.hpp>
#include <boost/unordered/unordered_node_set.hpp>
#include <boost/unordered/unordered_set.hpp>


namespace bmi = boost::multi_index;


namespace {
    struct Rec {
        size_t id;
        std::string name;
    };

    struct Point {
        int x;
        int y;
    };

    struct ByX {
        bool operator()(const Point& a, const Point& b) const { return a.x < b.x; }
    };
    struct HashX {
        std::size_t operator()(const Point& p) const { return static_cast<std::size_t>(p.x); }
    };
    struct EqX {
        bool operator()(const Point& a, const Point& b) const { return a.x == b.x; }
    };


    struct by_id {};
    struct by_name {};
    struct by_seq {};
    struct by_rnd {};


    using Full = bmi::multi_index_container<Rec,
        bmi::indexed_by<
            bmi::ordered_unique<bmi::tag<by_id>, bmi::member<Rec, size_t, &Rec::id>>,
            bmi::hashed_non_unique<bmi::tag<by_name>, bmi::member<Rec, std::string, &Rec::name>>,
            bmi::sequenced<bmi::tag<by_seq>>,
            bmi::random_access<bmi::tag<by_rnd>>
        >
    >;

    using HashOnly = bmi::multi_index_container<Rec,
        bmi::indexed_by<bmi::hashed_unique<bmi::member<Rec, size_t, &Rec::id>>>>;

    using HashMultiOnly = bmi::multi_index_container<Rec,
        bmi::indexed_by<bmi::hashed_non_unique<bmi::member<Rec, std::string, &Rec::name>>>>;

    using SeqOnly = bmi::multi_index_container<Rec, bmi::indexed_by<bmi::sequenced<>>>;

    using RndOnly = bmi::multi_index_container<Rec, bmi::indexed_by<bmi::random_access<>>>;

    using OrdMulti = bmi::multi_index_container<Rec,
        bmi::indexed_by<bmi::ordered_non_unique<bmi::member<Rec, std::string, &Rec::name>>>>;
}

int main() {
    // ---- Boost.MultiIndex ----------------------------------------------

    Full full;
    for (size_t i = 0; i < 7; ++i) full.insert({ i, "n" + std::to_string(i % 3) });
    const auto& by_id_idx = full.get<by_id>();
    const auto& by_name_idx = full.get<by_name>();
    const auto& by_seq_idx = full.get<by_seq>();
    const auto& by_rnd_idx = full.get<by_rnd>();
    auto mi_it = full.begin();

    HashOnly ho;
    for (size_t i = 0; i < 40; ++i) {
        ho.insert({
            .id   = i,
            .name = "h"
        });
    }

    HashMultiOnly hmo;
    for (size_t i = 0; i < 40; ++i) {
        hmo.insert({
            .id   = i,
            .name = "k" + std::to_string(i % 5)
        });
    }

    SeqOnly so;
    for (size_t i = 0; i < 4; ++i) {
        so.push_back({
            .id   = i,
            .name = "s"
        });
    }

    RndOnly ro;
    for (size_t i = 0; i < 4; ++i) {
        ro.push_back({
            .id   = i,
            .name = "r"
        });
    }

    OrdMulti om;
    for (size_t i = 0; i < 6; ++i) {
        om.insert({
            .id   = i,
            .name = "o" + std::to_string(i % 2)
        });
    }

    Full empty_full;
    HashOnly empty_ho;
    SeqOnly empty_so;
    RndOnly empty_ro;


    // ---- Boost.Unordered ------------------------------------------------

    boost::unordered_flat_map<int, std::string> ufm;
    for (int i = 0; i < 40; ++i)
        ufm.emplace(i, "v" + std::to_string(i));

    boost::unordered_flat_set<int> ufs;
    for (int i = 0; i < 40; ++i)
        ufs.insert(i);

    boost::unordered_node_map<int, std::string> unm;
    for (int i = 0; i < 20; ++i)
        unm.emplace(i, "n");

    boost::unordered_node_set<std::string> uns = { "a", "b" };
    boost::unordered_map<int, std::string> um;
    for (int i = 0; i < 30; ++i)
        um.emplace(i, "m");

    boost::unordered_multimap<int, int> umm;
    for (int i = 0; i < 10; ++i)
        umm.emplace(i % 3, i);

    boost::unordered_set<int> us;
    for (int i = 0; i < 30; ++i)
        us.insert(i);

    boost::unordered_flat_set<Point, HashX, EqX> ufs_custom = {
        { .x = 1, .y = 1 },
        { .x = 2, .y = 2 },
        { .x = 3, .y = 3 }
    };

    boost::unordered_flat_map<int, int> ufm_empty;
    boost::unordered_map<int, int> um_empty;

    auto foa_it = ufm.begin();
    auto fca_it = um.begin();

    // ---- Boost.Container ------------------------------------------------

    boost::container::flat_map<int, std::string> fm = {
        { 1, "a" },
        { 2, "b" },
        { 3, "c" }
    };
    boost::container::flat_multimap<int, int> fmm = {
        { 1, 1 },
        { 1, 2 }
    };
    boost::container::flat_map<int, int> fm_empty;
    
    auto fm_it = fm.find(2);


    boost::container::flat_set<int, std::less<>, boost::container::small_vector<int, 4>> fs_small =
        { 1, 2, 3, 4, 5, 6 };

    boost::container::flat_set<int, std::less<>, boost::container::static_vector<int, 8>> fs_static =
        { 3, 1, 2 };

    boost::container::flat_set<int, std::less<>, std::vector<int>> fs_std =
        { 5, 4, 3 };

    boost::container::flat_map<int, int, std::less<>, std::vector<std::pair<int, int>>> fm_std = {
        { 1, 10 },
        { 2, 20 }
    };

    boost::container::flat_set<Point, ByX> fs_custom = {
        { .x = 3, .y = 3 },
        { .x = 1, .y = 1 },
        { .x = 2, .y = 2 }
    };

    boost::container::vector<std::string> bvs = { "alpha", "beta" };
    boost::container::vector<int> bv_empty;
    boost::container::small_vector<int, 8> sv_inline = { 1, 2 };
    boost::container::small_vector<int, 2> sv_heap = { 1, 2, 3, 4, 5 };
    boost::container::static_vector<int, 4> stv = { 9, 8, 7 };
    boost::container::static_vector<int, 4> stv_empty;


    __asm__ volatile("nop");

    (void) by_id_idx;
    (void) by_name_idx;
    (void) by_seq_idx;
    (void) by_rnd_idx;
    (void) mi_it;
    (void) ho;
    (void) hmo;
    (void) so;
    (void) ro;
    (void) om;
    (void) empty_full;
    (void) empty_ho;
    (void) empty_so;
    (void) empty_ro;
    (void) ufs;
    (void) unm;
    (void) uns;
    (void) umm;
    (void) us;
    (void) ufs_custom;
    (void) ufm_empty;
    (void) um_empty;
    (void) foa_it;
    (void) fca_it;
    (void) fmm;
    (void) fm_empty;
    (void) fm_it;
    (void) fs_small;
    (void) fs_static;
    (void) fs_std;
    (void) fm_std;
    (void) fs_custom;
    (void) bvs;
    (void) bv_empty;
    (void) sv_inline;
    (void) sv_heap;
    (void) stv;
    (void) stv_empty;
}
