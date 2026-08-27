#pragma once


// std
#include <functional>
#include <type_traits>

// boost
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/hashed_index.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/tag.hpp>

// ers
#include <erslib/core/adaptor/transparent_base.hpp>
#include <erslib/core/hashing/rapid.hpp>


namespace ers::impl::adaptor {
    template<auto Member>
    struct member_key {
        using class_type = std::remove_cvref_t<member_class_t<Member>>;
        using result_type = projected_t<Member, const class_type&>;

        result_type operator()(const class_type& v) const
            noexcept(project_is_nothrow<Member, const class_type&>()) {
            return project<Member>(v);
        }
    };
}


namespace ers::impl::adaptor {
    template<typename Tag, auto Member, template<typename> typename Hasher = RapidHash>
    using hashed_unique_by_member = std::conditional_t<
        std::is_void_v<Tag>,
        boost::multi_index::hashed_unique<
            member_key<Member>, member_hash<Hasher, Member>, member_equal<Member>
        >,
        boost::multi_index::hashed_unique<
            boost::multi_index::tag<Tag>, member_key<Member>, member_hash<Hasher, Member>, member_equal<Member>
        >
    >;

    template<typename Tag, auto Member, template<typename> typename Hasher = RapidHash>
    using hashed_non_unique_by_member = std::conditional_t<
        std::is_void_v<Tag>,
        boost::multi_index::hashed_non_unique<
            member_key<Member>, member_hash<Hasher, Member>, member_equal<Member>
        >,
        boost::multi_index::hashed_non_unique<
            boost::multi_index::tag<Tag>, member_key<Member>, member_hash<Hasher, Member>, member_equal<Member>
        >
    >;

    template<typename Tag, auto Member, typename Compare = std::less<>>
    using ordered_unique_by_member = std::conditional_t<
        std::is_void_v<Tag>,
        boost::multi_index::ordered_unique<member_key<Member>, Compare>,
        boost::multi_index::ordered_unique<boost::multi_index::tag<Tag>, member_key<Member>, Compare>
    >;

    template<typename Tag, auto Member, typename Compare = std::less<>>
    using ordered_non_unique_by_member = std::conditional_t<
        std::is_void_v<Tag>,
        boost::multi_index::ordered_non_unique<member_key<Member>, Compare>,
        boost::multi_index::ordered_non_unique<boost::multi_index::tag<Tag>, member_key<Member>, Compare>
    >;
}

namespace ers::impl {
    template<typename Value, typename IndexList, typename Alloc = std::allocator<Value>>
    using MultiIndex = boost::multi_index::multi_index_container<Value, IndexList, Alloc>;


    template<typename Container, typename K>
    decltype(auto) get(const Container& c, const K& k) {
        auto it = c.find(k);
        if (it == c.end())
            throw std::out_of_range("key not found");
        return *it;
    }


    template<typename Index, typename K, typename Modifier>
    bool modify_at(Index& idx, const K& k, Modifier&& mod) {
        auto it = idx.find(k);
        if (it == idx.end())
            return false;
        return idx.modify(it, std::forward<Modifier>(mod));
    }

    template<typename Index, typename K, typename Modifier, typename Rollback>
    bool modify_at(Index& idx, const K& k, Modifier&& mod, Rollback&& rollback) {
        auto it = idx.find(k);
        if (it == idx.end())
            return false;
        return idx.modify(it, std::forward<Modifier>(mod), std::forward<Rollback>(rollback));
    }


    template<typename Index, typename Modifier>
    size_t modify_all(Index& idx, Modifier mod) {
        size_t count = 0;

        for (auto it = idx.begin(); it != idx.end(); ) {
            const auto next = std::next(it);
            if (idx.modify(it, mod))
                ++count;
            it = next;
        }

        return count;
    }

    template<typename Index, typename Predicate, typename Modifier>
    size_t modify_if(Index& idx, Predicate pred, Modifier mod) {
        size_t count = 0;

        for (auto it = idx.begin(); it != idx.end(); ) {
            const auto next = std::next(it);
            if (std::invoke(pred, *it) && idx.modify(it, mod))
                ++count;
            it = next;
        }

        return count;
    }
}


// Exports

namespace ers::adaptor {
    using impl::adaptor::member_key;

    using impl::adaptor::hashed_unique_by_member;
    using impl::adaptor::hashed_non_unique_by_member;
    using impl::adaptor::ordered_unique_by_member;
    using impl::adaptor::ordered_non_unique_by_member;
}

namespace ers {
    using impl::MultiIndex;
    using impl::get;
    using impl::modify_at;
    using impl::modify_all;
    using impl::modify_if;
}
