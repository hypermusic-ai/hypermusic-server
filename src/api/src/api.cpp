#include "api.hpp"

namespace hm
{
     asio::awaitable<http::Response> OPTIONS_feature(const http::Request &, std::vector<RouteArg>)
    {
        http::Response response;
        response.setVersion("HTTP/1.1");
        response.setHeader(http::Header::AccessControlAllowOrigin, "*");
        response.setHeader(http::Header::AccessControlAllowMethods, "GET, POST, OPTIONS");
        response.setHeader(http::Header::AccessControlAllowHeaders, "Content-Type");
        response.setHeader(http::Header::Connection, "close");
        response.setHeader(http::Header::ContentType, "text/plain");
        response.setCode(hm::http::Code::OK);
        co_return response;
    }

    asio::awaitable<http::Response> GET_feature(const http::Request &, std::vector<RouteArg> args, Registry & registry)
    {
        http::Response response;
        response.setVersion("HTTP/1.1");
        response.setHeader(http::Header::Connection, "close");
        response.setHeader(http::Header::AccessControlAllowOrigin, "*");

        if(args.size() > 2 || args.size() == 0)
        {
            response.setHeader(http::Header::ContentType, "text/plain");
            response.setCode(hm::http::Code::BadRequest);
            response.setBody("invalid url");
            co_return response;
        }

        auto feature_name_result = parse::parseRouteArgAs<std::string>(args.at(0));

        if(!feature_name_result)
        {
            response.setHeader(http::Header::ContentType, "text/plain");
            response.setCode(hm::http::Code::BadRequest);
            response.setBody("invalid url");
            co_return response;
        }

        std::optional<Feature> feature_res;

        if(args.size() == 2)
        {
            auto feature_id_result = parse::parseRouteArgAs<std::size_t>(args.at(1));

            if(!feature_id_result)
            {
                response.setHeader(http::Header::ContentType, "text/plain");
                response.setCode(hm::http::Code::BadRequest);
                response.setBody("invalid url");
                co_return response;
            }

            feature_res = co_await registry.getFeature(feature_name_result.value(), feature_id_result.value());
        }
        else if(args.size() == 1)
        {
            feature_res = co_await registry.getNewestFeature(feature_name_result.value());
        }

        if(!feature_res) 
        {
            response.setHeader(http::Header::ContentType, "text/plain");
            response.setCode(hm::http::Code::NotFound);
            response.setBody("feature not found");
            co_return response;
        }
        
        auto json_res = parse::parseFeatureToJson(*feature_res, parse::use_protobuf);

        if(!json_res)
        {
            response.setHeader(http::Header::ContentType, "text/plain");
            response.setCode(hm::http::Code::InternalServerError);
            response.setBody("internal server error");
            co_return response;
        }

        response.setHeader(http::Header::ContentType, "application/json");
        response.setCode(hm::http::Code::OK);
        response.setBody(std::move(*json_res));
        co_return std::move(response);
    }

