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

        if(MSVC)
            if("${type}" STREQUAL "INTERFACE_LIBRARY")
                target_compile_options(${real_target} INTERFACE /W0)
            else()
                target_compile_options(${real_target} PRIVATE /W0)
            endif()
        elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
            if("${type}" STREQUAL "INTERFACE_LIBRARY")
                target_compile_options(${real_target} INTERFACE -w)
            else()
                target_compile_options(${real_target} PRIVATE -w)
            endif()
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
silence_warnings(TARGETS spdlog::spdlog)

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
if(WIN32)
    target_compile_options(asio INTERFACE /wd4459)
endif()
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
silence_warnings(TARGETS nlohmann_json)

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
silence_warnings(TARGETS jwt-cpp::jwt-cpp)

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
silence_warnings(TARGETS secp256k1)

# ---------------------------------------------------------
# solc
# ---------------------------------------------------------
message(STATUS "Fetching dependency `solc` ...")
set(SOLC_VERSION "v0.8.30")

if(WIN32)
    set(SOLC_EXE_NAME "solc-windows.exe")
elseif(UNIX)
    set(SOLC_EXE_NAME "solc-static-linux")
endif()

set(SOLC_URL "https://github.com/ethereum/solidity/releases/download/${SOLC_VERSION}/${SOLC_EXE_NAME}")

set(SOLC_PREFIX "${CMAKE_BINARY_DIR}/_deps/solc")
set(SOLC_DOWNLOAD_DIR "${SOLC_PREFIX}/download")
set(SOLC_INSTALL_DIR "${CMAKE_BINARY_DIR}/_install/solc")
set(SOLC_CONFIG_MARKER "${SOLC_INSTALL_DIR}/solc_config_success.txt")

file(MAKE_DIRECTORY "${SOLC_INSTALL_DIR}/bin")
file(MAKE_DIRECTORY "${SOLC_INSTALL_DIR}/lib")
file(MAKE_DIRECTORY "${SOLC_INSTALL_DIR}/include")

message(STATUS "Downloading ${SOLC_EXE_NAME} from ${SOLC_URL}...")
ExternalProject_Add(solc_download
    URL             ${SOLC_URL}
    DOWNLOAD_DIR    ${SOLC_DOWNLOAD_DIR}
    PREFIX          ${SOLC_PREFIX}
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND ""
    LOG_DOWNLOAD ON
    DOWNLOAD_NO_EXTRACT ON
)

if(WIN32)
    add_custom_command(
        OUTPUT ${SOLC_CONFIG_MARKER}
        COMMAND ${CMAKE_COMMAND} -E copy "${SOLC_DOWNLOAD_DIR}/${SOLC_EXE_NAME}" "${SOLC_INSTALL_DIR}/bin/${SOLC_EXE_NAME}"
        COMMAND ${CMAKE_COMMAND} -E touch ${SOLC_CONFIG_MARKER}
        WORKING_DIRECTORY ${SOLC_PREFIX}
        COMMENT "Configuring Solc compiler"
    )
elseif(UNIX)
    add_custom_command(
        OUTPUT ${SOLC_CONFIG_MARKER}
        COMMAND ${CMAKE_COMMAND} -E copy "${SOLC_DOWNLOAD_DIR}/${SOLC_EXE_NAME}" "${SOLC_INSTALL_DIR}/bin/"
        COMMAND chmod +x "${SOLC_INSTALL_DIR}/bin/${SOLC_EXE_NAME}"
        COMMAND ${CMAKE_COMMAND} -E touch ${SOLC_CONFIG_MARKER}
        WORKING_DIRECTORY ${SOLC_PREFIX}
        COMMENT "Configuring Solc compiler"
    )
endif()

add_custom_target(solc_configure ALL DEPENDS ${SOLC_CONFIG_MARKER})

# Create an imported executable target
add_executable(solc_imported IMPORTED GLOBAL)
add_dependencies(solc_imported solc_download solc_configure)

