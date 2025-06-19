#include "transformation.hpp"

namespace dcn
{



    std::string constructTransformationSolidityCode(const Transformation & transformation)
    {
        /* ------------- EXAMPLE -------------
        // SPDX-License-Identifier: GPL-3.0

        pragma solidity >=0.7.0 <0.9.0;

        import "../TransformationBase.sol";

        contract Add is TransformationBase
        {
            constructor(address registryAddr) TransformationBase(registryAddr, "Add", 1)
            {}

            function run(uint32 x, uint32 [] calldata args) view external returns (uint32)
            {
                require(args.length == this.getArgsCount(), "wrong number of arguments");
                return x + args[0];
            }
        }
        */

        std::uint32_t argc = 1;
        return  "//SPDX-License-Identifier: MIT\n"
                "pragma solidity ^0.8.0;\n"
                "import \"transformation/TransformationBase.sol\";\n"
                "contract " + transformation.name() + " is TransformationBase{\n" // open contract
                "constructor(address registryAddr) TransformationBase(registryAddr, \"" + transformation.name() + "\"," +  std::to_string(argc) +"){}\n"
                "function run(uint32 x, uint32 [] calldata args) view external returns (uint32){\n" // open function
                "require(args.length == this.getArgsCount(), \"wrong number of arguments\");\n"
                + transformation.sol_src() + "\n}" // close function
                "\n}"; // close contract
    }
}

namespace dcn::parse
{
    std::optional<json> parseTransformationToJson(Transformation transformation, use_json_t)
    {
        json json_obj;

        json_obj["name"] = transformation.name();
        json_obj["sol_src"] = transformation.sol_src();

        return json_obj;
    }

    std::optional<Transformation> parseJsonToTransformation(json json_obj, use_json_t)
    {
        Transformation transformation;

        if (json_obj.contains("name")) {
            transformation.set_name(json_obj["name"].get<std::string>());
        }
        else return std::nullopt;
        

        if (json_obj.contains("sol_src")) {
            transformation.set_sol_src(json_obj["sol_src"].get<std::string>());
        }
        else return std::nullopt;
        

        return transformation;
    }

    std::optional<std::string> parseTransformationToJson(Transformation transformation, use_protobuf_t)
    {
        google::protobuf::util::JsonPrintOptions options;
        options.add_whitespace = true;
        options.preserve_proto_field_names = true; // Use snake_case from proto
        options.always_print_fields_with_no_presence = true;

        std::string json_str;
        auto status = google::protobuf::util::MessageToJsonString(transformation, &json_str, options);

        if (!status.ok()) return std::nullopt;

        return json_str;
    }

    std::optional<Transformation> parseJsonToTransformation(std::string json_str, use_protobuf_t)
    {
        google::protobuf::util::JsonParseOptions options;

        Transformation transformation;

        auto status = google::protobuf::util::JsonStringToMessage(json_str, &transformation, options);

        if(!status.ok()) return std::nullopt;

        return transformation;
    }
}