    asio::awaitable<http::Response> POST_feature(const http::Request & request, std::vector<RouteArg> args, AuthManager & auth_manager, Registry & registry, EVM & evm)
    {
        http::Response response;
        response.setVersion("HTTP/1.1");
        response.setHeader(http::Header::AccessControlAllowOrigin, "*");

        auto cookie_res = request.getHeader(http::Header::Cookie);
        if(cookie_res.empty())
        {
            response.setHeader(http::Header::Connection, "close");
            response.setHeader(http::Header::ContentType, "text/plain");
            response.setCode(hm::http::Code::Unauthorized);
            response.setBody("missing cookie");
            co_return std::move(response);
        }
        const std::string cookie_header = std::accumulate(cookie_res.begin(), cookie_res.end(), std::string(""));

        const auto token_res = parse::parseAccessTokenFromCookieHeader(cookie_header);
        if (token_res.has_value() == false) {
            response.setHeader(http::Header::Connection, "close");
            response.setHeader(http::Header::ContentType, "text/plain");
            response.setCode(hm::http::Code::Unauthorized);
            response.setBody("missing token");
            co_return std::move(response);
        }
        const std::string & token = token_res.value();

        auto verification_res = co_await auth_manager.verifyAccessToken(token);

        if(verification_res.has_value() == false)
        {
            response.setHeader(http::Header::Connection, "close");
            response.setHeader(http::Header::ContentType, "text/plain");
            response.setCode(hm::http::Code::Unauthorized);
            response.setBody("invalid token: " + verification_res.error());
            co_return std::move(response);
        }
        const auto & address = verification_res.value();

        spdlog::debug("token verified address : {}", address);

        // parse feature from json_string
        auto feature_res = parse::parseJsonToFeature(request.getBody(), parse::use_protobuf);

        if(!feature_res) 
        {
            response.setHeader(http::Header::Connection, "close");
            response.setHeader(http::Header::ContentType, "text/plain");
            response.setCode(hm::http::Code::BadRequest);
            response.setBody("Failed to parse feature");
            co_return std::move(response);
        }

        const Feature & feature = *feature_res;

        // create file with sol code
        // compile
        // deploy
        // add to EVM machine

        std::filesystem::path code_path = hm::getResourcesPath() / "contracts" / "features" / (feature.name() + ".sol");
        std::filesystem::path out_dir = hm::getResourcesPath() / "contracts" / "features" / "build";

        std::filesystem::create_directories(code_path.parent_path());
        std::filesystem::create_directories(out_dir);

        // create code file
        std::ofstream out_file(code_path); 
        if(!out_file.is_open())
        {
            spdlog::error("Failed to create file");
            response.setHeader(http::Header::Connection, "close");
            response.setHeader(http::Header::ContentType, "text/plain");
            response.setCode(hm::http::Code::InternalServerError);
            response.setBody("Failed to create file");
            co_return std::move(response);
        }

        //out_file << constructFeatureSolidityCode(feature.name(), feature.());
        out_file.close();

        // compile code
        if(!co_await evm.compile(code_path, out_dir))
        {
            spdlog::error("Failed to compile code");
            response.setHeader(http::Header::Connection, "close");
            response.setHeader(http::Header::ContentType, "text/plain");
            response.setCode(hm::http::Code::InternalServerError);
            response.setBody("Failed to compile code");
            co_return std::move(response);
        }

        // deploy code
        auto deploy_res = co_await evm.deploy(out_dir / (feature.name() + ".bin"), address, 1000000, 0);
        if(!deploy_res)
        {
            spdlog::error("Failed to deploy code : {}", deploy_res.error());
            response.setHeader(http::Header::Connection, "close");
            response.setHeader(http::Header::ContentType, "text/plain");
            response.setCode(hm::http::Code::InternalServerError);
            response.setBody("Failed to deploy code : " + deploy_res.error());
            co_return std::move(response);
        }

        auto version_res = co_await registry.addFeature(feature);
        if(!version_res) 
        {
            response.setHeader(http::Header::Connection, "close");
            response.setHeader(http::Header::ContentType, "text/plain");
            response.setCode(hm::http::Code::BadRequest);
            response.setBody("Failed to add feature");
            co_return std::move(response);
        }
        auto version = *version_res;

        spdlog::debug("feature '{}' added with hash : {}", feature.name(), std::to_string(version));
        
        json json_output;
        json_output["name"] = feature.name();
        json_output["version"] = std::to_string(version);
        json_output["address"] = deploy_res.value();

        response.setHeader(http::Header::Connection, "close");
        response.setHeader(http::Header::ContentType, "application/json");
        response.setCode(hm::http::Code::Created);
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
        response.setCode(hm::http::Code::OK);
        co_return response;
    }

    asio::awaitable<http::Response> GET_transformation(const http::Request & request, std::vector<RouteArg> args, Registry & registry)
    {
        http::Response response;
        response.setVersion("HTTP/1.1");
        response.setHeader(http::Header::Connection, "close");
        response.setHeader(http::Header::AccessControlAllowOrigin, "*");

        if(args.size() > 2 || args.size() == 0)
        {
            response.setHeader(http::Header::ContentType, "text/plain");
            response.setCode(hm::http::Code::BadRequest);
            response.setBody("invalid url");
            co_return response;
        }

        auto transformation_name_result = parse::parseRouteArgAs<std::string>(args.at(0));

        if(!transformation_name_result)
        {
            response.setHeader(http::Header::ContentType, "text/plain");
            response.setCode(hm::http::Code::BadRequest);
            response.setBody("invalid url");
            co_return response;
        }

        std::optional<Transformation> transformation_res;

        if(args.size() == 2)
        {
            auto transformation_id_result = parse::parseRouteArgAs<std::size_t>(args.at(1));

            if(!transformation_id_result)
            {
                response.setHeader(http::Header::ContentType, "text/plain");
                response.setCode(hm::http::Code::BadRequest);
                response.setBody("invalid url");
                co_return response;
            }

            transformation_res = co_await registry.getTransformation(transformation_name_result.value(), transformation_id_result.value());
        }
        else if(args.size() == 1)
        {
            transformation_res = co_await registry.getNewestTransformation(transformation_name_result.value());
        }

        if(!transformation_res) 
        {
            response.setHeader(http::Header::ContentType, "text/plain");
            response.setCode(hm::http::Code::NotFound);
            response.setBody("transformation not found");
            co_return response;
        }
        
        auto json_res = parse::parseTransformationToJson(*transformation_res, parse::use_protobuf);

        if(!json_res)
        {
            response.setHeader(http::Header::ContentType, "text/plain");
            response.setCode(hm::http::Code::InternalServerError);
            response.setBody("internal server error");
            co_return response;
        }

        response.setHeader(http::Header::ContentType, "application/json");
        response.setCode(hm::http::Code::OK);
        response.setBody(std::move(*json_res));
        co_return std::move(response);
    }

