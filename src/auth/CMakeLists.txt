add_module(NAME "Auth"
    DEPENDENCIES
        "${PROJECT_PREFIX}::Utils"
        "${PROJECT_PREFIX}::Native"
        "${PROJECT_PREFIX}::Crypto"
        spdlog::spdlog
        asio
        absl::hash
        secp256k1
        jwt-cpp::jwt-cpp
        evmc::evmc
)

