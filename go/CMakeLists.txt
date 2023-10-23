cmake_minimum_required(VERSION 3.5)
project(hydra_go)

set(HYDRA_GO_SOURCES downloader.go)
set(LIB libhydra_go.a)

add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${LIB}
  DEPENDS ${HYDRA_GO_SOURCES}
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  COMMAND env GOPATH=${GOPATH} go build -buildmode=c-archive
  -o "${CMAKE_CURRENT_BINARY_DIR}/${LIB}"
  COMMENT "Building hydra_go library...")
add_custom_target(hydra_go ALL DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/${LIB})
add_library(hydra_go_static STATIC IMPORTED GLOBAL)
set_target_properties(hydra_go_static PROPERTIES
  IMPORTED_LOCATION ${CMAKE_CURRENT_BINARY_DIR}/${LIB}
  INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR})