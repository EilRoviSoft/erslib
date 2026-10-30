#pragma once

// std
#include <atomic>
#include <future>


namespace test {
    class Gate {
    public:
        // Member functions

        Gate() :
            _future(_promise.get_future().share()) {
        }

        Gate(const Gate&) = delete;
        Gate& operator=(const Gate&) = delete;

        ~Gate() {
            open();
        }


        // Accessors

        std::shared_future<void> future() const {
            return _future;
        }


        // Modifiers

        void open() {
            if (!_opened.exchange(true))
                _promise.set_value();
        }


    private:
        std::promise<void> _promise;
        std::shared_future<void> _future;
        std::atomic_bool _opened = false;
    };
}
