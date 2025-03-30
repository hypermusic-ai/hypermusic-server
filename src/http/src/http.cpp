#include "http.hpp"

namespace hm
{
    HTTPRequest parseHTTPRequest(const std::string & request)
    {
        HTTPRequest http_request;

        std::istringstream request_stream(request);

        request_stream >> http_request.method >> http_request.path >> http_request.version;
        request_stream.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        std::string header_buffer;
        while (std::getline(request_stream, header_buffer) && header_buffer != "\r") 
        {            
            http_request.headers.emplace_back(header_buffer);
        }

        std::string body_buffer;
        while (std::getline(request_stream, body_buffer)) {
            http_request.body += body_buffer;
        }
        return http_request;
    }
}