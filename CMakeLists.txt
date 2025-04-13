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

message(STATUS "Install directory ${CMAKE_INSTALL_PREFIX}")
set(CMAKE_INSTALL_INCLUDEDIR "${CMAKE_INSTALL_PREFIX}/include")
set(CMAKE_INSTALL_LIBDIR "${CMAKE_INSTALL_PREFIX}/lib")
set(CMAKE_INSTALL_BINDIR "${CMAKE_INSTALL_PREFIX}/bin")
message(STATUS "include directory ${CMAKE_INSTALL_INCLUDEDIR}")
message(STATUS "lib directory ${CMAKE_INSTALL_LIBDIR}")
message(STATUS "bin directory ${CMAKE_INSTALL_BINDIR}")

if(HYPERMUSIC_DEPS_PATH)
    message(STATUS "Dependencies path specified to ${HYPERMUSIC_DEPS_PATH}")
    if(NOT EXISTS "${HYPERMUSIC_DEPS_PATH}")
        message(FATAL_ERROR "Dependencies path does not exist")
    else()
        find_package(spdlog REQUIRED NO_DEFAULT_PATH PATHS "${HYPERMUSIC_DEPS_PATH}/spdlog/1.15.1/lib/cmake/spdlog")

        find_package(absl REQUIRED NO_DEFAULT_PATH PATHS "${HYPERMUSIC_DEPS_PATH}/abseil/20250127.1/lib/cmake/absl")

        find_package(utf8_range REQUIRED NO_DEFAULT_PATH PATHS "${HYPERMUSIC_DEPS_PATH}/protobuf/30.1/lib/cmake/utf8_range")

        find_package(Protobuf REQUIRED NO_DEFAULT_PATH PATHS "${HYPERMUSIC_DEPS_PATH}/protobuf/30.1/lib/cmake/protobuf")
        find_program(Protobuf_PROTOC NAMES protoc PATHS "${HYPERMUSIC_DEPS_PATH}/protobuf/30.1/bin" NO_DEFAULT_PATH)

        find_package(asio REQUIRED NO_DEFAULT_PATH PATHS "${HYPERMUSIC_DEPS_PATH}/asio/1.34.0/lib/cmake/asio")

        find_package(CURL REQUIRED NO_DEFAULT_PATH PATHS "${HYPERMUSIC_DEPS_PATH}/curl/8.12.1/lib/cmake/CURL")

        find_package(evmc REQUIRED NO_DEFAULT_PATH PATHS "${HYPERMUSIC_DEPS_PATH}/evmc/12.1.0/lib/cmake/evmc")

        find_package(evmone REQUIRED NO_DEFAULT_PATH PATHS "${HYPERMUSIC_DEPS_PATH}/evmone/0.14.1/lib/cmake/evmone")

        if(BUILD_TESTING)
            find_package(GTest REQUIRED NO_DEFAULT_PATH PATHS "${HYPERMUSIC_DEPS_PATH}/GTest/1.16.0/lib/cmake/GTest")
        endif()

    endif()
else()
    message(STATUS "Dependencies path not specified, try to find it automatically")
endif()

add_subdirectory(src)

# create static library target
add_library("${PROJECT_NAME}Lib" STATIC "include/hypermusic.hpp")

target_include_directories("${PROJECT_NAME}Lib"     
    PUBLIC 
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
        "$<INSTALL_INTERFACE:${CMAKE_INSTALL_PREFIX}/include>")

target_link_libraries("${PROJECT_NAME}Lib"
    PUBLIC
        "${PROJECT_PREFIX}::Native"
        "${PROJECT_PREFIX}::Utils"
        "${PROJECT_PREFIX}::Server"
        "${PROJECT_PREFIX}::HTTP"
        "${PROJECT_PREFIX}::Session"
        "${PROJECT_PREFIX}::PT"
        PT_proto_gen
        "${PROJECT_PREFIX}::EVM"
)
set_target_properties("${PROJECT_NAME}Lib" PROPERTIES CXX_STANDARD 23)

target_sources("${PROJECT_NAME}Lib" PUBLIC
    FILE_SET public_headers
    TYPE HEADERS
    BASE_DIRS include
    FILES "include/hypermusic.hpp"
)

# static library install target
install(TARGETS ${PROJECT_NAME}Lib
    EXPORT ${PROJECT_NAME}LibTargets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    FILE_SET public_headers
)

# create executable target
add_executable("${PROJECT_NAME}Exe" src/main.cpp)

target_link_libraries("${PROJECT_NAME}Exe"
    PRIVATE
        "${PROJECT_NAME}Lib"
)

set_target_properties("${PROJECT_NAME}Exe" PROPERTIES CXX_STANDARD 23)

# executable install target
install(TARGETS ${PROJECT_NAME}Exe
    EXPORT ${PROJECT_NAME}ExeTargets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(FILES "$<TARGET_PDB_FILE:${PROJECT_NAME}Exe>"
    DESTINATION ${CMAKE_INSTALL_BINDIR}
    OPTIONAL
)

# Configure the Config file
include(CMakePackageConfigHelpers)
configure_package_config_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/HypermusicServer/HypermusicServerConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/cmake/HypermusicServer/HypermusicServerConfig.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/HypermusicServer
)
# Install the generated config file
install(FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/HypermusicServer/HypermusicServerTargets.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/cmake/HypermusicServer/HypermusicServerConfig.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/HypermusicServer
)

# build tests
if(BUILD_TESTING)
    add_subdirectory(tests)
endif()