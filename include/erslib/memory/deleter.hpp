#pragma once

// std
#include <memory_resource>


namespace ers {
    template<class T>
    struct deleter {
        std::pmr::memory_resource* mr;

        deleter() = default;
        explicit deleter(std::pmr::memory_resource* mr) noexcept :
            mr(mr) {
        }

        void operator()(T* ptr) {
            if (!ptr)
                return;

            std::destroy_at(ptr);
            mr->deallocate(ptr, 1);
        }
    };
}
