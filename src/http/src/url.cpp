#include "url.hpp"

namespace dcn::http
{
    URL::URL(const std::string & url) 
    : _url(url)
    {
    }

    URL::URL(std::string && url)
    : _url(std::move(url))
    {

    }

    URL::URL(const char * url)
    : URL(std::string(url))
    {
    }

    bool URL::operator==(const URL& other) const
    {
        return _url == other._url;
    }

    std::string URL::getFullPath() const
    {
        return _url.substr(0, _url.find('?'));
    }

    std::string URL::getQuery() const
    {
        size_t start = _url.find('?');
        if (start != std::string::npos)
        {
            size_t end = _url.find('#', start);
            return _url.substr(start + 1, end - start - 1);
        }
        return "";
    }

    std::string URL::getPathModule() const
    {
        size_t start = _url.find('/');
        size_t end = _url.find('/', start + 1);

        if(start == std::string::npos)return "";

        return _url.substr(start, end - start);
    }

    std::string URL::getPathInfo() const
    {
        size_t start = _url.find('/', _url.find('/') + 1);
        size_t end = _url.find('?');
        
        if(start == std::string::npos)return "";

        return _url.substr(start, end - start);
    }
}

namespace dcn::http
{
    std::vector<std::string> splitPathSegments(const std::string path)
    {
        std::vector<std::string> path_parts;
        if(path.empty())return path_parts;

        static const char path_delimeter = '/';

        std::stringstream ss(path);
        std::string segment;

        // first segment must be empty to ensure that path starts with /
        if(!std::getline(ss, segment, path_delimeter) || !segment.empty())return path_parts;

        while (std::getline(ss, segment, path_delimeter)) 
        {
            path_parts.push_back(segment);
        }

        return path_parts;
    }
}

