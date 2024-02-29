#include "request_parser.h"

HTTPRequest RequestParser::parse(const std::string& raw_request) {
    HTTPRequest request;
    std::istringstream iss(raw_request);
    
    // Parse request line
    std::string request_line;
    std::getline(iss, request_line);
    std::istringstream request_line_stream(request_line);
    request_line_stream >> request.method >> request.path >> request.version;
    
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
    
    // Parse body if present
    if (request.headers.find("Content-Length") != request.headers.end()) {
        int content_length = std::stoi(request.headers["Content-Length"]);
        std::vector<char> body_buffer(content_length);
        iss.read(body_buffer.data(), content_length);
        request.body = std::string(body_buffer.begin(), body_buffer.end());
    }

    // Parse query parameters
    parseQueryParams(request);
    
    return request;
}

void RequestParser::parseQueryParams(HTTPRequest& request) {
    size_t query_start = request.path.find('?');
    if (query_start != std::string::npos) {
        std::string query_string = request.path.substr(query_start + 1);
        request.path = request.path.substr(0, query_start);
        
        std::istringstream query_stream(query_string);
        std::string pair;
        while (std::getline(query_stream, pair, '&')) {
            size_t eq_pos = pair.find('=');
            if (eq_pos != std::string::npos) {
                std::string key = urlDecode(pair.substr(0, eq_pos));
                std::string value = urlDecode(pair.substr(eq_pos + 1));
                request.query_params[key] = value;
            }
        }
    }
}

std::string RequestParser::urlDecode(const std::string& encoded) {
    std::string decoded;
    for (size_t i = 0; i < encoded.length(); ++i) {
        if (encoded[i] == '%' && i + 2 < encoded.length()) {
            int value;
            std::istringstream is(encoded.substr(i + 1, 2));
            if (is >> std::hex >> value) {
                decoded += static_cast<char>(value);
                i += 2;
            } else {
                decoded += encoded[i];
            }
        } else if (encoded[i] == '+') {
            decoded += ' ';
        } else {
            decoded += encoded[i];
        }
    }
    return decoded;
}