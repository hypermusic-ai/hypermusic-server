#include "http.hpp"

namespace hm
{
    void HTTPBase::setVersion(const std::string & version) 
    { 
        _version = version; 
    }

    void HTTPBase::setBody(const std::string & body)
    {
        _body = body;

        setHeader(HTTPHeader::ContentLength, std::to_string(_body.size()));
    }

    void HTTPBase::addHeader(HTTPHeader header, const std::string & value)
    {
        _headers.emplace_back(std::make_pair(header, value));
    }

    void HTTPBase::setHeader(HTTPHeader header, const std::string & value)
    {
        auto result = std::ranges::find_if(_headers, [&](const auto & h) { return h.first == header; });
        if(result == _headers.end())
        {
            addHeader(header, value);
            return;
        }
        result->second = value; 
    }

    const std::string & HTTPBase::getVersion() const
    {
        return _version;
    }

    const HeadersList & HTTPBase::getHeaders() const
    {
        return _headers;
    }

    const std::string & HTTPBase::getBody() const
    {
        return _body;
    }

    void HTTPRequest::setMethod(const std::string & method)
    {
        _method = method;
    }

    void HTTPRequest::setPath(const std::string & path)
    {
        _path = path;
    }

    const std::string & HTTPRequest::getMethod() const
    {
        return _method;
    }

    const std::string & HTTPRequest::getPath() const
    {
        return _path;
    }

    void HTTPResponse::setCode(HTTPCode code)
    {
        _code = code;
    }

    const HTTPCode & HTTPResponse::getCode() const
    {
        return _code;
    }

    HTTPRequest parseHTTPRequest(const std::string & request)
    {
        HTTPRequest http_request;

        std::istringstream request_stream(request);

        std::string method_str;
        std::string path_str;
        std::string version_str;

        request_stream >> method_str >> path_str >> version_str;
        request_stream.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        http_request.setMethod(method_str);
        http_request.setPath(path_str);
        http_request.setVersion(version_str);

        std::string header_buffer;
        std::string header_key;
        std::string header_value; 
        while (std::getline(request_stream, header_buffer) && header_buffer != "\r") 
        {   
            const auto it = header_buffer.find(":");

            if (it != std::string::npos) 
            {
                header_key = header_buffer.substr(0, it);
                header_value = header_buffer.substr(it + 1);
            }
            else
            {
                header_key = header_buffer;
                header_value = "";
            }
            
            http_request.addHeader(headerFromString(header_key), header_value);

        }
        std::string body_line;
        std::string body_buffer;
        while (std::getline(request_stream, body_line)) {
            body_buffer += body_line;
        }

        http_request.setBody(body_buffer);

        return http_request;
    }
}