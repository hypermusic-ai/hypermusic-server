include("${PROJECT_SOURCE_DIR}/cmake/add_module.cmake")

add_module(NAME "Server"
    DEPENDENCIES
        "${PROJECT_PREFIX}::Native"
        "${PROJECT_PREFIX}::Session"
        absl::hash
        spdlog::spdlog
        asio
        CURL::libcurl
)

