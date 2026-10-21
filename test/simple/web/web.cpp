// doctest
#include <doctest/doctest.h>

// std
#include <chrono>
#include <string>
#include <utility>

// ers
#include <erslib/web.hpp>
#include <erslib/web/middleware/cors.hpp>
#include <erslib/web/middleware/json_body.hpp>
#include <erslib/web/middleware/rate_limit.hpp>


using std::string_literals::operator ""s;
using namespace std::chrono_literals;


// Details ---------------------------------------------------------------------

namespace {
    struct no_context {};

    struct by_address {
        static constexpr bool reset_on_success = true;

        template<typename AllContext>
        static std::string key(const crow::request& req, AllContext&) {
            return req.remote_ip_address;
        }
    };


    crow::request make_request(std::string body = {}) {
        crow::request req;
        req.body = std::move(body);
        req.remote_ip_address = "127.0.0.1";

        return req;
    }
}


// Implementation --------------------------------------------------------------

TEST_CASE("web: responses") {
    auto res = ers::web::make_failure(crow::BAD_REQUEST, "bad");

    REQUIRE(res.code == 400);
    REQUIRE(res.get_header_value("Content-Type") == "application/json");
    REQUIRE(utl::from_string(res.body)["error"].as_string() == "bad");

    REQUIRE(utl::from_string(ers::web::make_ok().body)["ok"].as_bool());
}

TEST_CASE("web: request helpers") {
    auto req = make_request();
    req.add_header("Authorization", "Bearer abc");
    req.add_header("User-Agent", "a\x01" "b");

    REQUIRE(ers::web::bearer_token(req) == "abc");
    REQUIRE(ers::web::read_header(req, "User-Agent") == "ab"s);
    REQUIRE(!ers::web::read_header(req, "Missing"));

    auto qs = crow::query_string("?limit=500&offset=x&flag=true");

    REQUIRE(ers::web::query_number<u32>(qs, "limit", 20, 1, 100) == 100);
    REQUIRE(ers::web::query_number<u32>(qs, "offset", 7, 0, 100) == 7);
    REQUIRE(ers::web::query_number<u32>(qs, "missing", 20, 1, 100) == 20);
    REQUIRE(ers::web::query_flag(qs, "flag"));
    REQUIRE(!ers::web::query_flag(qs, "limit"));
}

TEST_CASE("web: cors") {
    ers::web::Cors cors;
    ers::web::Cors::context ctx;

    cors.configure({ .origins = { "http://a.test" } });

    auto allowed = make_request();
    allowed.add_header("Origin", "http://a.test");
    crow::response allowed_res;
    cors.after_handle(allowed, allowed_res, ctx);
    REQUIRE(allowed_res.get_header_value("Access-Control-Allow-Origin") == "http://a.test");
    REQUIRE(allowed_res.get_header_value("Access-Control-Max-Age") == "86400");

    auto foreign = make_request();
    foreign.add_header("Origin", "http://b.test");
    crow::response foreign_res;
    cors.after_handle(foreign, foreign_res, ctx);
    REQUIRE(foreign_res.get_header_value("Access-Control-Allow-Origin").empty());
}

TEST_CASE("web: json body") {
    ers::web::JsonBody json_body;

    ers::web::JsonBody::context good_ctx;
    auto good = make_request(R"({"a": 1})");
    crow::response good_res;
    json_body.before_handle(good, good_res, good_ctx);
    REQUIRE(!good_res.is_completed());
    REQUIRE(utl::read_as<i64>(*good_ctx.body, "a") == 1);

    ers::web::JsonBody::context bad_ctx;
    auto bad = make_request("{");
    crow::response bad_res;
    json_body.before_handle(bad, bad_res, bad_ctx);
    REQUIRE(bad_res.code == 400);
    REQUIRE(!bad_ctx.body);
}

TEST_CASE("web: keyed rate limit") {
    ers::web::KeyedRateLimit<by_address> limiter;
    ers::web::KeyedRateLimit<by_address>::context ctx;
    no_context all;

    auto unconfigured = make_request();
    crow::response unconfigured_res;
    limiter.before_handle(unconfigured, unconfigured_res, ctx, all);
    REQUIRE(unconfigured_res.code == 500);

    limiter.configure(1, 1min, 16);

    auto first = make_request();
    crow::response first_res;
    limiter.before_handle(first, first_res, ctx, all);
    REQUIRE(!first_res.is_completed());

    auto second = make_request();
    crow::response second_res;
    limiter.before_handle(second, second_res, ctx, all);
    REQUIRE(second_res.code == 429);
    REQUIRE(!second_res.get_header_value("Retry-After").empty());

    crow::response ok_res;
    limiter.after_handle(first, ok_res, ctx);

    auto third = make_request();
    crow::response third_res;
    limiter.before_handle(third, third_res, ctx, all);
    REQUIRE(!third_res.is_completed());
}
