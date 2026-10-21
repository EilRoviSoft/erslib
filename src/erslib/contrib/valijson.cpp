#include "erslib/contrib/valijson/validator.hpp"

// ers
#include <erslib/contrib/valijson/validator.inl>


// Member functions

utl::impl::JsonValidator::JsonValidator(std::shared_ptr<const valijson::Schema> schema) noexcept :
    _schema(std::move(schema)) {
}


// Instantiations --------------------------------------------------------------

template ers::Result<utl::impl::JsonValidator> utl::impl::JsonValidator::compile<utl::impl::default_traits>(const Json&);
template ers::Status utl::impl::JsonValidator::validate<utl::impl::default_traits>(const Json&) const;

template ers::Result<utl::impl::JsonValidator> utl::impl::JsonValidator::compile<utl::impl::ordered_traits>(const OrderedJson&);
template ers::Status utl::impl::JsonValidator::validate<utl::impl::ordered_traits>(const OrderedJson&) const;
