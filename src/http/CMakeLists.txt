include("${PROJECT_SOURCE_DIR}/cmake/add_module.cmake")

add_module(NAME "HTTP"
    DEPENDENCIES
        "${PROJECT_PREFIX}::Native"
        "${PROJECT_PREFIX}::Utils"

        absl::hash
        absl::flat_hash_map
        spdlog::spdlog
        asio
)