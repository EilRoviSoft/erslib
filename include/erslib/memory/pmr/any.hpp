#pragma once

// std
#include <memory>

// ers
#include <erslib/hashing/rapid.hpp>
#include <erslib/memory/pmr/any-impl.hpp>


constexpr size_t Size = 16;
constexpr size_t Align = alignof(std::max_align_t);
using Hasher = ers::RapidHash<std::string_view>;


namespace ers::pmr {
    class TAny : protected internal::TAnyImpl<Size, Align, Hasher> {
    public:
        struct placeholder_t {};

        
        // Default Constructor

        TAny() :
            TAnyImpl {
                .mr     = nullptr,
                .vtable = nullptr,
                .type   = meta::type_hash_v<placeholder_t, Hasher>,
                .policy = SboPolicy::Empty
            } {
        }

        
        // Copy

        TAny(const TAny& other) :
            TAnyImpl {
                .mr     = other.mr,
                .vtable = other.vtable,
                .type   = meta::type_hash_v<placeholder_t, Hasher>,
            } {
            vtable->copy(*this, other.data());
        }
        TAny& operator=(const TAny& other) {
            _soft_reset();

            mr = other.mr;
            vtable = other.vtable;
            type = other.type;
            vtable->copy(*this, other.data());

            return *this;
        }

        
        // Move

        TAny(TAny&& other) noexcept :
            TAnyImpl {
                .mr     = other.mr,
                .vtable = other.vtable,
                .type   = meta::type_hash_v<placeholder_t, Hasher>,
            } {
            vtable->move(*this, other.data());
        }
        TAny& operator=(TAny&& other) noexcept {
            _soft_reset();

            mr = other.mr;
            vtable = other.vtable;
            type = other.type;
            vtable->move(*this, other.data());

            return *this;
        }

        
        // Value initializer

        template<typename T>
            requires (!std::is_same_v<T, TAny>)
        TAny(T&& v, std::pmr::memory_resource* provided_mr = std::pmr::get_default_resource()) {
            emplace_with_resource<T>(provided_mr, std::forward<T>(v));
        }
        template<typename T>
            requires (!std::is_same_v<T, TAny>)
        TAny& operator=(T&& v) {
            emplace<T>(std::forward<T>(v));
            return *this;
        }

        
        // Value in-place initializer

        template<typename T, typename... Args>
        explicit TAny(std::in_place_type_t<T>, Args&&... args) {
            emplace<T>(std::forward<Args>(args)...);
        }
        template<typename T, typename... Args>
        explicit TAny(std::in_place_type_t<T>, std::pmr::memory_resource* provided_mr, Args&&... args) {
            emplace_with_resource<T>(provided_mr, std::forward<Args>(args)...);
        }

        
        // Destructor

        ~TAny() {
            _soft_reset();
        }


        // Modifiers

        void change_resource(std::pmr::memory_resource* provided_mr) {
            vtable->change_resource(*this, provided_mr);
        }

        void reset() {
            if (policy != SboPolicy::Empty) {
                _soft_reset();

                mr = nullptr;
                vtable = nullptr;
                type = meta::type_hash_v<placeholder_t, Hasher>;
                policy = SboPolicy::Empty;
            }
        }

        template<typename T, typename... Args>
            requires std::is_constructible_v<std::decay_t<T>, Args...>
                && std::is_copy_constructible_v<std::decay_t<T>>
        auto emplace(Args&&... args) {
            return emplace_with_resource<T>(
                mr ? mr : std::pmr::get_default_resource(),
                std::forward<Args>(args)...
            );
        }

        template<typename T, typename... Args>
            requires std::is_constructible_v<std::decay_t<T>, Args...>
                && std::is_copy_constructible_v<std::decay_t<T>>
        auto emplace_with_resource(std::pmr::memory_resource* provided_mr, Args&&... args) {
            using U = std::decay_t<T>;

            U* ptr;
            if (policy != SboPolicy::Empty && same_as<U>() && *mr == *provided_mr) {
                ptr = data_as<U>();

                vtable->destroy(*this);
                std::construct_at(ptr, std::forward<Args>(args)...);
            } else {
                if (policy != SboPolicy::Empty)
                    _soft_reset();

                mr = provided_mr;
                vtable = vtable_type::get<U>();
                type = meta::type_hash_v<U, Hasher>;

                ptr = vtable_type::impl_alloc<U>(*this);
                std::construct_at(ptr, std::forward<Args>(args)...);
            }

            return ptr;
        }


        // Observers

        [[nodiscard]]
        constexpr bool has_value() const {
            return policy == SboPolicy::Empty;
        }

        template<typename T>
        [[nodiscard]]
        constexpr bool same_as() const {
            return type == meta::type_hash_v<T, Hasher>();
        }

    private:
        // Can only be called
        // if it's guaranteed that "mr", "vtable" and "type" will be replaced after that.
        void _soft_reset() {
            vtable->destroy(*this);
            vtable->dealloc(*this);
        }
    };
}
