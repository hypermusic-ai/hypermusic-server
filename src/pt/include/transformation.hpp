#pragma once
#include <absl/hash/hash.h>

#include "transformation.pb.h"

namespace hm
{
    template <typename H>
    inline H AbslHashValue(H h, const Transformation& t) {
        return H::combine(std::move(h), t.name(), t.sol_src());
    }
}