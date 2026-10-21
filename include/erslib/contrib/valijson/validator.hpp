#pragma once

// std
#include <exception>
#include <memory>
#include <string>
#include <utility>

// ers
#include <erslib/contrib/json/impl.hpp>
#include <erslib/core/type/result.hpp>

// export
#include <erslib/export.hpp>


namespace valijson {
    class Schema;
}


namespace utl::impl {
    class ERSLIB_EXPORT JsonValidator {
    public:
        // Member functions

        template<typename Traits>
        static ers::Result<JsonValidator> compile(const TNode<Traits>& schema);


        // Validators

        template<typename Traits>
        [[nodiscard]] ers::Status validate(const TNode<Traits>& document) const;


    private:
        std::shared_ptr<const valijson::Schema> _schema;


        explicit JsonValidator(std::shared_ptr<const valijson::Schema> schema) noexcept;
    };


    extern template ers::Result<JsonValidator> JsonValidator::compile<default_traits>(const Json&);
    extern template ers::Status JsonValidator::validate<default_traits>(const Json&) const;

    extern template ers::Result<JsonValidator> JsonValidator::compile<ordered_traits>(const OrderedJson&);
    extern template ers::Status JsonValidator::validate<ordered_traits>(const OrderedJson&) const;
}
