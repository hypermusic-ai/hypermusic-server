include("${PROJECT_SOURCE_DIR}/cmake/add_module.cmake")

add_module(NAME "Session"
    DEPENDENCIES
        "${PROJECT_PREFIX}::Native"
        "${PROJECT_PREFIX}::HTTP"
        absl::hash
        spdlog::spdlog
        asio
)