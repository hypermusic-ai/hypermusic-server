#include "api.hpp"

namespace dcn
{
    static asio::awaitable<std::expected<evmc::address, AuthenticationError>> _authenticate(const http::Request & request, const AuthManager & auth_manager)
    {
        auto cookie_res = request.getHeader(http::Header::Cookie);
        if(cookie_res.empty())
        {
            spdlog::error("Missing cookie");
            co_return std::unexpected(AuthenticationError::MissingCookie);
        }
        const std::string cookie_header = std::accumulate(cookie_res.begin(), cookie_res.end(), std::string(""));

        const auto token_res = parse::parseAccessTokenFromCookieHeader(cookie_header);
        if (token_res.has_value() == false) 
        {
            spdlog::error("Failed to parse token");
            co_return std::unexpected(AuthenticationError::MissingToken);
        }
        const std::string & token = token_res.value();

        auto verification_res = co_await auth_manager.verifyAccessToken(token);

        if(verification_res.has_value() == false)
        {
            spdlog::error("Failed to verify token");
            co_return std::unexpected(verification_res.error());
        }

        co_return verification_res.value();
    }

    static asio::awaitable<std::expected<std::vector<asio::detail::buffered_stream_storage::byte_type>, evmc_status_code>> _fetchOwner(EVM & evm, const evmc::address & address)
    {
        std::vector<uint8_t> input_data;
        const auto selector = constructFunctionSelector("getOwner()");
        input_data.insert(input_data.end(), selector.begin(), selector.end());
        co_return  co_await evm.execute(evm.getRegistryAddress(), address, input_data, 1'000'000, 0);
    }

    asio::awaitable<http::Response> OPTIONS_feature(const http::Request &, std::vector<RouteArg>)
    {
        http::Response response;
        response.setVersion("HTTP/1.1");
        response.setHeader(http::Header::AccessControlAllowOrigin, "*");
        response.setHeader(http::Header::AccessControlAllowMethods, "GET, POST, OPTIONS");
        response.setHeader(http::Header::AccessControlAllowHeaders, "Content-Type");
        response.setHeader(http::Header::Connection, "close");
        response.setHeader(http::Header::ContentType, "text/plain");
        response.setCode(http::Code::OK);
        co_return response;
    }

