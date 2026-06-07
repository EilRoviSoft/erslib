// std
#include <cstdio>
#include <print>
#include <vector>
#include <filesystem>

// sdl3
#include <SDL3/SDL.h>

// Rml
#include <RmlUi/Core.h>
#include <RmlUi/Debugger.h>


namespace fs = std::filesystem;


// SDL3 lifetimes

namespace {
    class SdlLifetime {
    public:
        explicit SdlLifetime(SDL_InitFlags flags) {
            _ok = SDL_Init(flags);
        }

        ~SdlLifetime() {
            if (_ok)
                SDL_Quit();
        }

        [[nodiscard]] bool ok() const noexcept {
            return _ok;
        }


    private:
        bool _ok = false;
    };


    class SdlTextInputScope {
    public:
        explicit SdlTextInputScope(SDL_Window& window) :
            _window(window) {
            SDL_StartTextInput(&_window);
        }

        ~SdlTextInputScope() {
            SDL_StopTextInput(&_window);
        }

        SdlTextInputScope(const SdlTextInputScope&) = delete;
        SdlTextInputScope& operator=(const SdlTextInputScope&) = delete;


    private:
        SDL_Window& _window;
    };


    struct SdlWindowDeleter {
        void operator()(SDL_Window* ptr) const noexcept {
            if (ptr)
                SDL_DestroyWindow(ptr);
        }
    };
    using SdlWindowPtr = std::unique_ptr<SDL_Window, SdlWindowDeleter>;

    struct SdlRendererDeleter {
        void operator()(SDL_Renderer* ptr) const noexcept {
            if (ptr)
                SDL_DestroyRenderer(ptr);
        }
    };
    using SdlRendererPtr = std::unique_ptr<SDL_Renderer, SdlRendererDeleter>;

    struct SdlTextureDeleter {
        void operator()(SDL_Texture* ptr) const noexcept {
            if (ptr)
                SDL_DestroyTexture(ptr);
        }
    };
    using SdlTexturePtr = std::unique_ptr<SDL_Texture, SdlTextureDeleter>;
}


// RmlUi lifetimes

namespace {
    class RmlLifetime {
    public:
        RmlLifetime(Rml::SystemInterface& system_interface, Rml::RenderInterface& render_interface) {
            Rml::SetSystemInterface(&system_interface);
            Rml::SetRenderInterface(&render_interface);

            _ok = Rml::Initialise();
        }

        ~RmlLifetime() {
            if (_ok)
                Rml::Shutdown();

            Rml::SetRenderInterface(nullptr);
            Rml::SetSystemInterface(nullptr);
        }

        RmlLifetime(const RmlLifetime&) = delete;
        RmlLifetime& operator=(const RmlLifetime&) = delete;

        [[nodiscard]] bool ok() const noexcept {
            return _ok;
        }


    private:
        bool _ok = false;
    };


    class RmlContextHandle {
    public:
        RmlContextHandle(Rml::String name, Rml::Vector2i dimensions) :
            _name(std::move(name)) {
            _context = Rml::CreateContext(_name, dimensions);
        }

        ~RmlContextHandle() {
            if (_context) {
                Rml::RemoveContext(_name);
            }
        }

        RmlContextHandle(const RmlContextHandle&) = delete;
        RmlContextHandle& operator=(const RmlContextHandle&) = delete;

        [[nodiscard]] bool valid() const noexcept {
            return _context != nullptr;
        }

        [[nodiscard]] Rml::Context& get() const {
            return *_context;
        }


    private:
        Rml::String _name;
        Rml::Context* _context = nullptr;
    };


    class RmlDocumentHandle {
    public:
        RmlDocumentHandle() = default;

        explicit RmlDocumentHandle(Rml::ElementDocument& document) :
            _document(&document) {
        }

        ~RmlDocumentHandle() {
            if (_document)
                _document->Close();
        }

        RmlDocumentHandle(const RmlDocumentHandle&) = delete;
        RmlDocumentHandle& operator=(const RmlDocumentHandle&) = delete;

        RmlDocumentHandle(RmlDocumentHandle&& other) noexcept :
            _document(std::exchange(other._document, nullptr)) {
        }

        RmlDocumentHandle& operator=(RmlDocumentHandle&& other) noexcept {
            if (this != &other) {
                if (_document)
                    _document->Close();

                _document = std::exchange(other._document, nullptr);
            }

            return *this;
        }

        [[nodiscard]] bool valid() const noexcept {
            return _document != nullptr;
        }

