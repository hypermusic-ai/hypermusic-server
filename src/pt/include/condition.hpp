#pragma once
#include <absl/hash/hash.h>

#include "condition.pb.h"

namespace hm
{
    /**
     * @brief Combines hash values for a Condition object.
     *
     * @tparam H The hash state type.
     * @param h The initial hash state.
     * @param c The Condition object whose attributes will be hashed.
     * @return A combined hash state incorporating the name and solution source of the Condition.
     */
    template <typename H>
    inline H AbslHashValue(H h, const Condition & c) {
        return H::combine(std::move(h), c.name(), c.sol_src());
    }
}