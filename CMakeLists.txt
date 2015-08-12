cmake_minimum_required(VERSION 2.8)

project(oshiya)
set(oshiya_VERSION_MAJOR 0)
set(oshiya_VERSION_MINOR 1)

#option(CONFIG_FILE "config file path" "~/oshiya.yml")
set(CONFIG_FILE "~/oshiya.yml" CACHE STRING "config file path")
set(STORAGE_DIR "~/" CACHE STRING "storage file directory")

# configuration header file
configure_file(
  ${PROJECT_SOURCE_DIR}/config.h.in
  ${PROJECT_SOURCE_DIR}/config.h
)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")

add_subdirectory(src)
add_subdirectory(test)

# Turn on verbose Makefiles for debugging
set(CMAKE_VERBOSE_MAKEFILE on CACHE BOOL "" FORCE)

message("System: ${CMAKE_SYSTEM_NAME}")
message("Compiler: ${CMAKE_CXX_COMPILER_ID}")
