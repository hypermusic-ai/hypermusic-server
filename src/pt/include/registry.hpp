#pragma once

#include <optional>
#include <string>
#include <filesystem>

#include "native.h"
#include <asio.hpp>
#include <absl/container/flat_hash_map.h>
#include <spdlog/spdlog.h>

#ifdef interface
    #undef interface
#endif
#include <evmc/evmc.hpp>
#ifndef interface
    #define interface __STRUCT__
#endif

#include "utils.hpp"
#include "feature.hpp"
#include "transformation.hpp"
#include "condition.hpp"
#include "file.hpp"

namespace dcn
{
    /**
     * @brief A class that represents a registry of features, transformations, and conditions.
     * 
     * This class is used to store and manage features, transformations, and conditions.
     * It provides methods to add, get, and remove features, transformations, and conditions.
     * It also provides methods to check if a feature, transformation, or condition exists.
     * 
     */
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
             * This function adds a feature to the registry. The feature is added to a
             * bucket with its name as the key. If the feature already exists, the
             * function returns `std::nullopt`. If the feature does not exist, it is
             * added and the hash of the feature is returned.
             */
            asio::awaitable<bool> addFeature(evmc::address address, Feature feature, evmc::address owner, std::filesystem::path source);
            
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
             * @brief Retrieves a specific feature by name and address.
             *
             * @param name The name of the feature.
             * @param address The address of the feature.
             * 
             * @return An `std::optional<Feature>` containing the feature if found,
             *         or `std::nullopt` if the feature or address does not exist.
             *
             * This function searches for a feature in the registry by its name and
             * address. If the feature name or address is not found, it returns
             * `std::nullopt`.
             */
            asio::awaitable<std::optional<Feature>> getFeature(const std::string& name, const evmc::address & address) const;

            /**
             * @brief Adds a transformation to the registry.
             * 
             * @param transformation The transformation to add.
             * 
             * 
             * This function adds a transformation to the registry. The transformation
             * is added to a bucket with its name as the key. If the transformation
             * already exists, the function returns `std::nullopt`. If the
             * transformation does not exist, it is added and the hash of the
             * transformation is returned.
             */
            asio::awaitable<bool> addTransformation(evmc::address address, Transformation transformation, evmc::address owner, std::filesystem::path source);

            /**
             * @brief Retrieves the newest transformation by name.
             *
             * @param name The name of the transformation.
             *
             * @return An `std::optional<Transformation>` containing the transformation
             *         if found, or `std::nullopt` if the transformation does not exist.
             *
             * This function searches for the newest transformation by name in the
             * registry. If the transformation does not exist, it returns `std::nullopt`.
             */
            asio::awaitable<std::optional<Transformation>> getNewestTransformation(const std::string& name) const;

            /**
             * @brief Retrieves a specific transformation by name and address.
             *
             * @param name The name of the transformation.
             * @param address The address of the transformation.
             *
             * @return An `std::optional<Transformation>` containing the transformation
             *         if found, or `std::nullopt` if the transformation or address does
             *         not exist.
             *
             * This function searches for a transformation in the registry by its name
             * and address. If the transformation name or address is not found, it
             * returns `std::nullopt`.
             */
            asio::awaitable<std::optional<Transformation>> getTransformation(const std::string& name, const evmc::address & address) const;

            /**
             * @brief Adds a condition to the registry.
             *
             * @param condition The condition to add.
             *
             * This function adds a condition to the registry. The condition is added
             * to a bucket with its name as the key. If the condition already exists,
             * the function returns `std::nullopt`. If the condition does not exist, it
             * is added and the hash of the condition is returned.
             */
            asio::awaitable<bool> addCondition(evmc::address address, Condition condition, std::filesystem::path source);

            /**
             * @brief Retrieves the newest condition by name.
             *
             * @param name The name of the condition.
             *
             * @return An `std::optional<Condition>` containing the condition if found,
             *         or `std::nullopt` if the condition does not exist.
             *
             * This function searches for the newest condition by name in the registry.
             * If the condition does not exist, it returns `std::nullopt`.
             */
            asio::awaitable<std::optional<Condition>> getNewestCondition(const std::string& name) const;

            /**
             * @brief Retrieves a specific condition by name and address.
             *
             * @param name The name of the condition.
             * @param address The address of the condition.
             *
             * @return An `std::optional<Condition>` containing the condition if found,
             *         or `std::nullopt` if the condition or address does not exist.
             *
             * This function searches for a condition in the registry by its name and
             * address. If the condition name or address is not found, it returns
             * `std::nullopt`.
             */
            asio::awaitable<std::optional<Condition>> getCondition(const std::string& name, const evmc::address & address) const;

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

        protected:

            /**
             * @brief Checks if a feature bucket exists in the registry.
             *
             * @param name The name of the feature bucket to check for.
             * 
             * @return True if the feature bucket with the given name exists,
             *         false otherwise.
             */
            asio::awaitable<bool> containsFeatureBucket(const std::string& name) const;

            /**
             * @brief Checks if a transformation bucket exists in the registry.
             *
             * @param name The name of the transformation bucket to check for.
             *
             * @return True if the transformation bucket with the given name exists,
             *         false otherwise.
             */
            asio::awaitable<bool> containsTransformationBucket(const std::string& name) const;

            /**
             * @brief Checks if a condition bucket exists in the registry.
             *
             * @param name The name of the condition bucket to check for.
             *
             * @return True if the condition bucket with the given name exists,
             *         false otherwise.
             */
            asio::awaitable<bool> containsConditionBucket(const std::string& name) const;

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
            asio::awaitable<bool> isFeatureBucketEmpty(const std::string& name) const;

            /**
             * @brief Checks if a transformation bucket is empty.
             *
             * @param name The name of the transformation bucket.
             *
             * @return true if the transformation bucket exists and is empty, false otherwise.
             *
             * This function first checks if the transformation bucket exists in the
             * registry. If it does not exist, the function returns true. If the
             * transformation bucket does exist, the function returns true if the
             * bucket is empty and false otherwise.
             */
            asio::awaitable<bool> isTransformationBucketEmpty(const std::string& name) const;

            /**
             * @brief Checks if a condition bucket is empty.
             *
             * @param name The name of the condition bucket.
             *
             * @return true if the condition bucket exists and is empty, false otherwise.
             *
             * This function first checks if the condition bucket exists in the
             * registry. If it does not exist, the function returns true. If the
             * condition bucket does exist, the function returns true if the bucket
             * is empty and false otherwise.
             */
            asio::awaitable<bool> isConditionBucketEmpty(const std::string& name) const;
        
        protected:
            template<class T>
            struct Node
            {
                T value;
                std::filesystem::path path;
            };

        private:
            asio::strand<asio::io_context::executor_type> _strand;

            absl::flat_hash_map<std::string, evmc::address> _newest_feature;
            absl::flat_hash_map<std::string, evmc::address> _newest_transformation;
            absl::flat_hash_map<std::string, evmc::address> _newest_condition;

            absl::flat_hash_map<std::string, absl::flat_hash_map<evmc::address, FeatureRecord>> _features;
            absl::flat_hash_map<std::string, absl::flat_hash_map<evmc::address, TransformationRecord>> _transformations;
            absl::flat_hash_map<std::string, absl::flat_hash_map<evmc::address, Node<Condition>>> _conditions;
    };
}