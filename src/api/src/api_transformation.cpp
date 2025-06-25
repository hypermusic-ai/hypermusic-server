#include "api.hpp"

namespace dcn
{
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
        response.setBodyWithContentLength("OK");
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
            response.setBodyWithContentLength("invalid url");
            co_return response;
        }

        auto transformation_name_result = parse::parseRouteArgAs<std::string>(args.at(0));

        if(!transformation_name_result)
        {
            response.setHeader(http::Header::ContentType, "text/plain");
            response.setCode(http::Code::BadRequest);
            response.setBodyWithContentLength("invalid url");
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
                response.setBodyWithContentLength("invalid url");
                co_return response;
            }

            const auto transformation_address_result = evmc::from_hex<evmc::address>(*transformation_address_arg);

            if(!transformation_address_result)
            {
                response.setHeader(http::Header::ContentType, "text/plain");
                response.setCode(http::Code::BadRequest);
                response.setBodyWithContentLength("invalid url");
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
            response.setBodyWithContentLength("transformation not found");
            co_return response;
        }
        auto json_res = parse::parseToJson(*transformation_res, parse::use_json);

        if(!json_res)
        {
            response.setHeader(http::Header::ContentType, "text/plain");
            response.setCode(http::Code::InternalServerError);
            response.setBodyWithContentLength("internal server error");
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

        co_await evm.setGas(evm.getRegistryAddress(), 1000000000);
        const auto exec_result = co_await evm.execute(evm.getRegistryAddress(), evm.getRegistryAddress(), input_data, 1000000000, 0);

        // check execution status
        if(!exec_result)
        {
            spdlog::error("Failed to fetch transformation {}", exec_result.error());
            response.setHeader(http::Header::Connection, "close");
            response.setHeader(http::Header::ContentType, "text/plain");
            response.setCode(http::Code::InternalServerError);
            response.setBodyWithContentLength(std::format("Failed to fetch transformation : {}", exec_result.error()));
            co_return std::move(response);
        }

        const auto transformation_address = decodeReturnedValue<evmc::address>(exec_result.value());
        const auto owner_result = co_await fetchOwner(evm, transformation_address);
        if(!owner_result)
        {
            spdlog::error("Failed to fetch owner {}", owner_result.error());
            response.setHeader(http::Header::Connection, "close");
            response.setHeader(http::Header::ContentType, "text/plain");
            response.setCode(http::Code::InternalServerError);
            response.setBodyWithContentLength(std::format("Failed to fetch owner : {}", owner_result.error()));
            co_return std::move(response);
        }

        const auto owner_address = decodeReturnedValue<evmc::address>(owner_result.value());

        (*json_res)["owner"] = evmc::hex(owner_address);
        (*json_res)["local_address"] = evmc::hex(transformation_address);
        (*json_res)["address"] = "0x0";

        response.setHeader(http::Header::ContentType, "application/json");
        response.setCode(http::Code::OK);
        response.setBodyWithContentLength(json_res->dump());
        co_return std::move(response);
    }

    asio::awaitable<http::Response> POST_transformation(const http::Request & request, std::vector<RouteArg> args, AuthManager & auth_manager, Registry & registry, EVM & evm)
    {
        http::Response response;
        response.setVersion("HTTP/1.1");
        response.setHeader(http::Header::AccessControlAllowOrigin, "*");

        const auto auth_result = co_await authenticate(request, auth_manager);

        if(auth_result.has_value() == false)
        {
            response.setHeader(http::Header::Connection, "close");
            response.setHeader(http::Header::ContentType, "text/plain");
            response.setCode(http::Code::Unauthorized);
            response.setBodyWithContentLength(std::format("Error: {}", auth_result.error()));
            co_return std::move(response);
        }
        const auto & address = auth_result.value();

        spdlog::debug(std::format("token verified address: {}", address));
        
        // every double-quote character (") is escaped with a backslash (\)
        auto transformation_res = parse::parseFromJson<Transformation>(utils::escapeSolSrcQuotes(request.getBody()), parse::use_protobuf);

        if(!transformation_res) 
        {
            spdlog::error("Failed to parse transformation");
            response.setHeader(http::Header::Connection, "close");
            response.setHeader(http::Header::ContentType, "text/plain");
            response.setCode(http::Code::BadRequest);
            response.setBodyWithContentLength("Failed to parse transformation");
            co_return std::move(response);
        }

        const Transformation & transformation = *transformation_res;

        TransformationRecord transformation_record;
        transformation_record.set_code_path((getResourcesPath() / "contracts" / "transformations" / (transformation.name() + ".sol")).string());
        transformation_record.set_owner(evmc::hex(address));
        *transformation_record.mutable_transformation() = std::move(transformation);

        if(!co_await deployTransformation(evm, registry, transformation_record))
        {
            spdlog::error("Failed to deploy transformation");
            response.setHeader(http::Header::Connection, "close");
            response.setHeader(http::Header::ContentType, "text/plain");
            response.setCode(http::Code::BadRequest);
            response.setBodyWithContentLength("Failed to deploy transformation");
            co_return std::move(response);
        }

        json json_output;
        json_output["name"] = transformation_record.transformation().name();
        json_output["owner"] = transformation_record.owner();
        json_output["address"] = "0x0";

        response.setHeader(http::Header::Connection, "close");
        response.setHeader(http::Header::ContentType, "application/json");
        response.setCode(http::Code::Created);
        response.setBodyWithContentLength(json_output.dump());

        co_return std::move(response);
    }
}