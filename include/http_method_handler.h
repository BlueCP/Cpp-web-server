#pragma once

#include <filesystem>
#include <fstream>

#include "request_parser.h"
#include "response_builder.h"
#include "file_cache.h"
#include "http_router.h"

class HTTPMethodHandler {
public:
    HTTPMethodHandler(FileCache& file_cache);
    std::string handleRequest(const HTTPRequest& request);

private:
    std::string handleGET(const HTTPRequest& request);
    std::string handlePOST(const HTTPRequest& request);
    std::string handlePUT(const HTTPRequest& request);
    std::string handleDELETE(const HTTPRequest& request);
    std::string handleOPTIONS(const HTTPRequest& request);
    std::string handleHEAD(const HTTPRequest& request);

    std::string getContentType(const std::string& path);
    std::string readFile(const std::string& path);

    FileCache& file_cache;
    HTTPRouter router;
};