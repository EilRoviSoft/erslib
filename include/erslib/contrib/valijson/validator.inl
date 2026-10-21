#pragma once

#if !VALIJSON_USE_EXCEPTIONS
#error "erslib/contrib/valijson needs VALIJSON_USE_EXCEPTIONS=1, without it a broken schema aborts the process"
#endif

// std
#include <exception>
#include <string>

// valijson
#include <valijson/schema.hpp>
#include <valijson/schema_parser.hpp>
#include <valijson/validation_results.hpp>
#include <valijson/validator.hpp>

// ers
#include <erslib/contrib/valijson/adapter.hpp>
#include <erslib/contrib/valijson/validator.hpp>


// Implementation --------------------------------------------------------------

template<typename Traits>
ers::Result<utl::impl::JsonValidator> utl::impl::JsonValidator::compile(const TNode<Traits>& schema) try {
    auto out = std::make_shared<valijson::Schema>();

    valijson::SchemaParser parser(valijson::SchemaParser::kDraft7);
    parser.populateSchema(valijson_adapter::Adapter<TNode<Traits>>(schema), *out);

    return JsonValidator(std::move(out));
}
catch (const std::exception& e) {
    return ers::make_error("Invalid JSON schema: {}", e.what());
}


// TODO: a validator per call drops valijson's regex cache, so every 'pattern' is compiled again; keep one per thread if schemas with patterns get hot
template<typename Traits>
ers::Status utl::impl::JsonValidator::validate(const TNode<Traits>& document) const try {
    valijson::Validator validator;
    valijson::ValidationResults results;

    if (validator.validate(*_schema, valijson_adapter::Adapter<TNode<Traits>>(document), &results))
        return ers::ok;


    std::string message;
    valijson::ValidationResults::Error error;

    while (results.popError(error)) {
        if (!message.empty())
            message += '\n';

        message += error.jsonPointer.empty() ? "/" : error.jsonPointer;
        message += ": ";
        message += error.description;
    }

    return ers::make_error("{}", message);
}
catch (const std::exception& e) {
    return ers::make_error("JSON validation failed: {}", e.what());
}