        [[nodiscard]] Rml::ElementDocument& get() const {
            return *_document;
        }


    private:
        Rml::ElementDocument* _document = nullptr;
    };

    RmlDocumentHandle load_document(Rml::Context& context, const Rml::String& path) {
        Rml::ElementDocument* document = context.LoadDocument(path);

        if (!document)
            return {};

        return RmlDocumentHandle { *document };
    }

}


// Main implementation

namespace {
    class SdlRmlSystemInterface final : public Rml::SystemInterface {
    public:
        double GetElapsedTime() override {
            return static_cast<double>(SDL_GetTicks()) / 1000.0;
        }

        bool LogMessage(Rml::Log::Type type, const Rml::String& message) override {
            switch (type) {
                case Rml::Log::LT_ERROR:
                case Rml::Log::LT_ASSERT:
                case Rml::Log::LT_WARNING:
                    std::println(stderr, "[RmlUi] {}", message);
                    break;

                default:
                    break;
            }

            return true;
        }
    };

    class SdlRmlRenderInterface final : public Rml::RenderInterface {
    public:
        explicit SdlRmlRenderInterface(SDL_Renderer& renderer) :
            _renderer(renderer) {
        }

        Rml::CompiledGeometryHandle CompileGeometry(
            Rml::Span<const Rml::Vertex> vertices,
            Rml::Span<const int> indices
        ) override {
            const Rml::CompiledGeometryHandle handle = _next_geometry_handle++;

            Geometry geometry;
            geometry.vertices.reserve(vertices.size());

            for (const Rml::Vertex& vertex : vertices) {
                SDL_Vertex sdl_vertex {};

                sdl_vertex.position.x = vertex.position.x;
                sdl_vertex.position.y = vertex.position.y;

                sdl_vertex.color.r = static_cast<float>(vertex.colour.red) / 255.0f;
                sdl_vertex.color.g = static_cast<float>(vertex.colour.green) / 255.0f;
                sdl_vertex.color.b = static_cast<float>(vertex.colour.blue) / 255.0f;
                sdl_vertex.color.a = static_cast<float>(vertex.colour.alpha) / 255.0f;

                sdl_vertex.tex_coord.x = vertex.tex_coord.x;
                sdl_vertex.tex_coord.y = vertex.tex_coord.y;

                geometry.vertices.push_back(sdl_vertex);
            }

            geometry.indices.assign(indices.begin(), indices.end());

            _geometries.emplace(handle, std::move(geometry));

            return handle;
        }

        void RenderGeometry(
            Rml::CompiledGeometryHandle handle,
            Rml::Vector2f translation,
            Rml::TextureHandle texture
        ) override {
            auto geometry_it = _geometries.find(handle);

            if (geometry_it == _geometries.end())
                return;

            const Geometry& geometry = geometry_it->second;

            SDL_Texture* sdl_texture = nullptr;

            if (texture != 0) {
                auto texture_it = _textures.find(texture);

                if (texture_it != _textures.end()) {
                    sdl_texture = texture_it->second.get();
                }
            }

            _translated_vertices = geometry.vertices;

            for (SDL_Vertex& vertex : _translated_vertices) {
                vertex.position.x += translation.x;
                vertex.position.y += translation.y;
            }

            SDL_RenderGeometry(
                &_renderer,
                sdl_texture,
                _translated_vertices.data(),
                static_cast<int>(_translated_vertices.size()),
                geometry.indices.data(),
                static_cast<int>(geometry.indices.size())
            );
        }

        void ReleaseGeometry(Rml::CompiledGeometryHandle handle) override {
            _geometries.erase(handle);
        }

        Rml::TextureHandle LoadTexture(
            Rml::Vector2i& texture_dimensions,
            const Rml::String&
        ) override {
            // Image loading not implemented in this sandbox.
            // Font atlas textures are created through GenerateTexture().
            texture_dimensions = Rml::Vector2i(0, 0);
            return 0;
        }