    asio::awaitable<http::Response> GET_feature(const http::Request &, std::vector<RouteArg> args, Registry & registry, EVM & evm)
    {
        http::Response response;
        response.setVersion("HTTP/1.1");
        response.setHeader(http::Header::Connection, "close");
        response.setHeader(http::Header::AccessControlAllowOrigin, "*");

        if(args.size() > 2 || args.size() == 0)
        {
            response.setHeader(http::Header::ContentType, "text/plain");
            response.setCode(http::Code::BadRequest);
            response.setBody("invalid url");
            co_return response;
        }

        auto feature_name_result = parse::parseRouteArgAs<std::string>(args.at(0));

        if(!feature_name_result)
        {
            response.setHeader(http::Header::ContentType, "text/plain");
            response.setCode(http::Code::BadRequest);
            response.setBody("invalid url");
            co_return response;
        }
        const auto & feature_name = feature_name_result.value();

        std::optional<Feature> feature_res;

        if(args.size() == 2)
        {
            const auto feature_address_arg = parse::parseRouteArgAs<std::string>(args.at(1));

            if(!feature_address_arg)
            {
                response.setHeader(http::Header::ContentType, "text/plain");
                response.setCode(http::Code::BadRequest);
                response.setBody("invalid url");
                co_return response;
            }

            const auto feature_address_result = evmc::from_hex<evmc::address>(feature_address_arg.value());

            if(!feature_address_result)
            {
                response.setHeader(http::Header::ContentType, "text/plain");
                response.setCode(http::Code::BadRequest);
                response.setBody("invalid url");
                co_return response;
            }

            feature_res = co_await registry.getFeature(feature_name_result.value(), feature_address_result.value());
        }
        else if(args.size() == 1)
        {
            feature_res = co_await registry.getNewestFeature(feature_name_result.value());
        }

        if(!feature_res) 
        {
            response.setHeader(http::Header::ContentType, "text/plain");
            response.setCode(http::Code::NotFound);
            response.setBody("feature not found");
            co_return response;
        }
        
        auto json_res = parse::parseToJson(*feature_res, parse::use_json);

        if(!json_res)
        {
            response.setHeader(http::Header::ContentType, "text/plain");
            response.setCode(http::Code::InternalServerError);
            response.setBody("internal server error");
            co_return response;
        }

        std::vector<uint8_t> input_data;
        // function selector
        const auto selector = constructFunctionSelector("getFeature(string)");
        input_data.insert(input_data.end(), selector.begin(), selector.end());

        // Step 2: Offset to string data (32 bytes with value 0x20)
        std::vector<uint8_t> offset(32, 0);
        offset[31] = 0x20;
        input_data.insert(input_data.end(), offset.begin(), offset.end());

        // Step 3: String length
        std::vector<uint8_t> str_len(32, 0);
        str_len[31] = static_cast<uint8_t>(feature_name.size());
        input_data.insert(input_data.end(), str_len.begin(), str_len.end());

        // Step 4: String bytes
        input_data.insert(input_data.end(), feature_name.begin(), feature_name.end());

        // Step 5: Padding to 32-byte boundary
        size_t padding = (32 - (feature_name.size() % 32)) % 32;
        input_data.insert(input_data.end(), padding, 0);
        const auto exec_result = co_await evm.execute(evm.getRegistryAddress(), evm.getRegistryAddress(), input_data, 1'000'000, 0);
        
        // check execution status
        if(!exec_result)
        {
            spdlog::error("Failed to fetch feature {}", exec_result.error());
            response.setHeader(http::Header::Connection, "close");
            response.setHeader(http::Header::ContentType, "text/plain");
            response.setCode(http::Code::InternalServerError);
            response.setBody(std::format("Failed to fetch feature : {}", exec_result.error()));
            co_return std::move(response);
        }

        const auto feature_address = decodeReturnedValue<evmc::address>(exec_result.value());
        const auto owner_result = co_await _fetchOwner(evm, feature_address);
        if(!owner_result)
        {
            spdlog::error("Failed to fetch owner {}", owner_result.error());
            response.setHeader(http::Header::Connection, "close");
            response.setHeader(http::Header::ContentType, "text/plain");
            response.setCode(http::Code::InternalServerError);
            response.setBody(std::format("Failed to fetch owner : {}", owner_result.error()));
            co_return std::move(response);
        }

        const auto owner_address = decodeReturnedValue<evmc::address>(owner_result.value());

        (*json_res)["owner"] = evmc::hex(owner_address);
        (*json_res)["local_address"] = evmc::hex(feature_address);
        (*json_res)["address"] = "0x0";

        response.setHeader(http::Header::ContentType, "application/json");
        response.setCode(http::Code::OK);
        response.setBody(json_res->dump());
        co_return std::move(response);
    }

