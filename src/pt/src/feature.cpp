#include "feature.hpp"


namespace hm
{
    std::pair<hm::HTTPCode, std::string> GET_feature(hm::SessionManager & session_mgr, const std::string & body)
    {
        // fetch from ???
        return {hm::HTTPCode::OK, ""};
    }

    std::pair<hm::HTTPCode, std::string> POST_feature(hm::SessionManager & session_mgr, const std::string & body)
    {
        // parse feature 
        hm::Feature feature;
        google::protobuf::util::JsonParseOptions options;
        auto status = google::protobuf::util::JsonStringToMessage(body, &feature, options);
        if (!status.ok()) 
        {
            return std::make_pair(hm::HTTPCode::INTERNAL_SERVER_ERROR, std::format("Failed to parse feature : {}", status.message()));
        }

        spdlog::debug("Parsed feature {}", feature.DebugString());        

        // add to local registry

        // add to EVM machine

        // return debug string
        return std::make_pair(hm::HTTPCode::OK, std::format("feature `{}` added", feature.name()));
    }
}