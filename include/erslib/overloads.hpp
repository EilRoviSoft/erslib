#pragma once


namespace ers {
    template<class... Ts>
    struct overloads : Ts... {
        using Ts::operator()...;
    };
}
