add_module(NAME "Parser"
    DEPENDENCIES
        "${PROJECT_PREFIX}::Native"
        spdlog::spdlog
        protobuf::libprotobuf
        asio
        json
)