# Install solc binaries
install(FILES "${SOLC_INSTALL_DIR}/bin/${SOLC_EXE_NAME}"
    DESTINATION ${CMAKE_INSTALL_BINDIR}
    PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
                GROUP_READ GROUP_EXECUTE
                WORLD_READ WORLD_EXECUTE)

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
silence_warnings(TARGETS evmc::evmc)

# ---------------------------------------------------------
# evmone
# ---------------------------------------------------------
message(STATUS "Fetching dependency `evmone` ...")
set(EVMONE_MAJOR_VERSION "0")
set(EVMONE_MINOR_VERSION "15")
set(EVMONE_PATCH_VERSION "0")
set(EVMONE_VERSION "${EVMONE_MAJOR_VERSION}.${EVMONE_MINOR_VERSION}.${EVMONE_PATCH_VERSION}")

if(WIN32)
    set(EVMONE_ARCHIVE_NAME "evmone-${EVMONE_VERSION}-windows-amd64.zip")
elseif(UNIX)
    set(EVMONE_ARCHIVE_NAME "evmone-${EVMONE_VERSION}-linux-x86_64.tar.gz")
endif()

set(EVMONE_URL "https://github.com/ethereum/evmone/releases/download/v${EVMONE_VERSION}/${EVMONE_ARCHIVE_NAME}")

set(EVMONE_PREFIX "${CMAKE_BINARY_DIR}/_deps/evmone")
set(EVMONE_ARCHIVE_EXTRACT_DIR "${EVMONE_PREFIX}/src/evmone_bin")
set(EVMONE_INSTALL_DIR "${CMAKE_BINARY_DIR}/_install/evmone") # Use a binary-tree install location
set(EVMONE_CONFIG_MARKER "${EVMONE_INSTALL_DIR}/evmone_config_success.txt")

file(MAKE_DIRECTORY "${EVMONE_INSTALL_DIR}/bin")
file(MAKE_DIRECTORY "${EVMONE_INSTALL_DIR}/lib")
file(MAKE_DIRECTORY "${EVMONE_INSTALL_DIR}/include")

ExternalProject_Add(evmone_bin_download
    URL ${EVMONE_URL}
    SOURCE_DIR ${EVMONE_ARCHIVE_EXTRACT_DIR}
    PREFIX ${EVMONE_PREFIX}
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND ""
    LOG_DOWNLOAD ON
)

# Create configuration target
add_custom_target(evmone_configure ALL DEPENDS ${EVMONE_CONFIG_MARKER})

# Create imported target
add_library(evmone IMPORTED STATIC GLOBAL)
add_dependencies(evmone evmone_bin_download evmone_configure)

