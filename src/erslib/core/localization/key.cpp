#include "erslib/core/localization/key.hpp"


ers::Result<std::string> ers::impl::make_localization_key(const fs::path& relative) {
    auto extension = relative.extension().string();

    if (extension.size() < 2) {
        return make_error("'{}' has no extension to name it by.",
            relative.generic_string());
    }


    std::string out = extension.substr(1);

    for (const auto& part : relative.parent_path())
        out += '.' + part.string();

    out += '.' + relative.stem().string();


    return out;
}
