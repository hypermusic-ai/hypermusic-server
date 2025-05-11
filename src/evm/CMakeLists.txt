add_module(NAME "EVM"
    DEPENDENCIES
        "${PROJECT_PREFIX}::Utils"
        "${PROJECT_PREFIX}::Native"
        absl::hash
        absl::flat_hash_map
        spdlog::spdlog
        asio
        evmc::evmc
        evmone
)