        Rml::TextureHandle GenerateTexture(
            Rml::Span<const Rml::byte> source,
            Rml::Vector2i source_dimensions
        ) override {
            SdlTexturePtr texture {
                SDL_CreateTexture(
                    &_renderer,
                    SDL_PIXELFORMAT_RGBA8888,
                    SDL_TEXTUREACCESS_STATIC,
                    source_dimensions.x,
                    source_dimensions.y
                )
            };

            if (!texture) {
                std::println(stderr, "SDL_CreateTexture failed: {}", SDL_GetError());
                return 0;
            }

            SDL_SetTextureBlendMode(texture.get(), SDL_BLENDMODE_BLEND);

            const int pitch = source_dimensions.x * 4;

            if (!SDL_UpdateTexture(texture.get(), nullptr, source.data(), pitch)) {
                std::println(stderr, "SDL_UpdateTexture failed: {}", SDL_GetError());
                return 0;
            }

            const Rml::TextureHandle handle = _next_texture_handle++;

            _textures.emplace(handle, std::move(texture));

            return handle;
        }

        void ReleaseTexture(Rml::TextureHandle texture) override {
            _textures.erase(texture);
        }

        void EnableScissorRegion(bool enable) override {
            if (enable) {
                SDL_SetRenderClipRect(&_renderer, &_clip_rect);
            } else {
                SDL_SetRenderClipRect(&_renderer, nullptr);
            }
        }

        void SetScissorRegion(Rml::Rectanglei region) override {
            _clip_rect.x = region.Left();
            _clip_rect.y = region.Top();
            _clip_rect.w = region.Width();
            _clip_rect.h = region.Height();

            SDL_SetRenderClipRect(&_renderer, &_clip_rect);
        }


    private:
        struct Geometry {
            std::vector<SDL_Vertex> vertices;
            std::vector<int> indices;
        };

        SDL_Renderer& _renderer;

        SDL_Rect _clip_rect {};
        std::vector<SDL_Vertex> _translated_vertices;

        Rml::CompiledGeometryHandle _next_geometry_handle = 1;
        Rml::TextureHandle _next_texture_handle = 1;

        std::unordered_map<Rml::CompiledGeometryHandle, Geometry> _geometries;
        std::unordered_map<Rml::TextureHandle, SdlTexturePtr> _textures;
    };
}


// Input helpers

namespace {
    int get_key_modifier_state() {
        SDL_Keymod mod = SDL_GetModState();

        int rml_mod = 0;

        if (mod & SDL_KMOD_SHIFT)
            rml_mod |= Rml::Input::KM_SHIFT;

        if (mod & SDL_KMOD_CTRL)
            rml_mod |= Rml::Input::KM_CTRL;

        if (mod & SDL_KMOD_ALT)
            rml_mod |= Rml::Input::KM_ALT;

        if (mod & SDL_KMOD_GUI)
            rml_mod |= Rml::Input::KM_META;

        return rml_mod;
    }

    int sdl_mouse_button_to_rml(Uint8 button) {
        switch (button) {
            case SDL_BUTTON_LEFT:
                return 0;
            case SDL_BUTTON_RIGHT:
                return 1;
            case SDL_BUTTON_MIDDLE:
                return 2;
            default:
                return 3;
        }
    }

    class ButtonClickListener final : public Rml::EventListener {
    public:
        explicit ButtonClickListener(Rml::Element& status) :
            _status(status) {
        }

        void ProcessEvent(Rml::Event&) override {
            _status.SetInnerRML("Button clicked!");
        }


    private:
        Rml::Element& _status;
    };

    class RmlEventListenerHandle {
    public:
        RmlEventListenerHandle(
            Rml::Element& element,
            Rml::EventId event_id,
            std::unique_ptr<Rml::EventListener> listener
        ) :
            _element(&element),
            _event_id(event_id),
            _listener(std::move(listener)) {
            _element->AddEventListener(_event_id, _listener.get());
        }

        ~RmlEventListenerHandle() {
            if (_element && _listener)
                _element->RemoveEventListener(_event_id, _listener.get());
        }

        RmlEventListenerHandle(const RmlEventListenerHandle&) = delete;
        RmlEventListenerHandle& operator=(const RmlEventListenerHandle&) = delete;

        RmlEventListenerHandle(RmlEventListenerHandle&& other) noexcept :
            _element(std::exchange(other._element, nullptr)),
            _event_id(other._event_id),
            _listener(std::move(other._listener)) {
        }

        RmlEventListenerHandle& operator=(RmlEventListenerHandle&& other) noexcept {
            if (this != &other) {
                if (_element && _listener)
                    _element->RemoveEventListener(_event_id, _listener.get());

                _element = std::exchange(other._element, nullptr);
                _event_id = other._event_id;
                _listener = std::move(other._listener);
            }

            return *this;
        }


    private:
        Rml::Element* _element = nullptr;
        Rml::EventId _event_id {};
        std::unique_ptr<Rml::EventListener> _listener;
    };
}


