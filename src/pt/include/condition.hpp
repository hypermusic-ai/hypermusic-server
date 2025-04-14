#pragma once
#include <absl/hash/hash.h>

#include "condition.pb.h"

namespace hm
{
    template <typename H>
    inline H AbslHashValue(H h, const Condition & c) {
        return H::combine(std::move(h), c.name(), c.sol_src());
    }
}