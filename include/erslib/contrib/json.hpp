#pragma once


// Includes --------------------------------------------------------------------

#include <erslib/contrib/json/impl.hpp>


// Exports ---------------------------------------------------------------------

namespace utl {
    using impl::EFormat;
    using impl::ENodeType;

    using impl::TNode;
    using impl::Node;
    using Json = Node;

    using impl::default_traits;
    using impl::string_adaptor;

    using Object = Node::object_type;
    using Array = Node::array_type;
    using String = Node::string_type;
    using Integral = Node::integral_type;
    using Floating = Node::floating_type;
    using Bool = Node::bool_type;
    using Null = Node::null_type;
}
