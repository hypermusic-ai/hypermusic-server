cmake_minimum_required(VERSION 3.30)

include(FetchContent)
include(ExternalProject)


function(silence_warnings)
    set(options)
    set(oneValueArgs)
    set(multiValueArgs TARGETS)
    cmake_parse_arguments(silence_warnings "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN} )

    foreach(tgt ${silence_warnings_TARGETS})
        get_target_property(alias_actual ${tgt} ALIASED_TARGET)
        if(alias_actual)
            set(real_target ${alias_actual})
        else()
            set(real_target ${tgt})
        endif()

        if(NOT TARGET ${real_target})
            message(FATAL_ERROR "Target ${real_target} not found, cannot silence warnings.")
        endif()

        get_target_property(type ${real_target} TYPE)

        if("${type}" STREQUAL "INTERFACE_LIBRARY")
            target_compile_options(${real_target} INTERFACE /W0)
        else()
            target_compile_options(${real_target} PRIVATE /W0)
        endif()
    endforeach()
endfunction()


message(STATUS "Dependencies path not specified, Fetching from repositories ...")
    
set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build static libs only")

# ---------------------------------------------------------
# OpenSSL
# ---------------------------------------------------------
message(STATUS "Fetching dependency `OpenSSL` ...")
find_package(OpenSSL)

# ---------------------------------------------------------
# spdlog
# ---------------------------------------------------------
message(STATUS "Fetching dependency `spdlog` ...")
FetchContent_Declare(
    spdlog
    GIT_REPOSITORY  "https://github.com/gabime/spdlog.git"
    GIT_TAG         "v1.11.0"
    SYSTEM
    OVERRIDE_FIND_PACKAGE
)
FetchContent_MakeAvailable(spdlog)

# ---------------------------------------------------------
# asio
# ---------------------------------------------------------
message(STATUS "Fetching dependency `asio` ...")
FetchContent_Declare(
    asio
    GIT_REPOSITORY "https://github.com/chriskohlhoff/asio.git"
    GIT_TAG        "asio-1-34-0"
    SYSTEM
    OVERRIDE_FIND_PACKAGE
)
FetchContent_MakeAvailable(asio)
add_library(asio INTERFACE)
target_compile_options(asio INTERFACE /wd4459)
target_include_directories(asio INTERFACE "${asio_SOURCE_DIR}/asio/include")
install(DIRECTORY ${asio_SOURCE_DIR}/asio/include/asio DESTINATION include)

# ---------------------------------------------------------
# abseil
# ---------------------------------------------------------
message(STATUS "Fetching dependency `abseil` ...")
FetchContent_Declare(
    abseil
    GIT_REPOSITORY "https://github.com/abseil/abseil-cpp.git"
    GIT_TAG "20250127.1"
    SYSTEM
    OVERRIDE_FIND_PACKAGE
)
set(ABSL_MSVC_STATIC_RUNTIME ON CACHE BOOL "" FORCE)
set(ABSL_ENABLE_INSTALL ON CACHE BOOL "" FORCE)
set(ABSL_PROPAGATE_CXX_STD ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(abseil)

silence_warnings(TARGETS 
    absl::algorithm
    absl::base
    absl::debugging
    absl::flat_hash_map
    absl::flags
    absl::memory
    absl::meta
    absl::numeric
    absl::random_random
    absl::strings
    absl::synchronization
    absl::time
    absl::utility
)

# ---------------------------------------------------------
# protobuf
# ---------------------------------------------------------
message(STATUS "Fetching dependency `protobuf` ...")
FetchContent_Declare(
        protobuf
        GIT_REPOSITORY "https://github.com/protocolbuffers/protobuf"
        GIT_TAG        "v30.2"
        SYSTEM
        OVERRIDE_FIND_PACKAGE
)
set(protobuf_MSVC_STATIC_RUNTIME ON CACHE BOOL "" FORCE)
set(protobuf_BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(protobuf_ABSL_PROVIDER "package" CACHE STRING "" FORCE)
set(absl_DIR "${abseil_SOURCE_DIR}" CACHE PATH "")
set(protobuf_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(protobuf_BUILD_CONFORMANCE OFF CACHE BOOL "" FORCE)
set(protobuf_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(protobuf)

# Suppress warnings for building protobuf
silence_warnings(TARGETS 
    protobuf::libprotobuf
    protobuf::libprotoc
    protobuf::protoc
    protobuf::protoc-gen-upb
    protobuf::protoc-gen-upb_minitable
    protobuf::protoc-gen-upbdefs
    protobuf::libupb
    protobuf::libprotobuf-lite
    utf8_range
    utf8_validity
)
# set protobuf protoc executable location
set(Protobuf_PROTOC_EXECUTABLE $<TARGET_FILE:protobuf::protoc> CACHE FILEPATH "" FORCE)
    
add_custom_target(check_protoc_version
    COMMAND "${Protobuf_PROTOC_EXECUTABLE}" --version
    COMMENT "Checking protoc version ${Protobuf_PROTOC_EXECUTABLE}"
    VERBATIM
)
    
# ---------------------------------------------------------
# json
# ---------------------------------------------------------
message(STATUS "Fetching dependency `json` ...")
FetchContent_Declare(
    json
    GIT_REPOSITORY  "https://github.com/nlohmann/json.git"
    GIT_TAG         "v3.12.0"
    SYSTEM
    OVERRIDE_FIND_PACKAGE
)
set(JSON_BuildTests OFF)
FetchContent_MakeAvailable(json)

# ---------------------------------------------------------
# jwt-cpp
# ---------------------------------------------------------
message(STATUS "Fetching dependency `jwt-cpp` ...")
FetchContent_Declare(
    jwt-cpp
    GIT_REPOSITORY  "https://github.com/Thalhammer/jwt-cpp.git"
    GIT_TAG         "v0.7.1"
    SYSTEM
    OVERRIDE_FIND_PACKAGE
)
set(JWT_BUILD_EXAMPLES OFF)
set(JWT_BUILD_TESTS OFF)
FetchContent_MakeAvailable(jwt-cpp)

# ---------------------------------------------------------
# secp256k1
# ---------------------------------------------------------
message(STATUS "Fetching dependency `secp256k1` ...")
FetchContent_Declare(
    secp256k1
    GIT_REPOSITORY  "https://github.com/bitcoin-core/secp256k1.git"
    GIT_TAG         "v0.6.0"
    SYSTEM
    OVERRIDE_FIND_PACKAGE
)
set(SECP256K1_DISABLE_SHARED ON)
set(SECP256K1_BUILD_BENCHMARK OFF)
set(SECP256K1_BUILD_TESTS OFF)
set(SECP256K1_BUILD_EXHAUSTIVE_TESTS OFF)
set(SECP256K1_BUILD_CTIME_TESTS OFF)
set(SECP256K1_ENABLE_MODULE_RECOVERY ON)
FetchContent_MakeAvailable(secp256k1)

# ---------------------------------------------------------
# solc
# ---------------------------------------------------------
message(STATUS "Fetching dependency `solidity` ...")
set(SOLC_VERSION "v0.8.30")
set(SOLC_EXE_NAME "solc-windows.exe")
set(SOLC_URL "https://github.com/ethereum/solidity/releases/download/${SOLC_VERSION}/${SOLC_EXE_NAME}")

set(SOLC_PREFIX "${CMAKE_BINARY_DIR}/_deps/solidity")
set(SOLC_DOWNLOAD_DIR "${SOLC_PREFIX}/download")
set(SOLC_INSTALL_DIR "${CMAKE_BINARY_DIR}/_install/solidity")

file(MAKE_DIRECTORY "${SOLC_INSTALL_DIR}/bin")
file(MAKE_DIRECTORY "${SOLC_INSTALL_DIR}/lib")
file(MAKE_DIRECTORY "${SOLC_INSTALL_DIR}/include")

message(STATUS "Downloading ${SOLC_EXE_NAME} from ${SOLC_URL}...")
ExternalProject_Add(solidity_bin
    URL             "${SOLC_URL}"
    DOWNLOAD_DIR    "${SOLC_DOWNLOAD_DIR}"
    PREFIX          "${SOLC_PREFIX}"
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_BINARY_DIR}/_deps/solidity/download/${SOLC_EXE_NAME}" "${SOLC_INSTALL_DIR}/bin/"
    LOG_DOWNLOAD ON
    DOWNLOAD_NO_EXTRACT ON
)
    
# Create an imported executable target
add_executable(solc_imported IMPORTED GLOBAL)
add_dependencies(solc_imported solidity_bin)

# Install solc binaries
install(DIRECTORY "${SOLC_INSTALL_DIR}/bin/"
        DESTINATION ${CMAKE_INSTALL_BINDIR})

# set solidity solc executable location
set(Solidity_SOLC_EXECUTABLE "${SOLC_EXE_NAME}" CACHE FILEPATH "Path to solc.exe" FORCE)

# Set the location of the executable
set_target_properties(solc_imported PROPERTIES
    IMPORTED_LOCATION "${Solidity_SOLC_EXECUTABLE}"
)

# ---------------------------------------------------------
# evmc
# ---------------------------------------------------------
message(STATUS "Fetching dependency `evmc` ...")
FetchContent_Declare(
    evmc
    GIT_REPOSITORY  "https://github.com/ethereum/evmc.git"
    GIT_TAG         "v12.1.0"
    SYSTEM
    OVERRIDE_FIND_PACKAGE
)
set(EVMC_INSTALL ON)
set(EVMC_TESTING OFF)
FetchContent_MakeAvailable(evmc)

# ---------------------------------------------------------
# evmone
# ---------------------------------------------------------
message(STATUS "Fetching dependency `evmone` ...")
set(EVMONE_VERSION "0.15.0")
set(EVMONE_ZIP_NAME "evmone-${EVMONE_VERSION}-windows-amd64.zip")
set(EVMONE_URL "https://github.com/ethereum/evmone/releases/download/v${EVMONE_VERSION}/${EVMONE_ZIP_NAME}")

set(EVMONE_PREFIX "${CMAKE_BINARY_DIR}/_deps/evmone")
set(EVMONE_ZIP_EXTRACT_DIR "${EVMONE_PREFIX}/src/evmone_bin")
set(EVMONE_INSTALL_DIR "${CMAKE_BINARY_DIR}/_install/evmone") # Use a binary-tree install location

file(MAKE_DIRECTORY "${EVMONE_INSTALL_DIR}/bin")
file(MAKE_DIRECTORY "${EVMONE_INSTALL_DIR}/lib")
file(MAKE_DIRECTORY "${EVMONE_INSTALL_DIR}/include")

ExternalProject_Add(evmone_bin
    URL ${EVMONE_URL}
    PREFIX ${EVMONE_PREFIX}
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND 
        ${CMAKE_COMMAND} -E copy "${EVMONE_ZIP_EXTRACT_DIR}/bin/evmone.dll" "${EVMONE_INSTALL_DIR}/bin/"
        && ${CMAKE_COMMAND} -E copy "${EVMONE_ZIP_EXTRACT_DIR}/lib/evmone.lib" "${EVMONE_INSTALL_DIR}/lib/"
        && ${CMAKE_COMMAND} -E copy_directory "${EVMONE_ZIP_EXTRACT_DIR}/include" "${EVMONE_INSTALL_DIR}/include"
    LOG_DOWNLOAD ON
)

# Create imported target
add_library(evmone IMPORTED STATIC GLOBAL)
add_dependencies(evmone evmone_bin)

set_target_properties(evmone PROPERTIES
    IMPORTED_LOCATION "${EVMONE_INSTALL_DIR}/lib/evmone.lib"
)

target_include_directories(evmone INTERFACE
    $<BUILD_INTERFACE:${EVMONE_INSTALL_DIR}/include>
)

# Install evmone binaries
install(DIRECTORY "${EVMONE_INSTALL_DIR}/bin/"
        DESTINATION ${CMAKE_INSTALL_BINDIR}
        FILES_MATCHING PATTERN "*.dll")

install(DIRECTORY "${EVMONE_INSTALL_DIR}/lib/"
        DESTINATION ${CMAKE_INSTALL_LIBDIR}
        FILES_MATCHING PATTERN "*.lib")

install(DIRECTORY "${EVMONE_INSTALL_DIR}/include/"
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

# ---------------------------------------------------------
# PT repository
# ---------------------------------------------------------
message(STATUS "Fetching dependency `PT` ...")
set(PT_REPO_PREFIX "${CMAKE_BINARY_DIR}/_deps/pt")
include(cmake/FetchPT.cmake)

set(PT_SOLIDITY_DIR "${PT_REPO_PREFIX}/solidity")
set(PT_ARTIFACTS_DIR "${PT_SOLIDITY_DIR}/artifacts")
set(PT_INSTALL_DIR "${CMAKE_BINARY_DIR}/_install/pt")
set(PT_BUILD_MARKER "${PT_SOLIDITY_DIR}/pt_build_success.txt")

message(STATUS "PT repository location : ${PT_REPO_PREFIX}")
message (STATUS "PT solidity location : ${PT_SOLIDITY_DIR}")
message (STATUS "PT artifacts location : ${PT_ARTIFACTS_DIR}")

file(MAKE_DIRECTORY "${PT_INSTALL_DIR}")
file(MAKE_DIRECTORY "${PT_INSTALL_DIR}/contracts")
file(MAKE_DIRECTORY "${PT_INSTALL_DIR}/node_modules")

add_custom_command(
    OUTPUT ${PT_BUILD_MARKER}
    COMMAND ${CMAKE_COMMAND} -E echo "Installing dependencies for Solidity contracts..." &&
            cd ${PT_SOLIDITY_DIR} &&
            npm install
            && ${CMAKE_COMMAND} -E touch ${PT_BUILD_MARKER}
            && ${CMAKE_COMMAND} -E copy_directory "${PT_SOLIDITY_DIR}/contracts" "${PT_INSTALL_DIR}/contracts"
            && ${CMAKE_COMMAND} -E copy_directory "${PT_SOLIDITY_DIR}/node_modules" "${PT_INSTALL_DIR}/node_modules"
    WORKING_DIRECTORY ${PT_SOLIDITY_DIR}
    COMMENT "Configuring PT repo"
)

add_custom_target(configure_pt_repo ALL DEPENDS ${PT_BUILD_MARKER})
add_dependencies(configure_pt_repo pt_repo)

install(DIRECTORY "${PT_INSTALL_DIR}/"
    DESTINATION "${CMAKE_INSTALL_PREFIX}/pt"
)

# Copy ABI or bytecode from artifacts if needed
#add_custom_command(
#    OUTPUT ${CMAKE_BINARY_DIR}/resources/abi/Runner.json
#    COMMAND ${CMAKE_COMMAND} -E copy
#            ${PT_ARTIFACTS_DIR}/contracts/Runner.sol/Runner.json
#            ${CMAKE_BINARY_DIR}/resources/abi/Runner.json
#    DEPENDS build_pt_contracts
#    COMMENT "Copying Runner ABI to resources"
#)

#add_custom_target(copy_pt_abi ALL
#    DEPENDS ${CMAKE_BINARY_DIR}/resources/abi/Runner.json
#)

#add_dependencies(${PROJECT_NAME} copy_pt_abi)


# ---------------------------------------------------------
# GTest
# ---------------------------------------------------------
if(HYPERMUSIC_BUILD_TESTS)
    message(STATUS "Fetching dependency `GTest` ...")
    FetchContent_Declare(
        googletest
        GIT_REPOSITORY "https://github.com/google/googletest"
        GIT_TAG        "v1.16.0"
        SYSTEM
    )
    FetchContent_MakeAvailable(googletest)
endif()
