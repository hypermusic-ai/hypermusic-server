#include "condition.hpp"

namespace hm
{
    std::pair<hm::HTTPCode, std::string> GET_condition(hm::SessionManager & session_mgr, const std::string & body)
    {
        return {hm::HTTPCode::OK, ""};
    }

    std::pair<hm::HTTPCode, std::string> POST_condition(hm::SessionManager & session_mgr, const std::string & body)
    {
        return {hm::HTTPCode::OK, ""};
    }
}