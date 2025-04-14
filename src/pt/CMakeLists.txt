# Generate .pb.cc and .pb.h files using protoc
message(STATUS "Protobuf Compiler: ${Protobuf_PROTOC}")

set(PROTO_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/proto")
set(PROTO_DST_DIR "${CMAKE_CURRENT_BINARY_DIR}/generated")

set(PROTO_FILES 
    "feature.proto"
    "condition.proto"
    "transformation.proto"
)
make_directory(${PROTO_DST_DIR})

set(PROTO_GEN_SRCS "")
set(PROTO_GEN_HDRS "")
foreach(PROTO_FILE ${PROTO_FILES})
    get_filename_component(PROTO_NAME ${PROTO_FILE} NAME_WE)
    set(GEN_SRC "${PROTO_DST_DIR}/${PROTO_NAME}.pb.cc")
    set(GEN_HDR "${PROTO_DST_DIR}/${PROTO_NAME}.pb.h")

    list(APPEND PROTO_GEN_SRCS ${GEN_SRC})
    list(APPEND PROTO_GEN_HDRS ${GEN_HDR})

    add_custom_command(
        OUTPUT ${GEN_SRC} ${GEN_HDR}
        COMMAND ${Protobuf_PROTOC} --proto_path=${PROTO_SRC_DIR} --cpp_out=${PROTO_DST_DIR} "${PROTO_SRC_DIR}/${PROTO_FILE}"
        DEPENDS "${PROTO_SRC_DIR}/${PROTO_FILE}"
        COMMENT "Generating C++ files from ${PROTO_FILE}"
    )

    set_source_files_properties(${GEN_SRC} PROPERTIES COMPILE_FLAGS "/wd4100")

endforeach()

# create target from generated files
add_library("PT_proto_gen" OBJECT ${PROTO_GEN_SRCS} ${PROTO_GEN_HDRS})
target_include_directories("PT_proto_gen" PUBLIC 
    ${Protobuf_INCLUDE_DIR} 
    "$<BUILD_INTERFACE:${PROTO_DST_DIR}>"
    "$<INSTALL_INTERFACE:${CMAKE_INSTALL_PREFIX}/include/generated>"
)

target_link_libraries("PT_proto_gen"    
    PUBLIC
    protobuf::libprotobuf
    )

# create install target for generated files
install(DIRECTORY ${PROTO_DST_DIR}/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/generated
    FILES_MATCHING PATTERN "*.h*"  # Only install .h* files
)

# Create PT target
add_module(NAME "PT"
    DEPENDENCIES
        "${PROJECT_PREFIX}::Native"
        "${PROJECT_PREFIX}::Session"
        "${PROJECT_PREFIX}::HTTP"
        absl::hash
        spdlog::spdlog
        asio
        PT_proto_gen
)