    asio::awaitable<http::Response> POST_feature(const http::Request & request, std::vector<RouteArg> args, AuthManager & auth_manager, Registry & registry, EVM & evm)
    {
        http::Response response;
        response.setVersion("HTTP/1.1");
        response.setHeader(http::Header::AccessControlAllowOrigin, "*");

        const auto auth_result = co_await _authenticate(request, auth_manager);

        if(auth_result.has_value() == false)
        {
            response.setHeader(http::Header::Connection, "close");
            response.setHeader(http::Header::ContentType, "text/plain");
            response.setCode(http::Code::Unauthorized);
            response.setBody(std::format("Error: {}", auth_result.error()));
            co_return std::move(response);
        }
        const auto & address = auth_result.value();

        spdlog::debug(std::format("token verified address : {}", address));

        // parse feature from json_string
        auto feature_res = parse::parseFromJson<Feature>(request.getBody(), parse::use_protobuf);

        if(!feature_res) 
        {
            response.setHeader(http::Header::Connection, "close");
            response.setHeader(http::Header::ContentType, "text/plain");
            response.setCode(http::Code::BadRequest);
            response.setBody("Failed to parse feature");
            co_return std::move(response);
        }

        const Feature & feature = *feature_res;

        // create file with sol code
        // compile
        // deploy
        // add to EVM machine

        std::filesystem::path code_path = getResourcesPath() / "contracts" / "features" / (feature.name() + ".sol");
        std::filesystem::path out_dir = getResourcesPath() / "contracts" / "features" / "build";

        std::filesystem::create_directories(code_path.parent_path());
        std::filesystem::create_directories(out_dir);

        // create code file
        std::ofstream out_file(code_path); 
        if(!out_file.is_open())
        {
            spdlog::error("Failed to create file");
            response.setHeader(http::Header::Connection, "close");
            response.setHeader(http::Header::ContentType, "text/plain");
            response.setCode(http::Code::InternalServerError);
            response.setBody("Failed to create file");
            co_return std::move(response);
        }

        out_file << constructFeatureSolidityCode(feature);
        out_file.close();

        // compile code
        if(!co_await evm.compile(code_path, out_dir, getPTPath()/ "contracts", getPTPath() / "node_modules"))
        {
            spdlog::error("Failed to compile code");
            response.setHeader(http::Header::Connection, "close");
            response.setHeader(http::Header::ContentType, "text/plain");
            response.setCode(http::Code::InternalServerError);
            response.setBody("Failed to compile code");
            co_return std::move(response);
        }
        
        co_await evm.addAccount(address, 1000000); //TODO
        
        auto deploy_res = co_await evm.deploy(
            out_dir / (feature.name() + ".bin"), 
            address, 
    encodeAsArg(evm.getRegistryAddress()),
            1000000, 
            0);

        if(!deploy_res)
        {
            spdlog::error("Failed to deploy code : {}", deploy_res.error());
            response.setHeader(http::Header::Connection, "close");
            response.setHeader(http::Header::ContentType, "text/plain");
            response.setCode(http::Code::InternalServerError);
            response.setBody(std::format("Failed to deploy code : {}",  deploy_res.error()));
            co_return std::move(response);
        }

        const auto owner_result = co_await _fetchOwner(evm, deploy_res.value());

        // check execution status
        if(!owner_result)
        {
            spdlog::error("Failed to fetch owner {}", owner_result.error());
            response.setHeader(http::Header::Connection, "close");
            response.setHeader(http::Header::ContentType, "text/plain");
            response.setCode(http::Code::InternalServerError);
            response.setBody(std::format("Failed to fetch owner : {}", owner_result.error()));
            co_return std::move(response);
        }

        if(!co_await registry.addFeature(deploy_res.value(), feature, code_path)) 
        {
            spdlog::error("Failed to add feature");
            response.setHeader(http::Header::Connection, "close");
            response.setHeader(http::Header::ContentType, "text/plain");
            response.setCode(http::Code::BadRequest);
            response.setBody("Failed to add feature");
            co_return std::move(response);
        }

        spdlog::debug("feature '{}' added", feature.name());
        
        json json_output;
        json_output["name"] = feature.name();
        json_output["owner"] = evmc::hex(decodeReturnedValue<evmc::address>(owner_result.value()));
        json_output["local_address"] = evmc::hex(deploy_res.value());
        json_output["address"] = "0x0";

        response.setHeader(http::Header::Connection, "close");
        response.setHeader(http::Header::ContentType, "application/json");
        response.setCode(http::Code::Created);
        response.setBody(json_output.dump());
        co_return std::move(response);
    }


    asio::awaitable<http::Response> OPTIONS_transformation(const http::Request &, std::vector<RouteArg>)
    {
        http::Response response;
        response.setVersion("HTTP/1.1");
        response.setHeader(http::Header::AccessControlAllowOrigin, "*");
        response.setHeader(http::Header::AccessControlAllowMethods, "GET, POST, OPTIONS");
        response.setHeader(http::Header::AccessControlAllowHeaders, "Content-Type");
        response.setHeader(http::Header::Connection, "close");
        response.setHeader(http::Header::ContentType, "text/plain");
        response.setCode(http::Code::OK);
        co_return response;
    }

