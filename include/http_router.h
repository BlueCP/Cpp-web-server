#pragma once

#include <optional>
#include <functional>

#include "request_parser.h"
#include "response_builder.h"

struct PairHash {
    size_t operator()(const std::pair<std::string, std::string> &p) const {
        return std::hash<std::string>{}(p.first) ^ std::hash<std::string>{}(p.second);
    }
};

class HTTPRouter {
public:
    HTTPRouter();
    std::optional<std::string> get_route(const HTTPRequest &request);
private:
    // add routes here
    // each route has the signature std::string(const HTTPRequest &request)
    std::string test_route(const HTTPRequest &request);
    void add_routes();

    std::unordered_map<std::pair<std::string, std::string>, std::function<std::string(const HTTPRequest&)>, PairHash> routes;
};