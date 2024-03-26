#include "http_method_handler.h"

HTTPMethodHandler::HTTPMethodHandler(FileCache& fc) : file_cache(fc) {}

std::string HTTPMethodHandler::handleRequest(const HTTPRequest& request) {
    if (auto result = router.get_route(request); result.has_value()) {
        return *result;
    }

    if (request.method == "GET") return handleGET(request);
    if (request.method == "POST") return handlePOST(request);
    if (request.method == "PUT") return handlePUT(request);
    if (request.method == "DELETE") return handleDELETE(request);
    if (request.method == "OPTIONS") return handleOPTIONS(request);
    if (request.method == "HEAD") return handleHEAD(request);

    ResponseBuilder rb;
    return rb.setStatus(405, "Method Not Allowed")
             .setHeader("Allow", "GET, POST, PUT, DELETE, OPTIONS, HEAD")
             .build();
}

std::string HTTPMethodHandler::handleGET(const HTTPRequest& request) {
    std::string path = "serve" + request.path;
    if (path == "serve/") {
        path = "serve/index.html";
    }

    ResponseBuilder rb;
    try {
        std::string content = file_cache.get(path);
        if (content.empty()) {
            content = readFile(path);
            file_cache.put(path, content);
        }
        std::string content_type = getContentType(path);
        
        return rb.setStatus(200, "OK")
                 .setHeader("Content-Type", content_type)
                 .setBody(content)
                 .build();
    } catch (const std::runtime_error& e) {
        return rb.setStatus(404, "Not Found")
                 .setHeader("Content-Type", "text/plain")
                 .setBody("404 Not Found")
                 .build();
    }
}

std::string HTTPMethodHandler::handlePOST(const HTTPRequest& request) {
    // TODO stub
    ResponseBuilder rb;
    return rb.setStatus(200, "OK")
             .setHeader("Content-Type", "text/plain")
             .setBody("Received POST data: " + request.body)
             .build();
}

std::string HTTPMethodHandler::handlePUT(const HTTPRequest& request) {
    // TODO stub
    ResponseBuilder rb;
    return rb.setStatus(200, "OK")
             .setHeader("Content-Type", "text/plain")
             .setBody("PUT request acknowledged for path: " + request.path)
             .build();
}

std::string HTTPMethodHandler::handleDELETE(const HTTPRequest& request) {
    // TODO stub
    ResponseBuilder rb;
    return rb.setStatus(200, "OK")
             .setHeader("Content-Type", "text/plain")
             .setBody("DELETE request acknowledged for path: " + request.path)
             .build();
}

std::string HTTPMethodHandler::handleOPTIONS(const HTTPRequest& request) {
    ResponseBuilder rb;
    return rb.setStatus(200, "OK")
             .setHeader("Allow", "GET, POST, PUT, DELETE, OPTIONS, HEAD")
             .setHeader("Content-Length", "0")
             .build();
}

std::string HTTPMethodHandler::handleHEAD(const HTTPRequest& request) {
    std::string get_response = handleGET(request);
    size_t header_end = get_response.find("\r\n\r\n");
    if (header_end != std::string::npos) {
        return get_response.substr(0, header_end + 4);
    }
    return get_response;
}

std::string HTTPMethodHandler::getContentType(const std::string& path) {
    std::string extension = std::filesystem::path(path).extension();
    if (extension == ".html" || extension == ".htm") return "text/html";
    if (extension == ".css") return "text/css";
    if (extension == ".js") return "application/javascript";
    if (extension == ".jpg" || extension == ".jpeg") return "image/jpeg";
    if (extension == ".png") return "image/png";
    if (extension == ".gif") return "image/gif";
    return "text/plain";
}

std::string HTTPMethodHandler::readFile(const std::string& path) {
    std::ifstream file(path, std::ios::binary);
    if (!file) {
        throw std::runtime_error("File not found");
    }
    return std::string((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
}