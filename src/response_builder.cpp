#include "response_builder.h"

ResponseBuilder& ResponseBuilder::setStatus(int code, const std::string& text) {
    status_code = code;
    status_text = text;
    return *this;
}

ResponseBuilder& ResponseBuilder::setHeader(const std::string& key, const std::string& value) {
    headers[key] = value;
    return *this;
}

ResponseBuilder& ResponseBuilder::setBody(const std::string& new_body) {
    body = new_body;
    return *this;
}

std::string ResponseBuilder::build() {
    std::ostringstream oss;
    oss << "HTTP/1.1 " << status_code << " " << status_text << "\r\n";
    
    for (const auto& header : headers) {
        oss << header.first << ": " << header.second << "\r\n";
    }
    
    if (headers.find("Content-Length") == headers.end()) {
        oss << "Content-Length: " << body.length() << "\r\n";
    }
    
    oss << "\r\n";
    oss << body;
    
    return oss.str();
}

std::string ResponseBuilder::getStatusText(int status_code) {
    switch (status_code) {
        case 200: return "OK";
        case 201: return "Created";
        case 204: return "No Content";
        case 400: return "Bad Request";
        case 401: return "Unauthorized";
        case 403: return "Forbidden";
        case 404: return "Not Found";
        case 405: return "Method Not Allowed";
        case 500: return "Internal Server Error";
        default: return "Unknown Status";
    }
}