    asio::awaitable<http::Response> GET_transformation(const http::Request & request, std::vector<RouteArg> args, Registry & registry, EVM & evm)
    {
        http::Response response;
        response.setVersion("HTTP/1.1");
        response.setHeader(http::Header::Connection, "close");
        response.setHeader(http::Header::AccessControlAllowOrigin, "*");

        if(args.size() > 2 || args.size() == 0)
        {
            response.setHeader(http::Header::ContentType, "text/plain");
            response.setCode(http::Code::BadRequest);
            response.setBody("invalid url");
            co_return response;
        }

        auto transformation_name_result = parse::parseRouteArgAs<std::string>(args.at(0));

        if(!transformation_name_result)
        {
            response.setHeader(http::Header::ContentType, "text/plain");
            response.setCode(http::Code::BadRequest);
            response.setBody("invalid url");
            co_return response;
        }
        const auto & transformation_name = transformation_name_result.value();

        std::optional<Transformation> transformation_res;

        if(args.size() == 2)
        {
            const auto transformation_address_arg = parse::parseRouteArgAs<std::string>(args.at(1));

            if(!transformation_address_arg)
            {
                response.setHeader(http::Header::ContentType, "text/plain");
                response.setCode(http::Code::BadRequest);
                response.setBody("invalid url");
                co_return response;
            }

            const auto transformation_address_result = evmc::from_hex<evmc::address>(*transformation_address_arg);

            if(!transformation_address_result)
            {
                response.setHeader(http::Header::ContentType, "text/plain");
                response.setCode(http::Code::BadRequest);
                response.setBody("invalid url");
                co_return response;
            }

            transformation_res = co_await registry.getTransformation(transformation_name_result.value(), transformation_address_result.value());
        }
        else if(args.size() == 1)
        {
            transformation_res = co_await registry.getNewestTransformation(transformation_name_result.value());
        }

        if(!transformation_res) 
        {
            response.setHeader(http::Header::ContentType, "text/plain");
            response.setCode(http::Code::NotFound);
            response.setBody("transformation not found");
            co_return response;
        }
        
        auto json_res = parse::parseTransformationToJson(*transformation_res, parse::use_json);

        if(!json_res)
        {
            response.setHeader(http::Header::ContentType, "text/plain");
            response.setCode(http::Code::InternalServerError);
            response.setBody("internal server error");
            co_return response;
        }

        std::vector<uint8_t> input_data;
        // function selector
        const auto selector = constructFunctionSelector("getTransformation(string)");
        input_data.insert(input_data.end(), selector.begin(), selector.end());

        // Step 2: Offset to string data (32 bytes with value 0x20)
        std::vector<uint8_t> offset(32, 0);
        offset[31] = 0x20;
        input_data.insert(input_data.end(), offset.begin(), offset.end());

        // Step 3: String length
        std::vector<uint8_t> str_len(32, 0);
        str_len[31] = static_cast<uint8_t>(transformation_name.size());
        input_data.insert(input_data.end(), str_len.begin(), str_len.end());

        // Step 4: String bytes
        input_data.insert(input_data.end(), transformation_name.begin(), transformation_name.end());

        // Step 5: Padding to 32-byte boundary
        size_t padding = (32 - (transformation_name.size() % 32)) % 32;
        input_data.insert(input_data.end(), padding, 0);
        const auto exec_result = co_await evm.execute(evm.getRegistryAddress(), evm.getRegistryAddress(), input_data, 1'000'000, 0);
        
        // check execution status
        if(!exec_result)
        {
            spdlog::error("Failed to fetch transformation {}", exec_result.error());
            response.setHeader(http::Header::Connection, "close");
            response.setHeader(http::Header::ContentType, "text/plain");
            response.setCode(http::Code::InternalServerError);
            response.setBody(std::format("Failed to fetch transformation : {}", exec_result.error()));
            co_return std::move(response);
        }

        const auto transformation_address = decodeReturnedValue<evmc::address>(exec_result.value());
        const auto owner_result = co_await _fetchOwner(evm, transformation_address);
        if(!owner_result)
        {
            spdlog::error("Failed to fetch owner {}", owner_result.error());
            response.setHeader(http::Header::Connection, "close");
            response.setHeader(http::Header::ContentType, "text/plain");
            response.setCode(http::Code::InternalServerError);
            response.setBody(std::format("Failed to fetch owner : {}", owner_result.error()));
            co_return std::move(response);
        }

        const auto owner_address = decodeReturnedValue<evmc::address>(owner_result.value());

        (*json_res)["owner"] = evmc::hex(owner_address);
        (*json_res)["local_address"] = evmc::hex(transformation_address);
        (*json_res)["address"] = "0x0";

        response.setHeader(http::Header::ContentType, "application/json");
        response.setCode(http::Code::OK);
        response.setBody(json_res->dump());
        co_return std::move(response);
    }

