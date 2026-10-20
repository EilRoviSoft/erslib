#include "erslib/contrib/l10n.hpp"

// std
#include <string>
#include <utility>


// Utility

namespace {
    ers::Status flatten(const utl::Json& node, std::string& key, std::vector<ers::bundle_entry_t>& out) {
        if (node.is_string()) {
            out.push_back({ .key = key, .text = node.as_string() });
            return ers::ok;
        }


        if (node.is_array()) {
            std::string text;


            for (size_t i = 0; i < node.as_array().size(); i++) {
                const auto& line = node.as_array()[i];

                if (!line.is_string()) {
                    return ers::make_error("'{}[{}]' must be a string, not {}.",
                        key, i, utl::type_label(line.type()));
                }

                if (i > 0)
                    text += '\n';

                text += line.as_string();
            }


            out.push_back({ .key = key, .text = std::move(text) });
            return ers::ok;
        }

        if (node.is_object()) {
            for (const auto& [name, child] : node.as_object()) {
                if (name.empty() || name.contains('.')) {
                    return ers::make_error("'{}{}{}': a key can't be empty or hold '.', nest objects instead.",
                        key, key.empty() ? "" : ".", name);
                }


                const auto size = key.size();

                if (!key.empty())
                    key += '.';

                key += name;

                ERS_TRY(flatten(child, key, out));

                key.resize(size);
            }


            return ers::ok;
        }


        return ers::make_error("'{}' must be a string, an array of strings or an object, not {}.",
            key, utl::type_label(node.type()));
    }
}


// Loaders

ers::Result<std::vector<ers::bundle_entry_t>> utl::impl::flatten_l10n(const Json& bundle) {
    if (!bundle.is_object())
        return ers::make_error("A bundle must be an object, not {}.", type_label(bundle.type()));


    std::vector<ers::bundle_entry_t> out;
    std::string key;

    ERS_TRY(flatten(bundle, key, out));


    return out;
}

ers::bundle_loader_t utl::impl::l10n_json_loader() {
    return {
        .suffix = std::string(l10n_json_suffix),
        .parse  = [](std::string_view text) -> ers::Result<std::vector<ers::bundle_entry_t>> {
            auto bundle = try_from_string<Json>(text);
            ERS_TRY(bundle);

            return flatten_l10n(*bundle);
        },
    };
}
