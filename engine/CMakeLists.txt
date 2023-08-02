cmake_minimum_required(VERSION 3.22.0)

project(ShuraEngine CXX C)

file(GLOB_RECURSE SRC_FILES     LIST_DIRECTORIES false RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.c??)
file(GLOB_RECURSE HEADER_FILES  LIST_DIRECTORIES false RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.h)

include_directories((platform))

if(NOT SRC_FILES STREQUAL "")
    add_library(ShuraEngine ${SRC_FILES} ${HEADER_FILES})

    set_property(TARGET ShuraEngine PROPERTY CXX_STANDARD 11)
    set_property(TARGET ShuraEngine PROPERTY CXX_STANDARD_REQUIRED ON)
endif()
