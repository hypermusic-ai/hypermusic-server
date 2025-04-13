#pragma once
#include <absl/hash/hash.h>

#include "feature.pb.h"

namespace hm
{
    template <typename H>
    inline H AbslHashValue(H h, const Dimension& d) 
    {
        h = H::combine(std::move(h), d.feature_name());
        for (const std::string & t : d.transformation_name()) {
            h = H::combine(std::move(h), t);
        }
        return h;
    }

    template <typename H>
    inline H AbslHashValue(H h, const Feature& f) {
        h = H::combine(std::move(h), f.name());
        for (const Dimension & d : f.dimensions()) {
            h = H::combine(std::move(h), d);
        }
        return h;
    }
}