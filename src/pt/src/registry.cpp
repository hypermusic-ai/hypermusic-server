#include "registry.hpp"

namespace hm
{   
    Registry::Registry( asio::io_context & io_context)
    : _strand(asio::make_strand(io_context))
    {

    }

    asio::awaitable<bool> Registry::checkIfSubFeaturesExist(const Feature & feature) const
    {
        co_await asio::dispatch(asio::bind_executor(_strand, asio::use_awaitable));

        for(const Dimension & dimension : feature.dimensions())
        {
            const std::string & subfeature_name = dimension.feature_name();
            if(co_await isFeatureBucketEmpty(subfeature_name))co_return false;

            const Feature & sub_feature = _features.at(subfeature_name).at(_newest_feature.at(subfeature_name));
            co_return (co_await checkIfSubFeaturesExist(sub_feature));
        }
        co_return true;
    }


    asio::awaitable<bool> Registry::containsFeatureBucket(const std::string& name) const 
    {
        co_await asio::dispatch(asio::bind_executor(_strand, asio::use_awaitable));

        co_return _features.contains(name);
    }

    asio::awaitable<bool> Registry::isFeatureBucketEmpty(const std::string& name) const 
    {
        co_await asio::dispatch(asio::bind_executor(_strand, asio::use_awaitable));

        if(co_await containsFeatureBucket(name) == false)co_return true;
        co_return _features.at(name).empty();
    }

    asio::awaitable<bool> Registry::containsTransformationBucket(const std::string& name) const 
    {
        co_await asio::dispatch(asio::bind_executor(_strand, asio::use_awaitable));

        co_return _transformations.contains(name);
    }

    asio::awaitable<bool> Registry::isTransformationBucketEmpty(const std::string& name) const 
    {
        co_await asio::dispatch(asio::bind_executor(_strand, asio::use_awaitable));

        if(co_await containsTransformationBucket(name) == false)co_return true;
        co_return _transformations.at(name).empty();
    }

    asio::awaitable<bool> Registry::containsConditionBucket(const std::string& name) const 
    {
        co_await asio::dispatch(asio::bind_executor(_strand, asio::use_awaitable));

        co_return _conditions.contains(name);
    }

    asio::awaitable<bool> Registry::isConditionBucketEmpty(const std::string& name) const 
    {
        co_await asio::dispatch(asio::bind_executor(_strand, asio::use_awaitable));

        if(co_await containsConditionBucket(name) == false)co_return true;
        co_return _conditions.at(name).empty();
    }


    asio::awaitable<std::optional<std::size_t>> Registry::addFeature(Feature feature)
    {
        if(feature.name().empty())
        {
            spdlog::error("Feature name is empty");
            co_return std::nullopt;
        }

        co_await asio::dispatch(asio::bind_executor(_strand, asio::use_awaitable));

        if(co_await checkIfSubFeaturesExist(feature) == false)
        {
            spdlog::error("Cannot find subfeatures for feature `{}`", feature.name());
            co_return std::nullopt;
        }

        if(! co_await containsFeatureBucket(feature.name()))
        {
            spdlog::debug("Feature bucket `{}` does not exists, creating new one ... ", feature.name());
            _features.try_emplace(feature.name(), absl::flat_hash_map<std::size_t, Feature>());
        }       

        std::size_t hash_version = absl::Hash<Feature>{}(feature);
        if(_features.at(feature.name()).contains(hash_version))
        {
            spdlog::error("Feature `{}` of this signature already exists", feature.name());
            co_return std::nullopt;
        }

        // check if transformations exists
        for(const Dimension & dimension : feature.dimensions())
        {
            for(const auto & transformation : dimension.transformation_name())
            {
                if(! co_await containsTransformationBucket(transformation))
                {
                    spdlog::error("Cannot find transformation `{}` used in feature `{}`", transformation, feature.name());
                    co_return std::nullopt;
                }
                if(co_await isTransformationBucketEmpty(transformation))
                {
                    spdlog::error("Cannot find transformation `{}` used in feature `{}`", transformation, feature.name());
                    co_return std::nullopt;
                }
            }
        }

        _newest_feature[feature.name()] = hash_version;

        _features.at(feature.name()).try_emplace(hash_version, std::move(feature));

        co_return hash_version;
    }

