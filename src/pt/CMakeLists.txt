add_module(NAME "PT"
    DEPENDENCIES
        "${PROJECT_PREFIX}::Native"
        spdlog::spdlog
        asio
)

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
endforeach()

# create target from generated files
add_library("PT_proto_gen" OBJECT ${PROTO_GEN_SRCS} ${PROTO_GEN_HDRS})
target_include_directories("PT_proto_gen" PUBLIC ${PROTO_DST_DIR} ${Protobuf_INCLUDE_DIR})
target_link_libraries("PT_proto_gen" PUBLIC protobuf::libprotobuf)

# link to PT target
target_link_libraries("PT" PUBLIC PT_proto_gen)