    asio::awaitable<http::Response> POST_transformation(const http::Request & request, std::vector<RouteArg> args, AuthManager & auth_manager, Registry & registry, EVM & evm)
    {
        http::Response response;
        response.setVersion("HTTP/1.1");
        response.setHeader(http::Header::AccessControlAllowOrigin, "*");

        auto cookie_res = request.getHeader(http::Header::Cookie);
        if(cookie_res.empty())
        {
            spdlog::error("missing cookie");
            response.setHeader(http::Header::Connection, "close");
            response.setHeader(http::Header::ContentType, "text/plain");
            response.setCode(hm::http::Code::Unauthorized);
            response.setBody("missing cookie");
            co_return std::move(response);
        }
        const std::string cookie_header = std::accumulate(cookie_res.begin(), cookie_res.end(), std::string(""));

        const auto token_res = parse::parseAccessTokenFromCookieHeader(cookie_header);
        if (token_res.has_value() == false) {
            spdlog::error("missing token");
            response.setHeader(http::Header::Connection, "close");
            response.setHeader(http::Header::ContentType, "text/plain");
            response.setCode(hm::http::Code::Unauthorized);
            response.setBody("missing token");
            co_return std::move(response);
        }
        const std::string & token = token_res.value();

        auto verification_res = co_await auth_manager.verifyAccessToken(token);
        if(verification_res.has_value() == false)
        {
            spdlog::error("invalid token: {}", verification_res.error());
            response.setHeader(http::Header::Connection, "close");
            response.setHeader(http::Header::ContentType, "text/plain");
            response.setCode(hm::http::Code::Unauthorized);
            response.setBody("invalid token: " + verification_res.error());
            co_return std::move(response);
        }
        const auto & address = verification_res.value();

        spdlog::debug("token verified address: {}", address);
        
        // every double-quote character (") is escaped with a backslash (\)
        auto transformation_res = parse::parseJsonToTransformation(escapeSolSrcQuotes(request.getBody()), parse::use_protobuf);

        if(!transformation_res) 
        {
            spdlog::error("Failed to parse transformation");
            response.setHeader(http::Header::Connection, "close");
            response.setHeader(http::Header::ContentType, "text/plain");
            response.setCode(hm::http::Code::BadRequest);
            response.setBody("Failed to parse transformation");
            co_return std::move(response);
        }

        const Transformation & transformation = *transformation_res;

        std::filesystem::path code_path = hm::getResourcesPath() / "contracts" / "transformations" / (transformation.name() + ".sol");
        std::filesystem::path out_dir = hm::getResourcesPath() / "contracts" / "transformations" / "build";

        std::filesystem::create_directories(code_path.parent_path());
        std::filesystem::create_directories(out_dir);

        // create code file
        std::ofstream out_file(code_path); 
        if(!out_file.is_open())
        {
            spdlog::error("Failed to create file");
            response.setHeader(http::Header::Connection, "close");
            response.setHeader(http::Header::ContentType, "text/plain");
            response.setCode(hm::http::Code::InternalServerError);
            response.setBody("Failed to create file");
            co_return std::move(response);
        }

        out_file << constructTransformationSolidityCode(transformation);
        out_file.close();

        // compile code
        if(!co_await evm.compile(code_path, out_dir))
        {
            spdlog::error("Failed to compile code");
            response.setHeader(http::Header::Connection, "close");
            response.setHeader(http::Header::ContentType, "text/plain");
            response.setCode(hm::http::Code::InternalServerError);
            response.setBody("Failed to compile code");
            co_return std::move(response);
        }

        // deploy code
        auto deploy_res = co_await evm.deploy(out_dir / (transformation.name() + ".bin"), address, 1000000, 0);
        if(!deploy_res)
        {
            spdlog::error("Failed to deploy code");
            response.setHeader(http::Header::Connection, "close");
            response.setHeader(http::Header::ContentType, "text/plain");
            response.setCode(hm::http::Code::InternalServerError);
            response.setBody("Failed to deploy code");
            co_return std::move(response);
        }

        auto version_res = co_await registry.addTransformation(transformation);
        if(!version_res) 
        {
            spdlog::error("Failed to add transformation to registry");
            response.setHeader(http::Header::Connection, "close");
            response.setHeader(http::Header::ContentType, "text/plain");
            response.setCode(hm::http::Code::BadRequest);
            response.setBody("Failed to add transformation");
            co_return std::move(response);
        }
        auto version = *version_res;

        spdlog::debug("transformation '{}' added with hash : {}", transformation.name(), std::to_string(version));
        
        json json_output;
        json_output["name"] = transformation.name();
        json_output["version"] = std::to_string(version);
        json_output["address"] = deploy_res.value();

        response.setHeader(http::Header::Connection, "close");
        response.setHeader(http::Header::ContentType, "application/json");
        response.setCode(hm::http::Code::Created);
        response.setBody(json_output.dump());
        co_return std::move(response);
    }


