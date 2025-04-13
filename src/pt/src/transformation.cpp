#include "transformation.hpp"

namespace hm{
    std::pair<hm::HTTPCode, std::string> GET_transformation(hm::SessionManager & session_mgr, const std::string & body)
    {
        return {hm::HTTPCode::OK, ""};
    }

    std::pair<hm::HTTPCode, std::string> POST_transformation(hm::SessionManager & session_mgr, const std::string & body)
    {
        return {hm::HTTPCode::OK, ""};
    }
}