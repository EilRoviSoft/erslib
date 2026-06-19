#pragma once

// std
#include <vector>

// aescript
#include <aescript/impl/parser.hpp>


namespace aescript {
    template<typename Owner, typename Element, typename Member>
    class ParseIntoWithIndex : public IParser {
    public:
        // Member functions

        ParseIntoWithIndex(std::vector<Element> Owner::* container, size_t index, Member Element::* member) :
            _container(container),
            _index(index),
            _member(member) {
        }


        // Parsers

        ers::Status exec(parser_context& ctx, sol::table table, std::string_view field, void* dst) const override {
            sol::object obj = table[field];
            if (!obj.valid())
                return ers::ok;

            auto& owner = *static_cast<Owner*>(dst);
            std::vector<Element>& container = owner.*_container;

            if (container.size() < _index + 1)
                container.resize(_index + 1);

            parser_t<Member> type_parser;
            return type_parser.exec(ctx, obj, container[_index].*_member);
        }


        // Misc

        ParserPtr clone() const override {
            return std::make_unique<ParseIntoWithIndex>(_container, _index, _member);
        }


    private:
        std::vector<Element> Owner::* _container;
        size_t _index;
        Member Element::* _member;
    };


    namespace properties {
        template<typename Owner, typename Element, typename Member>
        ParserPtr parse_into_with_index(std::vector<Element> Owner::* container, size_t index, Member Element::* member) {
            return std::make_unique<ParseIntoWithIndex<Owner, Element, Member>>(container, index, member);
        }

        template<typename Owner, typename Element, typename Member>
        ParserPtr parse_into_front(std::vector<Element> Owner::* container, Member Element::* member) {
            return parse_into_with_index<Owner, Element, Member>(container, 0, member);
        }
    }
}
