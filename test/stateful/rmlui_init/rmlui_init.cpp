#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN

// std
#include <filesystem>

// doctest
#include <doctest/doctest.h>

// Rml
#include <RmlUi/Core.h>


namespace {
    class TestSystemInterface final : public Rml::SystemInterface {
    public:
        double GetElapsedTime() override {
            return 0.0;
        }

        bool LogMessage(Rml::Log::Type, const Rml::String& message) override {
            return true;
        }
    };


    class NullRenderInterface final : public Rml::RenderInterface {
    public:
        Rml::CompiledGeometryHandle CompileGeometry(Rml::Span<const Rml::Vertex>, Rml::Span<const int>) override {
            return 1;
        }

        void RenderGeometry(Rml::CompiledGeometryHandle, Rml::Vector2f, Rml::TextureHandle) override {}

        void ReleaseGeometry(Rml::CompiledGeometryHandle) override {}

        Rml::TextureHandle LoadTexture(Rml::Vector2i& texture_dimensions, const Rml::String&) override {
            texture_dimensions = Rml::Vector2i(0, 0);
            return 0;
        }

        Rml::TextureHandle GenerateTexture(Rml::Span<const Rml::byte>, Rml::Vector2i) override {
            return 1;
        }

        void ReleaseTexture(Rml::TextureHandle) override {}

        void EnableScissorRegion(bool) override {}

        void SetScissorRegion(Rml::Rectanglei) override {}
    };
}


TEST_CASE("RmlUi initializes and creates context") {
    TestSystemInterface system_interface;
    NullRenderInterface render_interface;

    Rml::SetSystemInterface(&system_interface);
    Rml::SetRenderInterface(&render_interface);

    REQUIRE(Rml::Initialise());

    Rml::Context* context = Rml::CreateContext(
        "main",
        Rml::Vector2i(1280, 720)
    );

    REQUIRE(context != nullptr);

    Rml::RemoveContext("main");

    Rml::Shutdown();

    Rml::SetRenderInterface(nullptr);
    Rml::SetSystemInterface(nullptr);
}


TEST_CASE("RmlUi loads basic document") {
    TestSystemInterface system_interface;
    NullRenderInterface render_interface;

    Rml::SetSystemInterface(&system_interface);
    Rml::SetRenderInterface(&render_interface);

    REQUIRE(Rml::Initialise());

    REQUIRE(Rml::LoadFontFace(TEST_CWD "/res/Lato-Regular.ttf"));

    Rml::Context* context = Rml::CreateContext("main", Rml::Vector2i(1280, 720));

    REQUIRE(context != nullptr);

    Rml::ElementDocument* document = context->LoadDocument(TEST_CWD "/res/test.rml");

    REQUIRE(document != nullptr);

    document->Close();

    Rml::RemoveContext("main");
    Rml::Shutdown();

    Rml::SetRenderInterface(nullptr);
    Rml::SetSystemInterface(nullptr);
}
