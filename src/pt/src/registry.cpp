#include "registry.hpp"

namespace hm
{   
    Registry::Registry( asio::io_context & io_context)
    : _strand(asio::make_strand(io_context))
    {

    }

    bool Registry::containsFeatureBucket(const std::string& name) const 
    {
        return _features.contains(name);
    }

    bool Registry::isFeatureBucketEmpty(const std::string& name) const 
    {
        if(containsFeatureBucket(name) == false)return true;
        return _features.at(name).empty();
    }

    asio::awaitable<std::optional<std::size_t>> Registry::addFeature(Feature feature)
    {
        co_await asio::dispatch(asio::bind_executor(_strand, asio::use_awaitable));

        if(co_await checkIfSubFeaturesExist(feature) == false)
        {
            spdlog::error("Cannot find subfeatures for feature `{}`", feature.name());
            co_return std::nullopt;
        }

        if(!containsFeatureBucket(feature.name())) 
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

        _newest_feature[feature.name()] = hash_version;

        _features.at(feature.name()).try_emplace(hash_version, std::move(feature));

        co_return hash_version;
    }

    asio::awaitable<std::optional<Feature>> Registry::getNewestFeature(const std::string& name) const
    {
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

    asio::awaitable<bool> Registry::checkIfSubFeaturesExist(const Feature & feature) const
    {
        for(const Dimension & dimension : feature.dimensions())
        {
            const std::string & subfeature_name = dimension.feature_name();
            if(isFeatureBucketEmpty(subfeature_name))co_return false;

            const Feature & sub_feature = _features.at(subfeature_name).at(_newest_feature.at(subfeature_name));
            co_return (co_await checkIfSubFeaturesExist(sub_feature));
        }
        co_return true;
    }
}