add_module(NAME "API"
    DEPENDENCIES
        "${PROJECT_PREFIX}::Utils"
        "${PROJECT_PREFIX}::Native"
        "${PROJECT_PREFIX}::File"
        "${PROJECT_PREFIX}::PT"
        "${PROJECT_PREFIX}::Session"
        "${PROJECT_PREFIX}::HTTP"
        "${PROJECT_PREFIX}::Server"
        "${PROJECT_PREFIX}::Parser"
        "${PROJECT_PREFIX}::Auth"
        "${PROJECT_PREFIX}::EVM"

        spdlog::spdlog
        asio
        secp256k1
)

