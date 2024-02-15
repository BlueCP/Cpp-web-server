#include "request_parser.h"

HTTPRequest RequestParser::parse(const std::string& raw_request) {
    HTTPRequest request;
    std::istringstream iss(raw_request);
    
    // Parse request line
    iss >> request.method >> request.path >> request.version;
    
    // Parse headers
    std::string line;
    while (std::getline(iss, line) && line != "\r") {
        auto colon_pos = line.find(':');
        if (colon_pos != std::string::npos) {
            std::string key = line.substr(0, colon_pos);
            std::string value = line.substr(colon_pos + 1);
            // Trim leading and trailing whitespace
            value.erase(0, value.find_first_not_of(" \t"));
            value.erase(value.find_last_not_of(" \t\r\n") + 1);
            request.headers[key] = value;
        }
    }
    
    return request;
}