#pragma once

namespace ers {
    template<typename T>
    concept CallableConcept = requires { &T::operator(); };
}
