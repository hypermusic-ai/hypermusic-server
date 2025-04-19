#pragma once

#include <format>
#include <string>
#include <vector>
#include <sstream>

namespace hm::http
{
    class URL
    {
        public:
            URL() = default;
            URL(const std::string & url);
            URL(std::string && url);
            URL(const char * url);

            URL(const URL &) = default;
            URL(URL &&) = default;
            ~URL() = default;

            URL & operator=(const URL &) = default;
            URL & operator=(URL &&) = default;

            bool operator==(const URL& other) const;


            std::string getFullPath() const;
            std::string getPathModule() const;
            std::string getPathInfo() const;
            std::string getQuery() const;

        private:
            std::string _url;
    };

    /**
     * @brief Combines hash values for a RouteKey object.
     *
     * @tparam H The hash state type.
     * @param h The initial hash state.
     * @param url The URL object whose attributes will be hashed.
     * @return A combined hash state
     */
    template <typename H>
    inline H AbslHashValue(H h, const URL & url) {
        return H::combine(std::move(h), url.getFullPath());
    }

    std::vector<std::string> splitPathSegments(const std::string path);
}

template <>
struct std::formatter<hm::http::URL> : std::formatter<std::string> {
  auto format(const hm::http::URL & url, format_context& ctx) const {
    return formatter<string>::format(
      std::format("{}", url.getFullPath()), ctx);
  }
};