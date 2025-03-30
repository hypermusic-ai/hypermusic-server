message(STATUS "PACKAGE_PREFIX_DIR ${PACKAGE_PREFIX_DIR}")

set(HypermusicServer_INCLUDE_DIR "${CMAKE_CURRENT_LIST_DIR}/../../../include")
set(HypermusicServer_LIBRARY_DIR "${CMAKE_CURRENT_LIST_DIR}/../../../lib")
set(HypermusicServer_BIN_DIR "${CMAKE_CURRENT_LIST_DIR}/../../../bin")

find_library(HypermusicServer_LIBRARY NAMES HypermusicServerLib PATHS ${HypermusicServer_LIBRARY_DIR})

add_library(HypermusicServer INTERFACE)
target_include_directories(HypermusicServer INTERFACE ${HypermusicServer_INCLUDE_DIR})
target_include_directories(HypermusicServer INTERFACE ${HypermusicServer_INCLUDE_DIR}/generated)

target_link_libraries(HypermusicServer INTERFACE ${HypermusicServer_LIBRARY})
