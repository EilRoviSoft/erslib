#pragma once

// std
#include <atomic>
#include <memory>
#include <utility>


namespace ers::impl::thread_safe {
    // Holds an immutable value which readers take as a whole and writers replace as a whole.
    // A snapshot taken by a reader stays alive until it's dropped, even if the value was replaced
    template<typename T>
    class Snapshot {
    public:
        using value_type = T;
        using pointer = std::shared_ptr<const T>;


        // Member functions

        Snapshot() noexcept :
            _value(pointer()) {
        }

        explicit Snapshot(T value) :
            _value(std::make_shared<const T>(std::move(value))) {
        }


        // Accessors

        [[nodiscard]]
        pointer get() const noexcept {
            return _value.load(std::memory_order_acquire);
        }

        [[nodiscard]]
        explicit operator bool() const noexcept {
            return get() != nullptr;
        }


        // Modifiers

        void replace(T value) {
            replace(std::make_shared<const T>(std::move(value)));
        }

        void replace(pointer value) noexcept {
            _value.store(std::move(value), std::memory_order_release);
        }


    private:
        std::atomic<pointer> _value;
    };
}


// Exports ---------------------------------------------------------------------

namespace ers::thread_safe {
    using impl::thread_safe::Snapshot;
}
