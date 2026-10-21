#pragma once

// std
#include <memory>

// valijson
#include <valijson/schema_parser.hpp>
#include <valijson/validator.hpp>

// ers
#include <erslib/contrib/json/impl.hpp>
#include <erslib/contrib/json/validation/adapter.hpp>
#include <erslib/core/type/result.hpp>


namespace valijson {
    class Schema;
}

namespace utl::impl {
    class ERSLIB_EXPORT JsonValidator {
    public:
        template<typename Traits>
        static ers::Result<JsonValidator> compile(const TNode<Traits>& schema) try {
            auto out = std::make_shared<valijson::Schema>();

            valijson::SchemaParser parser(valijson::SchemaParser::kDraft7);
            parser.populateSchema(valijson_adapter::Adapter<TNode<Traits>>(schema), *out);

            return JsonValidator(std::move(out));
        } catch (const std::exception& e) {
            return ers::make_error("Invalid JSON schema: {}", e.what());
        }

        template<typename Traits>
        [[nodiscard]] ers::Status validate(const TNode<Traits>& document) const {
            valijson::Validator validator;
            valijson::ValidationResults results;

            if (validator.validate(*_schema, valijson_adapter::Adapter<TNode<Traits>>(document), &results))
                return ers::ok;


            std::string message;
            valijson::ValidationResults::Error error;

            while (results.popError(error)) {
                if (!message.empty())
                    message += '\n';

                for (const auto& part : error.context)
                    message += part;

                message += ": " + error.description;
            }

            return ers::make_error("{}", message);
        }


    private:
        std::shared_ptr<const valijson::Schema> _schema;


        explicit JsonValidator(std::shared_ptr<const valijson::Schema> schema);
    };
}
