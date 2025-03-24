# TODO fallback for downloading external dependencies

include(FetchContent)

FetchContent_Declare(
    spdlog
    GIT_REPOSITORY  "https://github.com/gabime/spdlog.git"
    GIT_TAG         "v1.11.0"
)
FetchContent_MakeAvailable(spdlog)
set_target_properties(spdlog PROPERTIES MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

FetchContent_Declare(
    asio
    GIT_REPOSITORY "https://github.com/chriskohlhoff/asio.git"
    GIT_TAG        "asio-1-34-0"
)
FetchContent_MakeAvailable(asio)


set(protobuf_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(protobuf_BUILD_CONFORMANCE OFF CACHE BOOL "" FORCE)
FetchContent_Declare(
    protobuf
    GIT_REPOSITORY "https://github.com/protocolbuffers/protobuf.git"
    GIT_TAG        "v30.1"
)
FetchContent_MakeAvailable(protobuf)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /FS")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /FS")
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
add_link_options("/DEBUG")        # Enable debug information in the linker
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreadedDebug")  # Ensure Debug mode
target_compile_options(libprotoc PRIVATE         
"$<$<CONFIG:Debug>:/Od>"
"$<$<CONFIG:Debug>:/MTd>"
"$<$<CONFIG:Debug>:/Z7>")
#set_target_properties(libtest_common_lite PROPERTIES CXX_STANDARD 23)
set_target_properties(protoc PROPERTIES CXX_STANDARD 23)
set_target_properties(libprotoc PROPERTIES CXX_STANDARD 23)
set_target_properties(libprotobuf PROPERTIES CXX_STANDARD 23)
set_target_properties(libprotobuf-lite PROPERTIES CXX_STANDARD 23)
target_compile_options(libprotobuf  
    PUBLIC         
        /FS
)
target_compile_options(libprotobuf-lite  
PUBLIC         
        /FS
)
target_link_libraries(libprotobuf PRIVATE absl::strings absl::base)

if(BUILD_TESTING)
    message(STATUS "CMAKE_BUILD_TYPE ${CMAKE_BUILD_TYPE}")
    set(BUILD_GMOCK OFF CACHE BOOL "" FORCE)
    set(BUILD_GTEST ON CACHE BOOL "" FORCE)
    set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_Declare(
        gtest
        GIT_REPOSITORY  "https://github.com/google/googletest.git"
        GIT_TAG         "v1.16.0"
    )
    FetchContent_MakeAvailable(gtest)
    target_compile_options(gtest PRIVATE         
    "$<$<CONFIG:Debug>:/Od>"
    "$<$<CONFIG:Debug>:/MTd>"
    "$<$<CONFIG:Debug>:/Z7>")
    set_target_properties(gtest PROPERTIES MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
    set_target_properties(gtest PROPERTIES CXX_STANDARD 23)
endif()