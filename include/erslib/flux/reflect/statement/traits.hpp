#pragma once

// std
#include <type_traits>

// ers
#include <erslib/flux/reflect/declaration.hpp>


namespace flux::impl::reflect {
    template<typename Tag>
    concept HasFragments = requires { typename Declaration<Tag>::Fragments; };

    struct no_fragments {};

    template<typename Tag>
    struct fragments_of {
        using type = no_fragments;
    };

    template<typename Tag>
        requires HasFragments<Tag>
    struct fragments_of<Tag> {
        using type = typename Declaration<Tag>::Fragments;
    };

    template<typename Tag>
    using fragments_t = typename fragments_of<Tag>::type;
}

namespace flux::impl::reflect {
    template<typename Tag>
    concept HasInput = requires { typename Declaration<Tag>::Input; };

    template<typename Tag>
    struct input_of {
        using type = void;
    };

    template<typename Tag>
        requires HasInput<Tag>
    struct input_of<Tag> {
        using type = typename Declaration<Tag>::Input;
    };

    template<typename Tag>
    using input_t = typename input_of<Tag>::type;


    template<typename T>
    struct wrapped_input {
        T value;
    };

    template<>
    struct wrapped_input<void> {};
}

namespace flux::impl::reflect {
    template<typename Tag>
    concept HasOutput = requires { typename Declaration<Tag>::Output; };

    template<typename Tag>
    struct output_of {
        using type = void;
    };

    template<typename Tag>
        requires HasOutput<Tag>
    struct output_of<Tag> {
        using type = typename Declaration<Tag>::Output;
    };

    template<typename Tag>
    using output_t = typename output_of<Tag>::type;
}
