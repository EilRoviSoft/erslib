#pragma once

// aescript
#include <aescript/impl/parser.hpp>


namespace aescript {
    template<typename Owner, typename Member>
    class ParseIntoParser : public IParser {
    public:
        // Member functions
        
        explicit ParseIntoParser(Member Owner::* member) :
            _member(member) {
        }


        // Parsers

        ers::Status exec(parser_context& ctx, sol::table table, std::string_view field, void* dst) const override {
            auto& owner = *static_cast<Owner*>(dst);
            parser_t<Member> type_parser;
            return type_parser.exec(ctx, table[field], owner.*_member);
        }


        // Misc

        ParserPtr clone() const override {
            return std::make_unique<ParseIntoParser>(_member);
        }


    private:
        Member Owner::* _member;
    };


    namespace parsers {
        template<typename Owner, typename Member>
        ParserPtr parse_into(Member Owner::* dst) {
            return std::make_unique<ParseIntoParser<Owner, Member>>(dst);
        }
    }
}
