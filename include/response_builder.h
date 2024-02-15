#pragma once

#include <string>
#include <unordered_map>
#include <sstream>

class ResponseBuilder {
public:
    ResponseBuilder& setStatus(int status_code, const std::string& status_text);
    ResponseBuilder& setHeader(const std::string& key, const std::string& value);
    ResponseBuilder& setBody(const std::string& body);
    std::string build();

private:
    int status_code = 200;
    std::string status_text = "OK";
    std::unordered_map<std::string, std::string> headers;
    std::string body;
};