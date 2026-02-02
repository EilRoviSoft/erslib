#pragma once

// std
#include <optional>

namespace ers {
    template<typename T>
    using ref = std::reference_wrapper<T>;

    template<typename T>
    using cref = std::reference_wrapper<const T>;

    template<typename T>
    class optref {
    public:
        optref() = default;
        optref(T& ref) :
            _ref(std::ref(ref)) {
        }
        optref(std::nullopt_t) :
            _ref(std::nullopt) {
        }

        explicit operator bool() const { return _ref.has_value(); }

        T& operator*() { return _ref->get(); }
        const T& operator*() const { return _ref->get(); }

        T* operator->() { return &_ref->get(); }
        const T* operator->() const { return &_ref->get(); }

        void reset() { this->_ref.reset(); }

    private:
        std::optional<ref<T>> _ref;
    };

    template<typename T>
    using optcref = optref<const T>;
}
