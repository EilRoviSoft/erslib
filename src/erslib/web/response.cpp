#include "erslib/web/response.hpp"

// std
#include <string>


// Details ---------------------------------------------------------------------

namespace {
    void write_json(crow::response& res, crow::status code, const utl::Json& body) {
        res.code = code;
        res.body = body.to_string(utl::EFormat::Minimized);
        res.set_header("Content-Type", "application/json");
    }
}


// Implementation --------------------------------------------------------------

utl::Json ers::web::impl::failure_json(std::string_view message) {
    utl::Json out;

    out["error"] = std::string(message);

    return out;
}

crow::response ers::web::impl::make_response(crow::status code, const utl::Json& body) {
    crow::response res;
    write_json(res, code, body);

    return res;
}

crow::response ers::web::impl::make_failure(crow::status code, std::string_view message) {
    return make_response(code, failure_json(message));
}

crow::response ers::web::impl::make_ok() {
    utl::Json body;

    body["ok"] = true;

    return make_response(crow::OK, body);
}

void ers::web::impl::end_with_json(crow::response& res, crow::status code, const utl::Json& body) {
    write_json(res, code, body);
    res.end();
}

void ers::web::impl::end_with_failure(crow::response& res, crow::status code, std::string_view message) {
    end_with_json(res, code, failure_json(message));
}
