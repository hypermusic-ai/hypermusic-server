cmake_minimum_required(VERSION 3.30)

enable_testing()

include(GoogleTest)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

set(PROJECT_NAME "HypermusicServerInstallPackageTests")
set(PROJECT_PREFIX "HypermusicServerInstallPackageTests")

project("${PROJECT_NAME}"   
    VERSION "0.0.1"
    DESCRIPTION "Hypermusic Server tests"
    HOMEPAGE_URL "https://github.com/hypermusic-ai/hypermusic-server"
    LANGUAGES CXX
)

add_executable("${PROJECT_NAME}"    
    "install_package.cpp"
)

find_package(GTest REQUIRED NO_DEFAULT_PATH PATHS "${HYPERMUSIC_DEPS_PATH}/GTest/1.16.0/lib/cmake/GTest")

find_package(spdlog REQUIRED NO_DEFAULT_PATH PATHS "${HYPERMUSIC_DEPS_PATH}/spdlog/1.15.1/lib/cmake/spdlog")
find_package(absl REQUIRED NO_DEFAULT_PATH PATHS "${HYPERMUSIC_DEPS_PATH}/abseil/20250127.1/lib/cmake/absl")
find_package(utf8_range REQUIRED NO_DEFAULT_PATH PATHS "${HYPERMUSIC_DEPS_PATH}/protobuf/30.1/lib/cmake/utf8_range")
find_package(Protobuf REQUIRED NO_DEFAULT_PATH PATHS "${HYPERMUSIC_DEPS_PATH}/protobuf/30.1/lib/cmake/protobuf")
find_program(Protobuf_PROTOC NAMES protoc PATHS "${HYPERMUSIC_DEPS_PATH}/protobuf/30.1/bin" NO_DEFAULT_PATH)
find_package(asio REQUIRED NO_DEFAULT_PATH PATHS "${HYPERMUSIC_DEPS_PATH}/asio/1.34.0/lib/cmake/asio")
find_package(CURL REQUIRED NO_DEFAULT_PATH PATHS "${HYPERMUSIC_DEPS_PATH}/curl/8.12.1/lib/cmake/CURL")
find_package(evmc REQUIRED NO_DEFAULT_PATH PATHS "${HYPERMUSIC_DEPS_PATH}/evmc/12.1.0/lib/cmake/evmc")
find_package(evmone REQUIRED NO_DEFAULT_PATH PATHS "${HYPERMUSIC_DEPS_PATH}/evmone/0.14.1/lib/cmake/evmone")
find_package(HypermusicServer REQUIRED NO_DEFAULT_PATH PATHS "../../install/lib/cmake/HypermusicServer")

target_link_libraries("${PROJECT_NAME}"
    PRIVATE
        GTest::gtest_main
        spdlog::spdlog
        asio
        CURL::libcurl
        evmc::evmc
        evmone
        protobuf::libprotobuf
        HypermusicServer
)

set_target_properties("${PROJECT_NAME}" PROPERTIES CXX_STANDARD 23)

gtest_discover_tests("${PROJECT_NAME}")
