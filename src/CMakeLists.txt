cmake_minimum_required(VERSION 3.22.0)

project(shoora)

include(../cmake_macros/prac.cmake)

SETUP_APP(shoora "shoora")

target_link_libraries(shoora external_libraries)

if(TARGET SharedUtils)
    target_link_libraries(shoora SharedUtils)
endif()

# Set the CWD of the VS 2022 Project
set_target_properties(shoora PROPERTIES
    VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/assets"
)