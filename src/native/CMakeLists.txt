set(MODULE_NAME "Native")

file(GLOB INCLUDE_SOURCES "include/*.h*")
file(GLOB SOURCES "src/*.c*")

if(WIN32)
    message(STATUS "Configuring module Native for Windows")
    file(GLOB_RECURSE WINDOWS_INCLUDE_SOURCES "include/windows/*.h*")
    file(GLOB_RECURSE WINDOWS_SOURCES "src/windows/*.c*")

    list(APPEND INCLUDE_SOURCES ${WINDOWS_INCLUDE_SOURCES})
    list(APPEND SOURCES ${WINDOWS_SOURCES})
elseif(UNIX AND NOT APPLE)
    message(STATUS "Configuring module Native for Unix")
    file(GLOB_RECURSE UNIX_INCLUDE_SOURCES "include/unix/*.h*")
    file(GLOB_RECURSE UNIX_SOURCES "src/unix/*.c*")

    list(APPEND INCLUDE_SOURCES ${UNIX_INCLUDE_SOURCES})
    list(APPEND SOURCES ${UNIX_SOURCES})
elseif(APPLE)
    message(STATUS "Configuring module Native for macOS")
    file(GLOB_RECURSE MAC_INCLUDE_SOURCES "include/mac/*.h*")
    file(GLOB_RECURSE MAC_SOURCES "src/mac/*.c*")

    list(APPEND INCLUDE_SOURCES ${MAC_INCLUDE_SOURCES})
    list(APPEND SOURCES ${MAC_SOURCES})
else()
    message(FATAL_ERROR "Unsupported platform")
endif()

add_library("${MODULE_NAME}"  
    OBJECT
        ${INCLUDE_SOURCES}
        ${SOURCES}
)

target_sources("${MODULE_NAME}" PUBLIC
    FILE_SET public_headers
    TYPE HEADERS
    BASE_DIRS include
    FILES  ${INCLUDE_SOURCES}
)

add_library("${PROJECT_PREFIX}::${MODULE_NAME}" ALIAS "${MODULE_NAME}")

target_include_directories("${MODULE_NAME}"     
    PUBLIC 
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
        "$<INSTALL_INTERFACE:${CMAKE_INSTALL_PREFIX}/include>")

target_link_libraries("${MODULE_NAME}" 
    PUBLIC 
    spdlog::spdlog
)

set_target_properties("${MODULE_NAME}" PROPERTIES CXX_STANDARD 23)

# Install target
install(TARGETS ${MODULE_NAME}
        EXPORT ${MODULE_NAME}Targets
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
        FILE_SET public_headers
)

