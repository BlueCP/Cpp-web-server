#pragma once

#include <string>
#include <unordered_map>
#include <sstream>
#include <iostream>

struct HTTPRequest {
    std::string method;
    std::string path;
    std::string version;
    std::unordered_map<std::string, std::string> headers;
};

class RequestParser {
public:
    static HTTPRequest parse(const std::string& raw_request);
};