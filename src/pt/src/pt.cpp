#include "pt.hpp"

namespace hm
{
    std::string escapeSolSrcQuotes(const std::string& json)
    {
        std::string output;
        size_t pos = 0;

        // Find start of "sol_src"
        size_t key_start = json.find("\"sol_src\"");
        if (key_start == std::string::npos) return json;

        // Copy everything up to the start of value
        size_t colon_pos = json.find(':', key_start);
        if (colon_pos == std::string::npos) return json;

        size_t quote_open = json.find('"', colon_pos);
        if (quote_open == std::string::npos) return json;

        std::size_t quote_close = json.find_last_of('"');
        if (quote_close == std::string::npos) return json;

        output = json.substr(0, quote_open + 1); // includes opening quote

        // Now escape contents of the sol_src string
        ++quote_open;
        for (size_t i = quote_open; i < quote_close; ++i)
        {
            const char & c = json[i];

            if (c == '"')
            {
                output += "\\\""; // escape quote
            } 
            else
            {
                output += c;
            }
        }

        return output + "\"}";
    }

    std::vector<uint8_t> constructFunctionSelector(std::string signature)
    {
        uint8_t hash[32];
        Keccak256::getHash(reinterpret_cast<const uint8_t*>(signature.data()), signature.size(), hash);
        return std::vector<uint8_t>(hash, hash + 32);
    }
}