if(WIN32)
    set(EVMONE_STATIC_LIBRARY "evmone.lib")
    set(EVMONE_STATIC_LIBRARY_PATH "${EVMONE_ARCHIVE_EXTRACT_DIR}/lib/${EVMONE_STATIC_LIBRARY}")

    set(EVMONE_SHARED_LIBRARY "evmone.dll")
    set(EVMONE_SHARED_LIBRARY_PATH "${EVMONE_ARCHIVE_EXTRACT_DIR}/bin/${EVMONE_SHARED_LIBRARY}")

    add_custom_command(
        OUTPUT ${EVMONE_CONFIG_MARKER}
        COMMAND cd "${EVMONE_PREFIX}"
            && ${CMAKE_COMMAND} -E echo  
                    "Copy extracted evmone includes"
                    "${EVMONE_ARCHIVE_EXTRACT_DIR}/include"
                    "to install directory"
                    "${EVMONE_INSTALL_DIR}/include"
            && ${CMAKE_COMMAND} -E copy_directory "${EVMONE_ARCHIVE_EXTRACT_DIR}/include" "${EVMONE_INSTALL_DIR}/include"
            && ${CMAKE_COMMAND} -E echo
                    "Copy extracted evmone libraries"
                    "${EVMONE_STATIC_LIBRARY_PATH} and ${EVMONE_SHARED_LIBRARY_PATH}"
                    "to install directories"
                    "${EVMONE_INSTALL_DIR}/lib/ and ${EVMONE_INSTALL_DIR}/bin/"
            && ${CMAKE_COMMAND} -E copy "${EVMONE_STATIC_LIBRARY_PATH}" "${EVMONE_INSTALL_DIR}/lib/"
            && ${CMAKE_COMMAND} -E copy "${EVMONE_SHARED_LIBRARY_PATH}" "${EVMONE_INSTALL_DIR}/bin/"
            && ${CMAKE_COMMAND} -E echo 
                    "Generate configurtaion marker ${EVMONE_CONFIG_MARKER}"
            && ${CMAKE_COMMAND} -E touch "${EVMONE_CONFIG_MARKER}"
        WORKING_DIRECTORY ${EVMONE_PREFIX}
        COMMENT "Configuring Evmone"
    )

    set_target_properties(evmone PROPERTIES
        IMPORTED_LOCATION "${EVMONE_INSTALL_DIR}/lib/${EVMONE_STATIC_LIBRARY}"         # evmone.lib
        INTERFACE_INCLUDE_DIRECTORIES "${EVMONE_INSTALL_DIR}/include"
    )

    install(FILES "${EVMONE_STATIC_LIBRARY_PATH}" DESTINATION ${CMAKE_INSTALL_LIBDIR})
    install(FILES "${EVMONE_SHARED_LIBRARY_PATH}" DESTINATION ${CMAKE_INSTALL_BINDIR})

