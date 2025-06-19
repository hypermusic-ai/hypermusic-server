#include "registry.hpp"

namespace dcn
{   
    Registry::Registry( asio::io_context & io_context)
    : _strand(asio::make_strand(io_context))
    {

    }

    asio::awaitable<bool> Registry::checkIfSubFeaturesExist(const Feature & feature) const
    {
        co_await utils::ensureOnStrand(_strand);

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
        co_await utils::ensureOnStrand(_strand);

        co_return _features.contains(name);
    }

    asio::awaitable<bool> Registry::isFeatureBucketEmpty(const std::string& name) const 
    {
        co_await utils::ensureOnStrand(_strand);

        if(co_await containsFeatureBucket(name) == false)co_return true;
        co_return _features.at(name).empty();
    }

    asio::awaitable<bool> Registry::containsTransformationBucket(const std::string& name) const 
    {
        co_await utils::ensureOnStrand(_strand);

        co_return _transformations.contains(name);
    }

    asio::awaitable<bool> Registry::isTransformationBucketEmpty(const std::string& name) const 
    {
        co_await utils::ensureOnStrand(_strand);

        if(co_await containsTransformationBucket(name) == false)co_return true;
        co_return _transformations.at(name).empty();
    }

    asio::awaitable<bool> Registry::containsConditionBucket(const std::string& name) const 
    {
        co_await utils::ensureOnStrand(_strand);

        co_return _conditions.contains(name);
    }

    asio::awaitable<bool> Registry::isConditionBucketEmpty(const std::string& name) const 
    {
        co_await utils::ensureOnStrand(_strand);

        if(co_await containsConditionBucket(name) == false)co_return true;
        co_return _conditions.at(name).empty();
    }


    asio::awaitable<bool> Registry::addFeature(Feature feature, const std::string & address)
    {
        if(feature.name().empty())
        {
            spdlog::error("Feature name is empty");
            co_return false;
        }

        co_await utils::ensureOnStrand(_strand);

        if(co_await checkIfSubFeaturesExist(feature) == false)
        {
            spdlog::error("Cannot find subfeatures for feature `{}`", feature.name());
            co_return false;
        }

        if(! co_await containsFeatureBucket(feature.name()))
        {
            spdlog::debug("Feature bucket `{}` does not exists, creating new one ... ", feature.name());
            _features.try_emplace(feature.name(), absl::flat_hash_map<std::string, Feature>());
        }       

        if(_features.at(feature.name()).contains(address))
        {
            spdlog::error("Feature `{}` of this signature already exists", feature.name());
            co_return false;
        }

        // check if transformations exists
        for(const Dimension & dimension : feature.dimensions())
        {
            for(const auto & transformation : dimension.transformations())
            {
                if(! co_await containsTransformationBucket(transformation.name()))
                {
                    spdlog::error("Cannot find transformation `{}` used in feature `{}`", transformation.name(), feature.name());
                    co_return false;
                }
                if(co_await isTransformationBucketEmpty(transformation.name()))
                {
                    spdlog::error("Cannot find transformation `{}` used in feature `{}`", transformation.name(), feature.name());
                    co_return false;
                }
            }
        }

        _newest_feature[feature.name()] = address;

        _features.at(feature.name()).try_emplace(address, std::move(feature));

        co_return true;
    }

    asio::awaitable<std::optional<Feature>> Registry::getNewestFeature(const std::string& name) const
    {
        co_await utils::ensureOnStrand(_strand);

        if(_newest_feature.contains(name) == false)co_return std::nullopt;
        co_return (co_await getFeature(name, _newest_feature.at(name)));
    }

    asio::awaitable<std::optional<Feature>> Registry::getFeature(const std::string& name, const std::string & address) const
    {
        co_await utils::ensureOnStrand(_strand);

        auto bucket_it = _features.find(name);
        if(bucket_it == _features.end()) 
        {
            co_return std::nullopt;
        }
        auto it = bucket_it->second.find(address);
        if(it == bucket_it->second.end()) 
        {
            co_return std::nullopt;
        }
        co_return it->second;
    }


    asio::awaitable<bool> Registry::addTransformation(Transformation transformation, const std::string & address)
    {
        if(transformation.name().empty())
        {
            spdlog::error("Transformation name is empty");
            co_return false;
        }

        co_await utils::ensureOnStrand(_strand);

        if(! co_await containsTransformationBucket(transformation.name())) 
        {
            spdlog::debug("Transformation bucket `{}` does not exists, creating new one ... ", transformation.name());
            _transformations.try_emplace(transformation.name(), absl::flat_hash_map<std::string, Transformation>());
        }

        if(_transformations.at(transformation.name()).contains(address))
        {
            spdlog::error("Transformation `{}` of this signature already exists", transformation.name());
            co_return false;
        }

        _newest_transformation[transformation.name()] = address;
        _transformations.at(transformation.name()).try_emplace(address, std::move(transformation));
        co_return true;
    }

    asio::awaitable<std::optional<Transformation>> Registry::getNewestTransformation(const std::string& name) const
    {
        co_await utils::ensureOnStrand(_strand);

        if(_newest_transformation.contains(name) == false)co_return std::nullopt;
        co_return (co_await getTransformation(name, _newest_transformation.at(name)));
    }

    asio::awaitable<std::optional<Transformation>> Registry::getTransformation(const std::string& name, const std::string & address) const
    {
        co_await utils::ensureOnStrand(_strand);

        auto bucket_it = _transformations.find(name);
        if(bucket_it == _transformations.end()) 
        {
            co_return std::nullopt;
        }
        auto it = bucket_it->second.find(address);
        if(it == bucket_it->second.end()) 
        {
            co_return std::nullopt;
        }
        co_return it->second;
    }


    asio::awaitable<bool> Registry::addCondition(Condition condition, const std::string & address)
    {
        co_await utils::ensureOnStrand(_strand);

        if(! co_await containsConditionBucket(condition.name())) 
        {
            spdlog::debug("Condition bucket `{}` does not exists, creating new one ... ", condition.name());
            _conditions.try_emplace(condition.name(), absl::flat_hash_map<std::string, Condition>());
        }


        if(_conditions.at(condition.name()).contains(address))
        {
            spdlog::error("Condition `{}` of this signature already exists", condition.name());
            co_return false;
        }

        _newest_condition[condition.name()] = address;
        _conditions.at(condition.name()).try_emplace(address, std::move(condition));
        co_return true;
    }

    asio::awaitable<std::optional<Condition>> Registry::getNewestCondition(const std::string& name) const
    {
        co_await utils::ensureOnStrand(_strand);

        if(_newest_condition.contains(name) == false)co_return std::nullopt;
        co_return (co_await getCondition(name, _newest_condition.at(name)));
    }

    asio::awaitable<std::optional<Condition>> Registry::getCondition(const std::string& name, const std::string & address) const
    {
        co_await utils::ensureOnStrand(_strand);

        auto bucket_it = _conditions.find(name);
        if(bucket_it == _conditions.end()) 
        {
            co_return std::nullopt;
        }
        auto it = bucket_it->second.find(address);
        if(it == bucket_it->second.end()) 
        {
            co_return std::nullopt;
        }
        co_return it->second;
    }
}