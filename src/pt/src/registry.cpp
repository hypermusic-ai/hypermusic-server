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

    asio::awaitable<std::optional<std::size_t>> Registry::addFeature(Feature feature)
    {
        co_await asio::dispatch(asio::bind_executor(_strand, asio::use_awaitable));

        if(containsFeatureBucket(feature.name())) {    
            spdlog::warn("Feature `{}` already exists", feature.name());
            co_return std::nullopt;
        }
        // create bucket
        _features.try_emplace(feature.name(), absl::flat_hash_map<std::size_t, Feature>());
                
        std::size_t hash_version = absl::Hash<Feature>{}(feature);
        _features.at(feature.name()).try_emplace(hash_version, std::move(feature));

        _newest_feature = hash_version;
        co_return hash_version;
    }

    asio::awaitable<std::optional<Feature>> Registry::getNewestFeature(const std::string& name) const
    {
        co_return (co_await getFeature(name, _newest_feature));
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
}