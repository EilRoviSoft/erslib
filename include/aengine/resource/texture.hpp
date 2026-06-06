#pragma once

// sf
#include <SFML/Graphics/Texture.hpp>

// aengine
#include <aengine/core/resource.hpp>


// TextureResource

namespace aengine {
    class TextureResource : public TResource<sf::Texture> {
    public:
        // Constructor

        explicit TextureResource(std::string_view path);


    protected:
        std::string m_path;


    private:
        std::function<ers::Status(control_block_type& cb)> cb_ctor();
    };


    template<>
    struct resource_type<sf::Texture> {
        using type = TextureResource;
    };
}
