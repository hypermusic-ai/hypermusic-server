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

            Registry(const Registry&) = delete;
            Registry& operator=(const Registry&) = delete;
            
            ~Registry() = default;

            /**
             * @brief Adds a feature to the registry.
             *
             * @param feature The feature to add.
             *
             * @return An `std::optional<std::size_t>` containing the hash of the feature
             *         if it was added, or `std::nullopt` if the feature already exists.
             *
             * This function adds a feature to the registry. The feature is added to a
             * bucket with its name as the key. If the feature already exists, the
             * function returns `std::nullopt`. If the feature does not exist, it is
             * added and the hash of the feature is returned.
             */
            asio::awaitable<std::optional<std::size_t>> addFeature(Feature feature);
            
            /**
             * @brief Retrieves the newest feature by name.
             *
             * @param name The name of the feature.
             *
             * @return An `std::optional<Feature>` containing the feature if found,
             *         or `std::nullopt` if the feature does not exist.
             *
             * This function searches for the newest feature by name in the registry.
             * If the feature does not exist, it returns `std::nullopt`.
             */
            asio::awaitable<std::optional<Feature>> getNewestFeature(const std::string& name) const;

            /**
             * @brief Retrieves a specific feature by name and version.
             *
             * @param name The name of the feature.
             * @param version The version of the feature.
             * 
             * @return An `std::optional<Feature>` containing the feature if found,
             *         or `std::nullopt` if the feature or version does not exist.
             *
             * This function searches for a feature in the registry by its name and
             * version. If the feature name or version is not found, it returns
             * `std::nullopt`.
             */
            asio::awaitable<std::optional<Feature>> getFeature(const std::string& name, std::size_t version) const;

        protected:

            /**
             * @brief Checks if a feature bucket exists in the registry.
             *
             * @param name The name of the feature bucket to check for.
             * 
             * @return True if the feature bucket with the given name exists,
             *         false otherwise.
             */
            bool containsFeatureBucket(const std::string& name) const;

            /**
             * @brief Checks if a feature bucket is empty.
             *
             * @param name The name of the feature bucket.
             *
             * @return true if the feature bucket exists and is empty, false otherwise.
             *
             * This function first checks if the feature bucket exists in the registry.
             * If it does not exist, the function returns true. If the feature bucket
             * does exist, the function returns true if the bucket is empty and false
             * otherwise.
             */
            bool isFeatureBucketEmpty(const std::string& name) const;

            /**
             * @brief Recursively checks if all subfeatures exist in the registry
             *
             * @param feature The feature to check
             *
             * @return true if all subfeatures exist, false otherwise
             *
             * This function works by iterating over all dimensions of the feature
             * and recursively checking if each subfeature exists in the registry.
             * If at any point any subfeature does not exist, the function returns false.
             */
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