    asio::awaitable<http::Response> POST_transformation(const http::Request & request, std::vector<RouteArg> args, AuthManager & auth_manager, Registry & registry, EVM & evm)
    {
        http::Response response;
        response.setVersion("HTTP/1.1");
        response.setHeader(http::Header::AccessControlAllowOrigin, "*");

        const auto auth_result = co_await _authenticate(request, auth_manager);

        if(auth_result.has_value() == false)
        {
            response.setHeader(http::Header::Connection, "close");
            response.setHeader(http::Header::ContentType, "text/plain");
            response.setCode(http::Code::Unauthorized);
            response.setBody(std::format("Error: {}", auth_result.error()));
            co_return std::move(response);
        }
        const auto & address = auth_result.value();

        spdlog::debug(std::format("token verified address: {}", address));
        
        // every double-quote character (") is escaped with a backslash (\)
        auto transformation_res = parse::parseJsonToTransformation(utils::escapeSolSrcQuotes(request.getBody()), parse::use_protobuf);

        if(!transformation_res) 
        {
            spdlog::error("Failed to parse transformation");
            response.setHeader(http::Header::Connection, "close");
            response.setHeader(http::Header::ContentType, "text/plain");
            response.setCode(http::Code::BadRequest);
            response.setBody("Failed to parse transformation");
            co_return std::move(response);
        }

        const Transformation & transformation = *transformation_res;
        if(transformation.name().empty() || transformation.sol_src().empty())
        {
            spdlog::error("Transformation name or source is empty");
            response.setHeader(http::Header::Connection, "close");
            response.setHeader(http::Header::ContentType, "text/plain");
            response.setCode(http::Code::BadRequest);
            response.setBody("Transformation name or source is empty");
            co_return std::move(response);
        }

        std::filesystem::path code_path = getResourcesPath() / "contracts" / "transformations" / (transformation.name() + ".sol");
        std::filesystem::path out_dir = getResourcesPath() / "contracts" / "transformations" / "build";

        std::filesystem::create_directories(code_path.parent_path());
        std::filesystem::create_directories(out_dir);

        // create code file
        std::ofstream out_file(code_path); 
        if(!out_file.is_open())
        {
            spdlog::error("Failed to create file");
            response.setHeader(http::Header::Connection, "close");
            response.setHeader(http::Header::ContentType, "text/plain");
            response.setCode(http::Code::InternalServerError);
            response.setBody("Failed to create file");
            co_return std::move(response);
        }

        out_file << constructTransformationSolidityCode(transformation);
        out_file.close();

        // compile code
        if(!co_await evm.compile(code_path, out_dir, getPTPath()/ "contracts", getPTPath() / "node_modules"))
        {
            spdlog::error("Failed to compile code");
            response.setHeader(http::Header::Connection, "close");
            response.setHeader(http::Header::ContentType, "text/plain");
            response.setCode(http::Code::InternalServerError);
            response.setBody("Failed to compile code");
            co_return std::move(response);
        }

        auto deploy_res = co_await evm.deploy(  
            out_dir / (transformation.name() + ".bin"), 
            address, 
            encodeAsArg(evm.getRegistryAddress()), 
            1000000, 
            0);
        
        if(!deploy_res)
        {
            spdlog::error("Failed to deploy code");
            response.setHeader(http::Header::Connection, "close");
            response.setHeader(http::Header::ContentType, "text/plain");
            response.setCode(http::Code::InternalServerError);
            response.setBody("Failed to deploy code");
            co_return std::move(response);
        }
        const auto owner_result = co_await _fetchOwner(evm, deploy_res.value());

        // check execution status
        if(!owner_result)
        {
            spdlog::error("Failed to fetch owner {}", owner_result.error());
            response.setHeader(http::Header::Connection, "close");
            response.setHeader(http::Header::ContentType, "text/plain");
            response.setCode(http::Code::InternalServerError);
            response.setBody(std::format("Failed to fetch owner : {}", owner_result.error()));
            co_return std::move(response);
        }

        if(!co_await registry.addTransformation(deploy_res.value(), transformation, code_path)) 
        {
            spdlog::error("Failed to add transformation to registry");
            response.setHeader(http::Header::Connection, "close");
            response.setHeader(http::Header::ContentType, "text/plain");
            response.setCode(http::Code::BadRequest);
            response.setBody("Failed to add transformation");
            co_return std::move(response);
        }

        spdlog::debug("transformation '{}' added", transformation.name());

        json json_output;
        json_output["name"] = transformation.name();
        json_output["owner"] = evmc::hex(decodeReturnedValue<evmc::address>(owner_result.value()));
        json_output["local_address"] = evmc::hex(deploy_res.value());
        json_output["address"] = "0x0";

        response.setHeader(http::Header::Connection, "close");
        response.setHeader(http::Header::ContentType, "application/json");
        response.setCode(http::Code::Created);
        response.setBody(json_output.dump());
        co_return std::move(response);
    }


