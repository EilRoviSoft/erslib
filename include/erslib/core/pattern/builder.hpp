#pragma once


namespace ers::impl::pattern {
    template<typename T>
    class IBuilder {
    public:
        virtual ~IBuilder() = default;


        [[nodiscard]] virtual bool check() const noexcept {
            return true;
        }


        virtual T build() && = 0;
    };
}


// Exports

namespace ers {
    namespace pattern = impl::pattern;
}
