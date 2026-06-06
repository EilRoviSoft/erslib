#include "aengine/resource/texture.hpp"


//aengine::TextureResource::TextureResource(std::string_view path) :
//    TResource(cb_ctor()),
//    m_path(path) {
//}
//
//
//std::function<ers::Status(aengine::TResource<sf::Texture>::control_block_type& cb)> aengine::TextureResource::cb_ctor() {
//    return [this](control_block_type& cb) -> ers::Status {
//        std::scoped_lock lock(cb.mutex);
//        sf::Texture result;
//
//        if (!result.loadFromFile(m_path))
//            return ers::make_error(ers::Severity::Error, "File {} is not found", m_path);
//
//        cb.value = std::move(result);
//
//        return ers::ok;
//    };
//}
