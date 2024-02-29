#pragma once

#include <string>
#include <unordered_map>
#include <sstream>
#include <iostream>
#include <vector>

struct HTTPRequest {
    std::string method;
    std::string path;
    std::string version;
    std::unordered_map<std::string, std::string> headers;
    std::string body;
    std::unordered_map<std::string, std::string> query_params;
};

class RequestParser {
public:
    static HTTPRequest parse(const std::string& raw_request);

private:
    static void parseQueryParams(HTTPRequest& request);
    static std::string urlDecode(const std::string& encoded);
};