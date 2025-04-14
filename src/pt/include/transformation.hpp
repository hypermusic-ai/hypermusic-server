#pragma once
#include <absl/hash/hash.h>

#include "transformation.pb.h"

namespace hm
{
    /**
    * @brief Combines hash values for a Transformation object.
    *
    * @tparam H The hash state type.
    * @param h The initial hash state.
    * @param t The Transformation object whose attributes will be hashed.
    * @return A combined hash state incorporating the name and source of the Transformation.
    */
    template <typename H>
    inline H AbslHashValue(H h, const Transformation& t) {
        return H::combine(std::move(h), t.name(), t.sol_src());
    }
}