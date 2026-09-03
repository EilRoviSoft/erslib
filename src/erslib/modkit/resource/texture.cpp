#include "erslib/modkit/resource/texture.hpp"


//ers::modkit::TextureResource::TextureResource(std::string_view path) :
//    TResource(cb_ctor()),
//    m_path(path) {
//}
//
//
//std::function<ers::Status(ers::modkit::TResource<sf::Texture>::control_block_type& cb)> ers::modkit::TextureResource::cb_ctor() {
//    return [this](control_block_type& cb) -> ers::Status {
//        std::scoped_lock lock(cb.mutex);
//        sf::Texture result;
//
//        if (!result.loadFromFile(m_path))
//            return make_error(ESeverity::Error, "File {} is not found", m_path);
//
//        cb.value = std::move(result);
//
//        return ok;
//    };
//}
