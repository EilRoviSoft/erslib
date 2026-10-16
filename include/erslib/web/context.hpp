// std
#include <concepts>


namespace ers::web::impl {
    template<typename AllContext, typename... Middlewares>
    concept ContextWith = (std::derived_from<AllContext, typename Middlewares::context> && ...);
}


namespace ers::web {
    using impl::ContextWith;
}
