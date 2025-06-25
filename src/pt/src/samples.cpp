#include "samples.hpp"

namespace dcn::parse
{
    std::optional<json> parseToJson(std::vector<Samples> samples, use_json_t) 
    {
        json arr = json::array();

        for (const auto& s : samples) {
            json obj;
            obj["feature_path"] = s.feature_path();
            obj["data"] = s.data();
            arr.push_back(obj);
        }

        return arr;
    }

    template<>
    std::optional<std::vector<Samples>> parseFromJson(json json_val, use_json_t) 
    {
        if (!json_val.is_array())
            return std::nullopt;

        std::vector<Samples> result;

        for (const auto& item : json_val) {
            if (!item.contains("feature_path") || !item.contains("data"))
                return std::nullopt;

            Samples s;
            s.set_feature_path(item["feature_path"].get<std::string>());

            for (const auto& val : item["data"]) {
                s.add_data(val.get<std::uint32_t>());
            }

            result.push_back(std::move(s));
        }

        return result;
    }

}