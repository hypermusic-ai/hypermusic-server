#pragma once

#include "native.h"
#include <asio.hpp>
#include <absl/container/flat_hash_map.h>

#include "feature.hpp"
#include "transformation.hpp"
#include "condition.hpp"

namespace hm
{
    class Registry
    {
        public:
            Registry() = delete;
            Registry(asio::io_context & io_context);
            ~Registry() = default;

            asio::awaitable<void> addFeature(Feature feature)
            {
                co_return;
            }
            asio::awaitable<void> addTransformation(Transformation transformation)
            {
                co_return;

            }
            asio::awaitable<void> addCondition(Condition condition)
            {
                co_return;
            }

            asio::awaitable<std::optional<Feature>> getFeature(const std::string& id)
            {
                return {};
            }
            //asio::awaitable<const Transformation &> getTransformation(const std::string& id);
            //asio::awaitable<const Condition &> getCondition(const std::string& id);

        private:
            asio::strand<asio::io_context::executor_type> _strand;

            absl::flat_hash_map<std::string, Feature> _features;
            absl::flat_hash_map<std::string, Transformation> _transformations;
            absl::flat_hash_map<std::string, Condition> _conditions;
    };
}