int main(int, char**) {
    constexpr const char* font_path = TEST_CWD "/res/Lato-Regular.ttf";
    constexpr const char* document_path = TEST_CWD "/res/test.rml";


    SdlLifetime sdl { SDL_INIT_VIDEO | SDL_INIT_EVENTS };

    if (!sdl.ok()) {
        std::println(stderr, "SDL_Init failed: {}", SDL_GetError());
        return 1;
    }


    SdlWindowPtr window {
        SDL_CreateWindow(
            "RmlUi + SDL3 Sandbox",
            1280,
            720,
            SDL_WINDOW_RESIZABLE
        )
    };

    if (!window) {
        std::println(stderr, "SDL_CreateWindow failed: {}", SDL_GetError());
        return 1;
    }


    SdlRendererPtr renderer {
        SDL_CreateRenderer(window.get(), nullptr)
    };

    if (!renderer) {
        std::println(stderr, "SDL_CreateRenderer failed: {}", SDL_GetError());
        return 1;
    }


    SdlRmlSystemInterface system_interface;
    SdlRmlRenderInterface render_interface { *renderer };

    RmlLifetime rml { system_interface, render_interface };

    if (!rml.ok()) {
        std::println(stderr, "Rml::Initialise failed");
        return 1;
    }

    if (!fs::exists(font_path)) {
        std::println(stderr, "Missing font: {}", font_path);
        return 1;
    }

    if (!Rml::LoadFontFace(font_path)) {
        std::println(stderr, "Rml::LoadFontFace failed: {}", font_path);
        return 1;
    }


    int width = 1280;
    int height = 720;

    SDL_GetWindowSize(window.get(), &width, &height);

    RmlContextHandle context_handle {
        "main",
        Rml::Vector2i(width, height)
    };

    if (!context_handle.valid()) {
        std::println(stderr, "Rml::CreateContext failed");
        return 1;
    }


    Rml::Context& context = context_handle.get();

    Rml::Debugger::Initialise(&context);

    auto document = load_document(context, document_path);

    if (!document.valid()) {
        std::println(stderr, "Could not load {}", document_path);
        return 1;
    }

    document.get().Show();


    std::optional<RmlEventListenerHandle> click_listener;

    if (auto* button = document.get().GetElementById("button")) {
        if (auto* status = document.get().GetElementById("status")) {
            click_listener.emplace(
                *button,
                Rml::EventId::Click,
                std::make_unique<ButtonClickListener>(*status)
            );
        }
    }


    bool running = true;

    SdlTextInputScope text_input { *window };

    while (running) {
        SDL_Event event {};

        while (SDL_PollEvent(&event)) {
            const int modifiers = get_key_modifier_state();

            switch (event.type) {
                case SDL_EVENT_QUIT:
                    running = false;
                    break;

                case SDL_EVENT_KEY_DOWN:
                    if (event.key.key == SDLK_ESCAPE)
                        running = false;

                    if (event.key.key == SDLK_F8)
                        Rml::Debugger::SetVisible(!Rml::Debugger::IsVisible());

                    break;

                case SDL_EVENT_TEXT_INPUT:
                    context.ProcessTextInput(event.text.text);
                    break;

                case SDL_EVENT_MOUSE_MOTION:
                    context.ProcessMouseMove(
                        static_cast<int>(event.motion.x),
                        static_cast<int>(event.motion.y),
                        modifiers
                    );
                    break;

                case SDL_EVENT_MOUSE_BUTTON_DOWN:
                    context.ProcessMouseButtonDown(
                        sdl_mouse_button_to_rml(event.button.button),
                        modifiers
                    );
                    break;

                case SDL_EVENT_MOUSE_BUTTON_UP:
                    context.ProcessMouseButtonUp(
                        sdl_mouse_button_to_rml(event.button.button),
                        modifiers
                    );
                    break;

                case SDL_EVENT_MOUSE_WHEEL:
                    context.ProcessMouseWheel(
                        event.wheel.y,
                        modifiers
                    );
                    break;

                case SDL_EVENT_WINDOW_RESIZED:
                    context.SetDimensions({
                        event.window.data1,
                        event.window.data2
                    });
                    break;

                default:
                    break;
            }
        }

        context.Update();

        SDL_SetRenderDrawColor(renderer.get(), 10, 12, 18, 255);
        SDL_RenderClear(renderer.get());

        context.Render();

        SDL_RenderPresent(renderer.get());
    }

    return 0;
}
