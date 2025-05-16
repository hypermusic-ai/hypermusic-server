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

target_link_libraries("${PROJECT_NAME}"
    PRIVATE
        GTest::gtest_main
        spdlog::spdlog
        asio
        evmc::evmc
        evmone
        protobuf::libprotobuf
        HypermusicServer
)

set_target_properties("${PROJECT_NAME}" PROPERTIES CXX_STANDARD 23)

gtest_discover_tests("${PROJECT_NAME}")
