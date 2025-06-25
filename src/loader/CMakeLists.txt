include("${PROJECT_SOURCE_DIR}/cmake/add_module.cmake")

add_module(NAME "Loader"
    DEPENDENCIES

    "${PROJECT_PREFIX}::Native"
    "${PROJECT_PREFIX}::PT"
    "${PROJECT_PREFIX}::Utils"
    "${PROJECT_PREFIX}::EVM"
    "${PROJECT_PREFIX}::Parser"
    "${PROJECT_PREFIX}::File"


    spdlog::spdlog
    asio
    evmc::evmc
)