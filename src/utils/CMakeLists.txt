include("${PROJECT_SOURCE_DIR}/cmake/add_module.cmake")

add_module(NAME "Utils"
    DEPENDENCIES
        "${PROJECT_PREFIX}::Native"
        spdlog::spdlog
        asio
)