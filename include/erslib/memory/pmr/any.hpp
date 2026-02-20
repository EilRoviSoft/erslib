#pragma once

// std
#include <memory>

// ers
#include <erslib/memory/pmr/any-impl.hpp>


// Implementation

namespace ers::internal {
    template<size_t Size, size_t Align>
    class TAny : protected TAnyImpl<Size, Align> {
        using base_type = TAnyImpl<Size, Align>;
        using vtable_type = base_type::vtable_type;
        using storage_type = base_type::storage_type;

    public:
        struct placeholder_t {};


        // Default Constructor

        TAny() :
            base_type {
                .mr      = nullptr,
                .vtable  = nullptr,
                .type    = meta::type_hash_v<placeholder_t>,
                .policy  = SboPolicy::Empty,
                .storage = storage_type { .heap = nullptr }
            } {
        }


        // Copy

        TAny(const TAny& other) :
            base_type {
                .mr      = other.mr,
                .vtable  = other.vtable,
                .type    = meta::type_hash_v<placeholder_t>,
                .policy  = SboPolicy::Empty,
                .storage = storage_type { .heap = nullptr }
            } {
            this->vtable->copy(*this, other.data());
        }
        TAny& operator=(const TAny& other) {
            _soft_reset();

            this->mr = other.mr;
            this->vtable = other.vtable;
            this->type = other.type;
            this->vtable->copy(*this, other.data());

            return *this;
        }


        // Move

        TAny(TAny&& other) noexcept :
            base_type {
                .mr      = other.mr,
                .vtable  = other.vtable,
                .type    = meta::type_hash_v<placeholder_t>,
                .policy  = SboPolicy::Empty,
                .storage = storage_type { .heap = nullptr }
            } {
            this->vtable->move(*this, other.data());
        }
        TAny& operator=(TAny&& other) noexcept {
            _soft_reset();

            this->mr = other.mr;
            this->vtable = other.vtable;
            this->type = other.type;
            this->vtable->move(*this, other.data());

            return *this;
        }


        // Value initializer

        template<typename T>
            requires (!std::is_same_v<T, TAny>)
        TAny(T&& v, std::pmr::memory_resource* provided_mr = std::pmr::get_default_resource()) :
            base_type {
                .mr      = nullptr,
                .vtable  = nullptr,
                .type    = meta::type_hash_v<placeholder_t>,
                .policy  = SboPolicy::Empty,
                .storage = storage_type { .heap = nullptr }
            } {
            emplace_with_resource<T>(provided_mr, std::forward<T>(v));
        }
        template<typename T>
            requires (!std::is_same_v<T, TAny>)
        TAny& operator=(T&& v) {
            this->policy = SboPolicy::Empty;

            emplace<T>(std::forward<T>(v));

            return *this;
        }


        // Value in-place initializer

        template<typename T, typename... Args>
        explicit TAny(std::in_place_type_t<T>, Args&&... args) :
            base_type {
                .mr      = nullptr,
                .vtable  = nullptr,
                .type    = meta::type_hash_v<placeholder_t>,
                .policy  = SboPolicy::Empty,
                .storage = storage_type { .heap = nullptr }
            } {
            emplace<T>(std::forward<Args>(args)...);
        }
        template<typename T, typename... Args>
        explicit TAny(std::in_place_type_t<T>, std::pmr::memory_resource* provided_mr, Args&&... args) :
            base_type {
                .policy = SboPolicy::Empty
            } {
            emplace_with_resource<T>(provided_mr, std::forward<Args>(args)...);
        }


        // Destructor

        ~TAny() {
            _soft_reset();
        }


        // Modifiers

        void change_resource(std::pmr::memory_resource* provided_mr) {
            this->vtable->change_resource(*this, provided_mr);
        }

        void reset() {
            if (this->policy != SboPolicy::Empty) {
                _soft_reset();

                this->mr = nullptr;
                this->vtable = nullptr;
                this->type = meta::type_hash_v<placeholder_t>;
                this->policy = SboPolicy::Empty;
            }
        }

        template<typename T, typename... Args>
            requires std::is_constructible_v<std::decay_t<T>, Args...>
            && std::is_copy_constructible_v<std::decay_t<T>>
        auto emplace(Args&&... args) {
            return emplace_with_resource<T>(
                this->mr ? this->mr : std::pmr::get_default_resource(),
                std::forward<Args>(args)...
            );
        }

        template<typename T, typename... Args>
            requires std::is_constructible_v<std::decay_t<T>, Args...>
            && std::is_copy_constructible_v<std::decay_t<T>>
        auto emplace_with_resource(std::pmr::memory_resource* provided_mr, Args&&... args) {
            using U = std::decay_t<T>;

            U* ptr;
            if (this->policy != SboPolicy::Empty && same_as<U>() && *this->mr == *provided_mr) {
                ptr = this->template data_as<U>();

                this->vtable->destroy(*this);
                std::construct_at(ptr, std::forward<Args>(args)...);
            } else {
                if (this->policy != SboPolicy::Empty)
                    _soft_reset();

                this->mr = provided_mr;
                this->vtable = &vtable_type::template get<U>();
                this->type = meta::type_hash_v<U>;

                ptr = vtable_type::template impl_alloc<U>(*this);
                std::construct_at(ptr, std::forward<Args>(args)...);
            }

            return ptr;
        }


        // Observers

        [[nodiscard]]
        constexpr bool has_value() const {
            return this->policy == SboPolicy::Empty;
        }

        template<typename T>
        [[nodiscard]]
        constexpr bool same_as() const {
            return this->type == meta::type_hash_v<T>;
        }

    private:
        // Can only be called
        // if it's guaranteed that "mr", "vtable" and "type" will be replaced after that.
        void _soft_reset() {
            this->vtable->destroy(*this);
            this->vtable->dealloc(*this);
        }
    };
}


// General use-case

namespace ers::pmr {
    using Any = internal::TAny<32, alignof(std::max_align_t)>;
}
