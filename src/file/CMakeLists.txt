add_module(NAME "File"
    DEPENDENCIES
        "${PROJECT_PREFIX}::Native"
        spdlog::spdlog
        asio
)

