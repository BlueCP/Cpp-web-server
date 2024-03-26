#include "http_router.h"

HTTPRouter::HTTPRouter() {
    add_routes();
}

std::optional<std::string> HTTPRouter::get_route(const HTTPRequest &request) {
    ResponseBuilder rb;
    if (routes.find({request.path, request.method}) != routes.end()) {
        return rb
            .setStatus(200, "OK")
            .setBody(routes[{request.path, request.method}](request))
            .build();
    } else {
        return {};
    }
}

std::string HTTPRouter::test_route(const HTTPRequest &request) {
    if (request.query_params.find("key") != request.query_params.end()) {
        return "key = " + request.query_params.at("key");
    } else {
        return "no key";
    }
}

void HTTPRouter::add_routes() {
    routes[{"/default", "GET"}] = [this](const HTTPRequest &request){ return test_route(request); };
}