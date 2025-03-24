set(MODULE_NAME "PT")

file(GLOB_RECURSE INCLUDE_SOURCES "include/*.h*")
file(GLOB_RECURSE SOURCES "src/*.c*")

add_library("${MODULE_NAME}"  
    OBJECT
        ${INCLUDE_SOURCES}
        ${SOURCES}
)

add_library("${PROJECT_PREFIX}::${MODULE_NAME}" ALIAS "${MODULE_NAME}")

target_include_directories("${MODULE_NAME}"     
    PUBLIC 
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
        "$<INSTALL_INTERFACE:${CMAKE_INSTALL_PREFIX}/include>")

target_link_libraries("${MODULE_NAME}" 
    PUBLIC 
        spdlog::spdlog
        asio
)

set_target_properties("${MODULE_NAME}" PROPERTIES CXX_STANDARD 23)

install(TARGETS "${MODULE_NAME}"
    EXPORT "${PROJECT_PREFIX}Targets"
    LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
    ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
    RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
    INCLUDES DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}")

