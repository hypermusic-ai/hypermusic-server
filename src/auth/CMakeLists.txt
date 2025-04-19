add_module(NAME "Auth"
    DEPENDENCIES
        "${PROJECT_PREFIX}::Utils"
        "${PROJECT_PREFIX}::Native"
        spdlog::spdlog
        asio
        absl::hash
        libsecp256k1::secp256k1
)

