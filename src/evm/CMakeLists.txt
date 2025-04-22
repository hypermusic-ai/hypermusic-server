add_module(NAME "EVM"
    DEPENDENCIES
        "${PROJECT_PREFIX}::Utils"
        "${PROJECT_PREFIX}::Native"
        spdlog::spdlog
        asio
        evmc::evmc
        evmone
)

