#include "aengine/core.hpp"


aengine::Core::Core(fs::path cwd) :
    m_cwd(std::move(cwd)) {
}


aengine::Context::Context(Core& value) :
    TProxy(value) {
}