    asio::awaitable<http::Response> GET_condition(const http::Request &)
    {
        
        http::Response response;
        response.setVersion("HTTP/1.1");
        response.setCode(hm::http::Code::OK);
        response.setHeader(http::Header::Connection, "close");
        response.setHeader(http::Header::ContentType, "text/plain");
        response.setBody("OK");

        co_return std::move(response);
    }

    asio::awaitable<http::Response> POST_condition(const http::Request &)
    {
        http::Response response;
        response.setVersion("HTTP/1.1");
        response.setCode(hm::http::Code::OK);
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

        if(args.size() > 2 || args.size() == 0)
        {
            response.setHeader(http::Header::ContentType, "text/plain");
            response.setCode(hm::http::Code::BadRequest);
            response.setBody("invalid url");
            co_return response;
        }

        auto receipent_result = parse::parseRouteArgAs<std::string>(args.at(0));

        if(!receipent_result)
        {
            spdlog::error("invalid receipent");
            response.setHeader(http::Header::ContentType, "text/plain");
            response.setCode(hm::http::Code::BadRequest);
            response.setBody("invalid receipent");
            co_return response;
        }
        const auto & receipent = receipent_result.value();

        auto cookie_res = request.getHeader(http::Header::Cookie);
        if(cookie_res.empty())
        {
            spdlog::error("missing cookie");
            response.setHeader(http::Header::Connection, "close");
            response.setHeader(http::Header::ContentType, "text/plain");
            response.setCode(hm::http::Code::Unauthorized);
            response.setBody("missing cookie");
            co_return std::move(response);
        }
        const std::string cookie_header = std::accumulate(cookie_res.begin(), cookie_res.end(), std::string(""));

        const auto token_res = parse::parseAccessTokenFromCookieHeader(cookie_header);
        if (token_res.has_value() == false) {
            spdlog::error("missing token");
            response.setHeader(http::Header::Connection, "close");
            response.setHeader(http::Header::ContentType, "text/plain");
            response.setCode(hm::http::Code::Unauthorized);
            response.setBody("missing token");
            co_return std::move(response);
        }
        const std::string & token = token_res.value();

        auto verification_res = co_await auth_manager.verifyAccessToken(token);
        if(verification_res.has_value() == false)
        {
            spdlog::error("invalid token: {}", verification_res.error());
            response.setHeader(http::Header::Connection, "close");
            response.setHeader(http::Header::ContentType, "text/plain");
            response.setCode(hm::http::Code::Unauthorized);
            response.setBody("invalid token: " + verification_res.error());
            co_return std::move(response);
        }
        const auto & address = verification_res.value();


        const auto selector = constructFunctionSelector("sayHello()");

        const auto exec_result = co_await evm.execute(address, receipent, selector, 1000, 0);
        // run code
        if(!exec_result)
        {
            spdlog::error("Failed to execute code {}", exec_result.error());
            response.setHeader(http::Header::Connection, "close");
            response.setHeader(http::Header::ContentType, "text/plain");
            response.setCode(hm::http::Code::InternalServerError);
            response.setBody("Failed to compile code " + exec_result.error());
            co_return std::move(response);
        }

        spdlog::debug("executed code: {}", exec_result.value());

        json json_output;
        json_output["output"] = decodeReturnedValueFromHex<std::string>(exec_result.value());

        response.setHeader(http::Header::Connection, "close");
        response.setHeader(http::Header::ContentType, "application/json");
        response.setCode(hm::http::Code::Created);
        response.setBody(json_output.dump());
        co_return std::move(response);
    }
}