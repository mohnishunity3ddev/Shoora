cmake_minimum_required(VERSION 3.22.0)
project(ShuraEngine)

include(../cmake_macros/prac.cmake)
include_directories(external)

SETUP_WIN32_APP(ShuraEngine "ShuraEngine")


####NOTE: Get all files from a particular path
# get_filename_component(MY_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/external" ABSOLUTE)
# file(GLOB_RECURSE EXTERNAL_LIBS_SRC "${MY_SOURCE_DIR}/*.cpp" "${MY_SOURCE_DIR}/*.c")
# file(GLOB_RECURSE EXTERNAL_LIBS_HEADER "${MY_SOURCE_DIR}/*.hpp" "${MY_SOURCE_DIR}/*.h")

####NOTE: Import an already exisiting library
set(LIBS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/external)
add_library(extern STATIC IMPORTED)
set_target_properties(extern PROPERTIES IMPORTED_LOCATION ${LIBS_DIR}/volk/volk.lib)

# add_library(volk STATIC external/volk/volk.c external/volk/volk.h)
target_link_libraries(ShuraEngine PRIVATE extern)