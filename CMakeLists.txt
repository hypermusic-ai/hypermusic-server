cmake_minimum_required(VERSION 3.30)

set(PROJECT_NAME "HypermusicServer")
set(PROJECT_PREFIX "HypermusicServer")

project("${PROJECT_NAME}"   
    VERSION "0.0.1"
    DESCRIPTION "Hypermusic-Server"
    HOMEPAGE_URL "https://github.com/hypermusic-ai/hypermusic-server"
    LANGUAGES CXX
)
include("cmake/compile_options.cmake")

option(HYPERMUSIC_ENABLE_INSTALL "Enable install rule" OFF)
option(HYPERMUSIC_DEPS_PATH "Path to dependencies" "")

if(HYPERMUSIC_DEPS_PATH)
    message(STATUS "Dependencies path specified to ${HYPERMUSIC_DEPS_PATH}")
    if(NOT EXISTS "${HYPERMUSIC_DEPS_PATH}")
        message(FATAL_ERROR "Dependencies path does not exist")
    else()
        set(spdlog_DIR "${HYPERMUSIC_DEPS_PATH}/spdlog/1.15.1/lib/cmake/spdlog")
        find_package(spdlog REQUIRED NO_DEFAULT_PATH)

        set(absl_DIR "${HYPERMUSIC_DEPS_PATH}/abseil/20250127.1/lib/cmake/absl")
        find_package(absl REQUIRED NO_DEFAULT_PATH)

        set(utf8_range_DIR "${HYPERMUSIC_DEPS_PATH}/protobuf/30.1/lib/cmake/utf8_range")
        find_package(utf8_range REQUIRED NO_DEFAULT_PATH)

        set(protobuf_DIR "${HYPERMUSIC_DEPS_PATH}/protobuf/30.1/lib/cmake/protobuf")
        find_package(protobuf REQUIRED NO_DEFAULT_PATH)

        set(asio_DIR "${HYPERMUSIC_DEPS_PATH}/asio/1.34.0/lib/cmake/asio")
        find_package(asio REQUIRED NO_DEFAULT_PATH)

        set(CURL_DIR "${HYPERMUSIC_DEPS_PATH}/curl/8.12.1/lib/cmake/CURL")
        find_package(CURL REQUIRED NO_DEFAULT_PATH)

        set(evmc_DIR "${HYPERMUSIC_DEPS_PATH}/evmc/12.1.0/lib/cmake/evmc")
        find_package(evmc REQUIRED NO_DEFAULT_PATH)

        set(evmone_DIR "${HYPERMUSIC_DEPS_PATH}/evmone/0.14.1/lib/cmake/evmone")
        find_package(evmone REQUIRED NO_DEFAULT_PATH)

        if(BUILD_TESTING)
            set(GTest_DIR "${HYPERMUSIC_DEPS_PATH}/GTest/1.16.0/lib/cmake/GTest")
            find_package(GTest REQUIRED NO_DEFAULT_PATH)
        endif()

    endif()
else()
    message(STATUS "Dependencies path not specified, try to find it automatically")
endif()


add_subdirectory(src)
if(BUILD_TESTING)
    add_subdirectory(tests)
endif()

add_executable("${PROJECT_NAME}"    
    src/main.cpp)

target_include_directories("${PROJECT_NAME}"     
    PUBLIC 
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
        "$<INSTALL_INTERFACE:${CMAKE_INSTALL_PREFIX}/include>")

target_link_libraries("${PROJECT_NAME}"
    PRIVATE
        "${PROJECT_PREFIX}::Native"
        "${PROJECT_PREFIX}::Server"
        "${PROJECT_PREFIX}::HTTP"
        "${PROJECT_PREFIX}::Session"
        "${PROJECT_PREFIX}::PT"
        "${PROJECT_PREFIX}::EVM"
)

set_target_properties("${PROJECT_NAME}" PROPERTIES CXX_STANDARD 23)
