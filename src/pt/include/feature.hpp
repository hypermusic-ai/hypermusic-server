#pragma once
#include <absl/hash/hash.h>

#include "feature.pb.h"

namespace hm
{
    /**
    * @brief Combines hash values for a Dimension object.
    *
    * @tparam H The hash state type.
    * @param h The initial hash state.
    * @param d The Dimension object whose attributes will be hashed.
    * @return A combined hash state incorporating the feature name and all transformation names of the Dimension.
    */
    template <typename H>
    inline H AbslHashValue(H h, const Dimension& d) 
    {
        h = H::combine(std::move(h), d.feature_name());
        for (const std::string & t : d.transformation_name()) {
            h = H::combine(std::move(h), t);
        }
        return h;
    }

    /**
    * @brief Combines hash values for a Feature object.
    *
    * @tparam H The hash state type.
    * @param h The initial hash state.
    * @param f The Feature object whose attributes will be hashed.
    * @return A combined hash state incorporating the name and dimensions of the Feature.
    */
    template <typename H>
    inline H AbslHashValue(H h, const Feature& f) {
        h = H::combine(std::move(h), f.name());
        for (const Dimension & d : f.dimensions()) {
            h = H::combine(std::move(h), d);
        }
        return h;
    }
}