    asio::awaitable<std::optional<Feature>> Registry::getNewestFeature(const std::string& name) const
    {
        co_await asio::dispatch(asio::bind_executor(_strand, asio::use_awaitable));

        if(_newest_feature.contains(name) == false)co_return std::nullopt;
        co_return (co_await getFeature(name, _newest_feature.at(name)));
    }

    asio::awaitable<std::optional<Feature>> Registry::getFeature(const std::string& name, std::size_t version) const
    {
        co_await asio::dispatch(asio::bind_executor(_strand, asio::use_awaitable));

        auto bucket_it = _features.find(name);
        if(bucket_it == _features.end()) 
        {
            co_return std::nullopt;
        }
        auto it = bucket_it->second.find(version);
        if(it == bucket_it->second.end()) 
        {
            co_return std::nullopt;
        }
        co_return it->second;
    }


    asio::awaitable<std::optional<std::size_t>> Registry::addTransformation(Transformation transformation)
    {
        if(transformation.name().empty())
        {
            spdlog::error("Transformation name is empty");
            co_return std::nullopt;
        }

        co_await asio::dispatch(asio::bind_executor(_strand, asio::use_awaitable));

        if(! co_await containsTransformationBucket(transformation.name())) 
        {
            spdlog::debug("Transformation bucket `{}` does not exists, creating new one ... ", transformation.name());
            _transformations.try_emplace(transformation.name(), absl::flat_hash_map<std::size_t, Transformation>());
        }

        std::size_t hash_version = absl::Hash<Transformation>{}(transformation);

        if(_transformations.at(transformation.name()).contains(hash_version))
        {
            spdlog::error("Transformation `{}` of this signature already exists", transformation.name());
            co_return std::nullopt;
        }

        _newest_transformation[transformation.name()] = hash_version;
        _transformations.at(transformation.name()).try_emplace(hash_version, std::move(transformation));
        co_return hash_version;
    }

    asio::awaitable<std::optional<Transformation>> Registry::getNewestTransformation(const std::string& name) const
    {
        co_await asio::dispatch(asio::bind_executor(_strand, asio::use_awaitable));

        if(_newest_transformation.contains(name) == false)co_return std::nullopt;
        co_return (co_await getTransformation(name, _newest_transformation.at(name)));
    }

    asio::awaitable<std::optional<Transformation>> Registry::getTransformation(const std::string& name, std::size_t version) const
    {
        co_await asio::dispatch(asio::bind_executor(_strand, asio::use_awaitable));

        auto bucket_it = _transformations.find(name);
        if(bucket_it == _transformations.end()) 
        {
            co_return std::nullopt;
        }
        auto it = bucket_it->second.find(version);
        if(it == bucket_it->second.end()) 
        {
            co_return std::nullopt;
        }
        co_return it->second;
    }


    asio::awaitable<std::optional<std::size_t>> Registry::addCondition(Condition condition)
    {
        co_await asio::dispatch(asio::bind_executor(_strand, asio::use_awaitable));

        if(! co_await containsConditionBucket(condition.name())) 
        {
            spdlog::debug("Condition bucket `{}` does not exists, creating new one ... ", condition.name());
            _conditions.try_emplace(condition.name(), absl::flat_hash_map<std::size_t, Condition>());
        }

        std::size_t hash_version = absl::Hash<Condition>{}(condition);

        if(_conditions.at(condition.name()).contains(hash_version))
        {
            spdlog::error("Condition `{}` of this signature already exists", condition.name());
            co_return std::nullopt;
        }

        _newest_condition[condition.name()] = hash_version;
        _conditions.at(condition.name()).try_emplace(hash_version, std::move(condition));
        co_return hash_version;
    }

    asio::awaitable<std::optional<Condition>> Registry::getNewestCondition(const std::string& name) const
    {
        co_await asio::dispatch(asio::bind_executor(_strand, asio::use_awaitable));

        if(_newest_condition.contains(name) == false)co_return std::nullopt;
        co_return (co_await getCondition(name, _newest_condition.at(name)));
    }

    asio::awaitable<std::optional<Condition>> Registry::getCondition(const std::string& name, std::size_t version) const
    {
        co_await asio::dispatch(asio::bind_executor(_strand, asio::use_awaitable));

        auto bucket_it = _conditions.find(name);
        if(bucket_it == _conditions.end()) 
        {
            co_return std::nullopt;
        }
        auto it = bucket_it->second.find(version);
        if(it == bucket_it->second.end()) 
        {
            co_return std::nullopt;
        }
        co_return it->second;
    }
}