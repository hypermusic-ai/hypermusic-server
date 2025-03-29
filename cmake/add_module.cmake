function(add_module)
    set(options)
    set(oneValueArgs NAME)
    set(multiValueArgs DEPENDENCIES)
    cmake_parse_arguments(add_module "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN} )

    file(GLOB_RECURSE INCLUDE_SOURCES "include/*.h*")
    file(GLOB_RECURSE SOURCES "src/*.c*")

    add_library("${add_module_NAME}"  
        OBJECT
            ${INCLUDE_SOURCES}
            ${SOURCES}
    )

    add_library("${PROJECT_PREFIX}::${add_module_NAME}" ALIAS "${add_module_NAME}")

    target_include_directories("${add_module_NAME}"     
        PUBLIC 
            "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
            "$<INSTALL_INTERFACE:${CMAKE_INSTALL_PREFIX}/include>"
    )

    target_link_libraries("${add_module_NAME}" 
        PUBLIC 
            ${add_module_DEPENDENCIES}
    )

    set_target_properties("${add_module_NAME}" PROPERTIES CXX_STANDARD 23)

    install(TARGETS "${add_module_NAME}"
        EXPORT "${PROJECT_PREFIX}Targets"
        LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
        ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
        RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
        INCLUDES DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}")

endfunction()