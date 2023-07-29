cmake_minimum_required(VERSION 3.10)

project(xeno)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

if ("${CMAKE_BUILD_TYPE}" STREQUAL "")
    set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Set the build type" FORCE)
endif()

add_subdirectory(deps/raylib)

add_executable(xeno
    main.cpp
)

target_link_libraries(xeno PUBLIC raylib)