#include "route.hpp"

namespace hm
{
    bool RouteKey::operator==(const RouteKey& other) const {
        return method == other.method && path == other.path;
    }
}