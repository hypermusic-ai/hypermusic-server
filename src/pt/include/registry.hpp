#pragma once

#include <optional>
#include <string>

#include "native.h"
#include <asio.hpp>
#include <absl/container/flat_hash_map.h>
#include <spdlog/spdlog.h>


#include "feature.hpp"

#include "condition.pb.h"
#include "transformation.pb.h"

namespace hm
{
    class Registry
    {
        public:
            Registry() = delete;
            Registry(asio::io_context & io_context);
            ~Registry() = default;

            asio::awaitable<std::optional<std::size_t>> addFeature(Feature feature);
            asio::awaitable<std::optional<Feature>> getNewestFeature(const std::string& name) const;
            asio::awaitable<std::optional<Feature>> getFeature(const std::string& name, std::size_t version) const;


            // asio::awaitable<void> addTransformation(Transformation transformation)
            // {
            //     co_return;

            // }
            // asio::awaitable<void> addCondition(Condition condition)
            // {
            //     co_return;
            // }


            //asio::awaitable<const Transformation &> getTransformation(const std::string& id);
            //asio::awaitable<const Condition &> getCondition(const std::string& id);
        protected:
            bool containsFeatureBucket(const std::string& name) const;
            bool isFeatureBucketEmpty(const std::string& name) const ;

            asio::awaitable<bool> checkIfSubFeaturesExist(const Feature & feature) const;

        private:
            asio::strand<asio::io_context::executor_type> _strand;

            absl::flat_hash_map<std::string, std::size_t> _newest_feature;
            absl::flat_hash_map<std::string, std::size_t> _newest_transformation;
            absl::flat_hash_map<std::string, std::size_t> _newest_condition;

            absl::flat_hash_map<std::string, absl::flat_hash_map<std::size_t, Feature>> _features;
            absl::flat_hash_map<std::string, Transformation> _transformations;
            absl::flat_hash_map<std::string, Condition> _conditions;
    };
}