#pragma once

// std
#include <exception>


namespace ers {
    class ParseError : public std::exception {
    };
}
