#pragma once

// std
#include <string>
#include <string_view>
#include <vector>

// ers
#include <erslib/luaschema/impl/descriptor.hpp>

// export
#include <erslib/export.hpp>


namespace ers::luaschema::impl {
    class ERSLIB_EXPORT OneOfProperty : public IDescriptor {
    public:
        // Member functions

        explicit OneOfProperty(std::vector<std::string> names);


        // Executors

        Status verify(sol::table table) const override;


    private:
        std::vector<std::string> _names;
    };


    namespace properties {
        ERSLIB_EXPORT Descriptor one_of(std::initializer_list<std::string_view> il);
    }
}
