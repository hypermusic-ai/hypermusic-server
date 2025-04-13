#pragma once

#include <spdlog/spdlog.h>
#include <google/protobuf/util/json_util.h>

#include "condition.pb.h"
#include "session.hpp"
#include "http.hpp"

namespace hm
{
    std::pair<hm::HTTPCode, std::string> GET_condition(hm::SessionManager & session_mgr, const std::string & body);

    std::pair<hm::HTTPCode, std::string> POST_condition(hm::SessionManager & session_mgr, const std::string & body);
}