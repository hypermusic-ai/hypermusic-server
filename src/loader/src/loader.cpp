#include "loader.hpp"

namespace dcn
{
    template<class T>
    static absl::flat_hash_map<std::string, T> _loadJSONRecords(std::filesystem::path subdir)
    {
        std::ifstream file;

        static const auto storage_dir = getStoragePath() / subdir;

        absl::flat_hash_map<std::string, T> loaded_data;
        std::optional<T> loaded_result;
        
        bool success = true;

        try {
            for (const auto& entry : std::filesystem::directory_iterator(storage_dir)) 
            {
                if (!entry.is_regular_file() || entry.path().extension() != ".json") continue;
                
                spdlog::debug(std::format("Found JSON file: {}", entry.path().string()));
                file.open(entry.path());

                if (!file.is_open()) {
                    spdlog::error(std::format("Failed to open file: {}", entry.path().string()));
                    continue;
                }

                std::string json((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

                loaded_result = parse::parseFromJson<T>(json, parse::use_protobuf);
                if(!loaded_result)
                {
                    spdlog::error(std::format("Failed to parse JSON: {}", json));
                    continue;
                }

                loaded_data.try_emplace(entry.path().stem().string(), std::move(*loaded_result));

                file.close();
            }
        } 
        catch (const std::filesystem::filesystem_error& e) 
        {
            spdlog::error(std::format("Filesystem error: {}", e.what()));
            success = false;
        } 
        catch (const std::exception& e) 
        {
            spdlog::error(std::format("Exception: {}", e.what()));
            success = false;
        }

        if(!success)
        {
            return {};
        }

        return loaded_data;
    }

    asio::awaitable<bool> deployFeature(EVM & evm, Registry & registry, FeatureRecord feature_record)
    {
        if(co_await registry.checkIfSubFeaturesExist(feature_record.feature()) == false)
        {
            spdlog::error("Cannot find subfeatures for feature");
            co_return false;
        }

        const std::filesystem::path code_path = feature_record.code_path();
        static const std::filesystem::path out_dir = getResourcesPath() / "contracts" / "features" / "build";

        std::filesystem::create_directories(code_path.parent_path());
        std::filesystem::create_directories(out_dir);

        // create code file
        std::ofstream out_file(code_path); 
        if(!out_file.is_open())
        {
            spdlog::error("Failed to create file");
            co_return false;
        }

        out_file << constructFeatureSolidityCode(feature_record.feature());
        out_file.close();

        // compile code
        if(!co_await evm.compile(code_path, out_dir, getPTPath()/ "contracts", getPTPath() / "node_modules"))
        {
            spdlog::error("Failed to compile code");
            co_return false;
        }
        
        const auto address_result = evmc::from_hex<evmc::address>(feature_record.owner());
        if(!address_result)
        {
            spdlog::error("Failed to parse address");
            co_return false;
        }
        const auto & address = *address_result;

        co_await evm.addAccount(address, 1000000000);
        co_await evm.setGas(address, 1000000000);
        
        auto deploy_res = co_await evm.deploy(
            out_dir / (feature_record.feature().name() + ".bin"), 
            address, 
            encodeAsArg(evm.getRegistryAddress()),
            1000000000, 
            0);

        if(!deploy_res)
        {
            spdlog::error("Failed to deploy code : {}", deploy_res.error());
            co_return false;
        }

        const auto owner_result = co_await fetchOwner(evm, deploy_res.value());

        // check execution status
        if(!owner_result)
        {
            spdlog::error("Failed to fetch owner {}", owner_result.error());
            co_return false;
        }

        const auto owner_address = decodeReturnedValue<evmc::address>(owner_result.value());

        if(!co_await registry.addFeature(deploy_res.value(), feature_record.feature(), owner_address, code_path)) 
        {
            spdlog::error("Failed to add feature");
            co_return false;
        }

        spdlog::debug("feature '{}' added", feature_record.feature().name());
        co_return true;
    }

    asio::awaitable<bool> deployTransformation(EVM & evm, Registry & registry, TransformationRecord transformation_record)
    {
        if(transformation_record.transformation().name().empty() || transformation_record.transformation().sol_src().empty())
        {
            spdlog::error("Transformation name or source is empty");
            co_return false;
        }

        const std::filesystem::path code_path = transformation_record.code_path();
        static const std::filesystem::path out_dir = getResourcesPath() / "contracts" / "transformations" / "build";

        std::filesystem::create_directories(code_path.parent_path());
        std::filesystem::create_directories(out_dir);

        // create code file
        std::ofstream out_file(code_path); 
        if(!out_file.is_open())
        {
            spdlog::error("Failed to create file");
            co_return false;
        }

        out_file << constructTransformationSolidityCode(transformation_record.transformation());
        out_file.close();

        // compile code
        if(!co_await evm.compile(code_path, out_dir, getPTPath()/ "contracts", getPTPath() / "node_modules"))
        {
            spdlog::error("Failed to compile code");
            co_return false;
        }

        const auto address_result = evmc::from_hex<evmc::address>(transformation_record.owner());
        if(!address_result)
        {
            spdlog::error("Failed to parse address");
            co_return false;
        }
        const auto & address = *address_result;

        co_await evm.addAccount(address, 1000000000);
        co_await evm.setGas(address, 1000000000);

        auto deploy_res = co_await evm.deploy(  
            out_dir / (transformation_record.transformation().name() + ".bin"), 
            address, 
            encodeAsArg(evm.getRegistryAddress()), 
            1000000, 
            0);
        
        if(!deploy_res)
        {
            spdlog::error("Failed to deploy code");
            co_return false;
        }
        const auto owner_result = co_await fetchOwner(evm, deploy_res.value());

        // check execution status
        if(!owner_result)
        {
            spdlog::error("Failed to fetch owner {}", owner_result.error());
            co_return false;
        }
        const auto owner_address = decodeReturnedValue<evmc::address>(owner_result.value());

        if(!co_await registry.addTransformation(deploy_res.value(), transformation_record.transformation(), owner_address, code_path)) 
        {
            spdlog::error("Failed to add transformation to registry");
            co_return false;
        }

        spdlog::debug("transformation '{}' added", transformation_record.transformation().name());
        co_return true;
    }

    asio::awaitable<bool> loadStoredFeatures(EVM & evm, Registry & registry)
    {
        spdlog::info("Loading stored features...");

        auto loaded_features = _loadJSONRecords<FeatureRecord>("features");
        if(loaded_features.empty())
        {
            co_return false;
        }

        const auto sorted_features = utils::topologicalSort<FeatureRecord, Dimension, google::protobuf::RepeatedPtrField>(
                loaded_features,
                [](const FeatureRecord & record){return record.feature().dimensions();},
                [](const Dimension & dim) {return dim.feature_name();}
            );

        bool success = true;
        for(const auto & name : sorted_features)
        {
            if(!co_await deployFeature(evm, registry, std::move(loaded_features.at(name))))
            {
                spdlog::error("Failed to deploy feature `{}`", name);
                success = false;
            }
        }

        co_return success;
    }

    asio::awaitable<bool> loadStoredTransformations(EVM & evm, Registry & registry)
    {
        spdlog::info("Loading stored transformations...");

        auto loaded_transformations = _loadJSONRecords<TransformationRecord>("transformations");
        if(loaded_transformations.empty())
        {
            co_return false;
        }

        bool success = true;
        for(auto & [name, transformation] : loaded_transformations)
        {
            if(!co_await deployTransformation(evm, registry, std::move(transformation)))
            {
                spdlog::error("Failed to deploy transformation `{}`", name);
                success = false;
            }
        }

        co_return success;
    }
}