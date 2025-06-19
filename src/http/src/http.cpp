#include "http.hpp"

namespace dcn::http
{
    MessageBase::MessageBase()
    {
        setHeader(Header::ContentLength, "0");
    }

    void MessageBase::setVersion(const std::string & version) 
    { 
        _version = version; 
    }

    void MessageBase::setBody(const std::string & body)
    {
        _body = body;

        setHeader(Header::ContentLength, std::to_string(_body.size()));
    }

    void MessageBase::addHeader(Header header, const std::string & value)
    {
        _headers.emplace_back(std::make_pair(header, value));
    }

    void MessageBase::setHeader(Header header, const std::string & value)
    {
        auto result = std::ranges::find_if(_headers, [&](const auto & h) { return h.first == header; });
        if(result == _headers.end())
        {
            addHeader(header, value);
            return;
        }
        result->second = value; 
    }

    std::vector<std::string> MessageBase::getHeader(Header header) const
    {
        std::vector<std::string> values;
        for(const auto & h : _headers)
        {
            if(h.first == header)
            {
                values.push_back(h.second);
            }
        }

        return values;
    }


    const std::string & MessageBase::getVersion() const
    {
        return _version;
    }

    const HeadersList & MessageBase::getHeaders() const
    {
        return _headers;
    }

    const std::string & MessageBase::getBody() const
    {
        return _body;
    }

    void Request::setMethod(const Method & method)
    {
        _method = method;
    }

    void Request::setPath(URL path)
    {
        _path = std::move(path);
    }

    const Method & Request::getMethod() const
    {
        return _method;
    }

    const URL & Request::getPath() const
    {
        return _path;
    }

    void Response::setCode(Code code)
    {
        _code = code;
    }

    const Code & Response::getCode() const
    {
        return _code;
    }
}

namespace dcn::parse
{
    http::Request parseRequestFromString(const std::string & request)
    {
        http::Request http_request;

        std::istringstream request_stream(request);

        std::string method_str;
        std::string path_str;
        std::string version_str;

        request_stream >> method_str >> path_str >> version_str;
        request_stream.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        http_request.setMethod(parse::parseMethodFromString(method_str));
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
            
            http_request.addHeader(parse::parseHeaderFromString(header_key), header_value);

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