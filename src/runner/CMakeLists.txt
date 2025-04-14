add_module(NAME "Runner"
    DEPENDENCIES
        "${PROJECT_PREFIX}::Native"
        "${PROJECT_PREFIX}::EVM"
        spdlog::spdlog
        asio
)