elseif(UNIX)
    set(EVMONE_SHARED_LIBRARY "libevmone.so.${EVMONE_VERSION}")
    set(EVMONE_SHARED_LIBRARY_PATH "${EVMONE_ARCHIVE_EXTRACT_DIR}/lib/${EVMONE_SHARED_LIBRARY}")

    # Copy extracted libraries to install directory & Generate configuration marker
    add_custom_command(
        OUTPUT ${EVMONE_CONFIG_MARKER}
        COMMAND cd "${EVMONE_PREFIX}"
            && ${CMAKE_COMMAND} -E echo  
                    "Copy extracted evmone includes"
                    "${EVMONE_ARCHIVE_EXTRACT_DIR}/include"
                    "to install directory"
                    "${EVMONE_INSTALL_DIR}/include"
            && ${CMAKE_COMMAND} -E copy_directory "${EVMONE_ARCHIVE_EXTRACT_DIR}/include" "${EVMONE_INSTALL_DIR}/include"
            && ${CMAKE_COMMAND} -E echo
                    "Copy extracted evmone library"
                    "${EVMONE_SHARED_LIBRARY_PATH}"
                    "to install directory"
                    "${EVMONE_INSTALL_DIR}/lib/"
            && ${CMAKE_COMMAND} -E copy "${EVMONE_SHARED_LIBRARY_PATH}" "${EVMONE_INSTALL_DIR}/lib/"
            && ${CMAKE_COMMAND} -E echo 
                    "Generate configurtaion marker ${EVMONE_CONFIG_MARKER}"
            && ${CMAKE_COMMAND} -E touch "${EVMONE_CONFIG_MARKER}"
        WORKING_DIRECTORY ${EVMONE_PREFIX}
        COMMENT "Configuring Evmone"
    )

    set_target_properties(evmone PROPERTIES
        IMPORTED_LOCATION "${EVMONE_INSTALL_DIR}/lib/${EVMONE_SHARED_LIBRARY}"          # libevmone.so
        INTERFACE_INCLUDE_DIRECTORIES "${EVMONE_INSTALL_DIR}/include"
    )

    set(_symlink_script "
        execute_process(COMMAND \"${CMAKE_COMMAND}\" -E create_symlink
            \"libevmone.so.${EVMONE_VERSION}\"
            \"libevmone.so.${EVMONE_MAJOR_VERSION}.${EVMONE_MINOR_VERSION}\"
            WORKING_DIRECTORY \"${CMAKE_INSTALL_LIBDIR}\")

        execute_process(COMMAND \"${CMAKE_COMMAND}\" -E create_symlink
            \"libevmone.so.${EVMONE_MAJOR_VERSION}.${EVMONE_MINOR_VERSION}\"
            \"libevmone.so.${EVMONE_MAJOR_VERSION}\"
            WORKING_DIRECTORY \"${CMAKE_INSTALL_LIBDIR}\")

        execute_process(COMMAND \"${CMAKE_COMMAND}\" -E create_symlink
            \"libevmone.so.${EVMONE_MAJOR_VERSION}\"
            \"libevmone.so\"
            WORKING_DIRECTORY \"${CMAKE_INSTALL_LIBDIR}\")
    ")

    install(FILES "${EVMONE_INSTALL_DIR}/lib/libevmone.so.${EVMONE_VERSION}" DESTINATION "${CMAKE_INSTALL_LIBDIR}")
    install(CODE "${_symlink_script}")

endif()

target_include_directories(evmone INTERFACE $<BUILD_INTERFACE:${EVMONE_INSTALL_DIR}/include>)

install(DIRECTORY "${EVMONE_INSTALL_DIR}/include/" DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})


# ---------------------------------------------------------
# PT repository
# ---------------------------------------------------------

if(HYPERMUSIC_USE_SUBMODULE_PT)
    message(STATUS "Using submodule `PT`")
    set(PT_REPO_PREFIX "${CMAKE_SOURCE_DIR}/submodule/pt")
else()
    message(STATUS "Fetching dependency `PT` ...")
    set(PT_REPO_PREFIX "${CMAKE_BINARY_DIR}/_deps/pt")
    include(cmake/FetchPT.cmake)
endif()

set(PT_SOLIDITY_DIR "${PT_REPO_PREFIX}/solidity")
set(PT_ARTIFACTS_DIR "${PT_SOLIDITY_DIR}/artifacts")
set(PT_INSTALL_DIR "${CMAKE_BINARY_DIR}/_install/pt")
set(PT_CONFIG_MARKER "${PT_INSTALL_DIR}/pt_config_success.txt")

message(STATUS "PT repository location : ${PT_REPO_PREFIX}")
message (STATUS "PT solidity location : ${PT_SOLIDITY_DIR}")
message (STATUS "PT artifacts location : ${PT_ARTIFACTS_DIR}")

file(MAKE_DIRECTORY "${PT_INSTALL_DIR}")
file(MAKE_DIRECTORY "${PT_INSTALL_DIR}/contracts")
file(MAKE_DIRECTORY "${PT_INSTALL_DIR}/node_modules")

add_custom_command(
    OUTPUT ${PT_CONFIG_MARKER}
    COMMAND ${CMAKE_COMMAND} -E echo "Configuring PT"
            && cd ${PT_SOLIDITY_DIR}
            && npm install
            && ${CMAKE_COMMAND} -E copy_directory "${PT_SOLIDITY_DIR}/contracts" "${PT_INSTALL_DIR}/contracts"
            && ${CMAKE_COMMAND} -E copy_directory "${PT_SOLIDITY_DIR}/node_modules" "${PT_INSTALL_DIR}/node_modules"
            && ${CMAKE_COMMAND} -E touch ${PT_CONFIG_MARKER}
    WORKING_DIRECTORY ${PT_SOLIDITY_DIR}
    COMMENT "Configuring PT repo"
)

add_custom_target(pt_configure ALL DEPENDS ${PT_CONFIG_MARKER})

if(NOT HYPERMUSIC_USE_SUBMODULE_PT)
    add_dependencies(pt_configure pt_download)
endif()

install(DIRECTORY "${PT_INSTALL_DIR}/"
    DESTINATION "${CMAKE_INSTALL_PREFIX}/pt"
)

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
