add_module(NAME "API"
    DEPENDENCIES
        "${PROJECT_PREFIX}::Utils"
        "${PROJECT_PREFIX}::Native"
        "${PROJECT_PREFIX}::PT"
        "${PROJECT_PREFIX}::Session"
        "${PROJECT_PREFIX}::HTTP"
        "${PROJECT_PREFIX}::Server"
        "${PROJECT_PREFIX}::Parser"
        "${PROJECT_PREFIX}::Auth"
        spdlog::spdlog
        asio
        libsecp256k1::secp256k1
)