    asio::awaitable<http::Response> GET_condition(const http::Request &)
    {
        
        http::Response response;
        response.setVersion("HTTP/1.1");
        response.setCode(http::Code::OK);
        response.setHeader(http::Header::Connection, "close");
        response.setHeader(http::Header::ContentType, "text/plain");
        response.setBody("OK");

        co_return std::move(response);
    }

    asio::awaitable<http::Response> POST_condition(const http::Request &)
    {
        http::Response response;
        response.setVersion("HTTP/1.1");
        response.setCode(http::Code::OK);
        response.setHeader(http::Header::Connection, "close");
        response.setHeader(http::Header::ContentType, "text/plain");
        response.setBody("OK");

        co_return std::move(response);
    }

    asio::awaitable<http::Response> GET_execute(const http::Request & request, std::vector<RouteArg> args, const AuthManager & auth_manager, const Registry & registry, EVM & evm)
    {
        http::Response response;
        response.setVersion("HTTP/1.1");
        response.setHeader(http::Header::Connection, "close");
        response.setHeader(http::Header::AccessControlAllowOrigin, "*");

        if(args.size() != 2 && args.size() != 3)
        {
            response.setHeader(http::Header::ContentType, "text/plain");
            response.setCode(http::Code::BadRequest);
            response.setBody("invalid url");
            co_return response;
        }

        const auto auth_result = co_await _authenticate(request, auth_manager);

        if(auth_result.has_value() == false)
        {
            response.setHeader(http::Header::Connection, "close");
            response.setHeader(http::Header::ContentType, "text/plain");
            response.setCode(http::Code::Unauthorized);
            response.setBody(std::format("Error: {}", auth_result.error()));
            co_return std::move(response);
        }
        const auto & address = auth_result.value();

        // input arguments - from url

        // name
        auto feature_name_result = parse::parseRouteArgAs<std::string>(args.at(0));
        if(!feature_name_result)
        {
            spdlog::error("invalid feature name");
            response.setHeader(http::Header::ContentType, "text/plain");
            response.setCode(http::Code::BadRequest);
            response.setBody("invalid feature name");
            co_return response;
        }
        const auto & feature_name = feature_name_result.value();

        // N
        const auto N_result = parse::parseRouteArgAs<std::uint32_t>(args.at(1));
        if(!N_result)
        {
            spdlog::error("invalid number of samples");
            response.setHeader(http::Header::ContentType, "text/plain");
            response.setCode(http::Code::BadRequest);
            response.setBody("invalid number of samples");
            co_return response;
        }
        const std::uint32_t & N = N_result.value();

        // running instaces
        std::vector<std::tuple<std::uint32_t, std::uint32_t>> running_instances;
        if(args.size() == 3)
        {
            auto running_instances_result = parse::parseRouteArgAs<std::vector<std::tuple<std::uint32_t, std::uint32_t>>>(args.at(2));
            if(!running_instances_result)
            {
                spdlog::error("invalid running instaces");
                response.setHeader(http::Header::ContentType, "text/plain");
                response.setCode(http::Code::BadRequest);
                response.setBody("invalid running instaces");
                co_return response;
            }
            running_instances = running_instances_result.value();
        }

        std::vector<uint8_t> input_data;
        // runner function selector
        const auto selector = constructFunctionSelector("gen(string,uint32,(uint32,uint32)[])");
        input_data.insert(input_data.end(), selector.begin(), selector.end());

        // 1. Offset to start of string data
        std::vector<uint8_t> offset_to_string(32, 0);
        offset_to_string[31] = 0x60;
        input_data.insert(input_data.end(), offset_to_string.begin(), offset_to_string.end());

        // 2. uint32 argument, properly encoded as a 32-byte word
        std::vector<std::uint8_t> N_bytes = encodeAsArg(N);
        input_data.insert(input_data.end(), N_bytes.begin(), N_bytes.end());

        // (String encoding)
        std::vector<std::uint8_t> name_bytes = encodeAsArg(feature_name);

        // 3. Offset to vector<tuple>, will be right after string
        std::vector<uint8_t> offset_tuple_vec(32, 0);
        size_t offset_to_tuple_vec = 0x60 + name_bytes.size();  // string offset + string dynamic payload
        offset_tuple_vec[28] = (offset_to_tuple_vec >> 24) & 0xFF;
        offset_tuple_vec[29] = (offset_to_tuple_vec >> 16) & 0xFF;
        offset_tuple_vec[30] = (offset_to_tuple_vec >> 8) & 0xFF;
        offset_tuple_vec[31] = (offset_to_tuple_vec) & 0xFF;
        input_data.insert(input_data.end(), offset_tuple_vec.begin(), offset_tuple_vec.end());

        // 4. Append the string bytes (dynamic)
        input_data.insert(input_data.end(), name_bytes.begin(), name_bytes.end());
        
        // 5. Append vector<tuple<uint32_t, uint32_t>> bytes (dynamic)
        std::vector<uint8_t> tuple_vec_bytes = encodeAsArg(running_instances);
        input_data.insert(input_data.end(), tuple_vec_bytes.begin(), tuple_vec_bytes.end());

        // execute call to runner
        const auto exec_result = co_await evm.execute(address, evm.getRunnerAddress(), input_data, 1'000'000, 0);
        
        // check execution status
        if(!exec_result)
        {
            spdlog::error("Failed to execute code {}", exec_result.error());
            response.setHeader(http::Header::Connection, "close");
            response.setHeader(http::Header::ContentType, "text/plain");
            response.setCode(http::Code::InternalServerError);
            response.setBody(std::format("Failed to execute code : {}", exec_result.error()));
            co_return std::move(response);
        }

        auto samples = decodeReturnedValue<std::vector<Samples>>(exec_result.value());

        json json_output = parse::parseToJson(samples, parse::use_json);
        response.setHeader(http::Header::Connection, "close");
        response.setHeader(http::Header::ContentType, "application/json");
        response.setCode(http::Code::Created);
        response.setBody(json_output.dump());
        co_return std::move(response);
    }
}