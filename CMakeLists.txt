cmake_minimum_required(VERSION 3.22.0)

project(ShooraProject CXX C)

set(EXTERN_LIB_PATH ${CMAKE_CURRENT_SOURCE_DIR}/external)

set(CMAKE_CXX_STANDARD 11) # this does nothing for MSVC, use target_compile_options below

# Definitions
if(WIN32)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
    add_definitions(-DWIN32_LEAN_AND_MEAN)
    add_definitions(-DNOMINMAX)
    add_definitions(-DVK_USE_PLATFORM_WIN32_KHR)
    add_definitions(-DGLFW_EXPOSE_NATIVE_WIN32)
    add_definitions(-DWIN32)
endif()

option(USE_DEBUG "Enter debug mode" OFF)
if (USE_DEBUG)
  add_definitions(-D_DEBUG)
endif()

###### Find OpenGL
# find_package(OpenGL REQUIRED)

###### Assimp Stuff
# set(ASSIMP_NO_EXPORT            ON CACHE BOOL "")
# set(ASSIMP_BUILD_DRACO          OFF CACHE BOOL "")
# set(ASSIMP_BUILD_ASSIMP_TOOLS   OFF CACHE BOOL "")
# set(ASSIMP_BUILD_TESTS          OFF CACHE BOOL "")
# set(ASSIMP_INSTALL_PDB          OFF CACHE BOOL "")
# set(ASSIMP_BUILD_ALL_IMPORTERS_BY_DEFAULT OFF CACHE BOOL "")
# set(ASSIMP_BUILD_OBJ_IMPORTER   ON CACHE BOOL "")
# set(ASSIMP_BUILD_GLTF_IMPORTER  ON CACHE BOOL "")
# if(WIN32)
#     set(ASSIMP_BUILD_ZLIB       ON CACHE BOOL "")
# endif()
# add_subdirectory(external/assimp)

# if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
#     set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-ignored-attributes -Wno-defaulted-function-deleted -Wno-braced-scalar-init -Wno-writable-strings -Wno-format -Wno-deprecated-declarations -Wno-macro-redefined")
# endif ()

###### GLAD Stuff
# set(GLAD_SOURCES external/glad/src/glad.c)
# add_library(glad ${GLAD_SOURCES})

###### Include Directories ...
# include_directories(external/glfw/include)
# include_directories(external/glm)
# include_directories(external/glad/include)
# include_directories(external/stb)
# include_directories(external/assimp/include)
# include_directories($<TARGET_FILE_DIR:assimp>/../include) # there is one config.h file which gets build in assimp's build directory by cmake.
# include_directories(external/imgui)

# Verbose Mode. Prints out all the compile messages to the screen.
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -v")


include_directories($ENV{VULKAN_SDK}/Include)
# set(EXTERNAL_INCLUDE_DIR $ENV{CPP_LIBRARIES})
# include_directories(${EXTERNAL_INCLUDE_DIR}/glfw/include)
# include_directories(${EXTERNAL_INCLUDE_DIR}/glm)
# include_directories(${EXTERNAL_INCLUDE_DIR}/stb)
# include_directories(${EXTERNAL_INCLUDE_DIR}/imgui)

include_directories(engine)

set(LIBRARY_DIR ${CMAKE_SOURCE_DIR}/libs)
# Create an IMPORTED library target for your .lib file
# add_library(external_libraries STATIC IMPORTED)
# set_target_properties(external_libraries PROPERTIES IMPORTED_LOCATION ${LIBRARY_DIR}/glfw3.lib)

# if(WIN32)
#     set_property(TARGET zlibstatic    PROPERTY FOLDER "ThirdPartyLibraries")
# endif()
# set_property(TARGET glfw            PROPERTY FOLDER "ThirdPartyLibraries")
# set_property(TARGET glad            PROPERTY FOLDER "ThirdPartyLibraries")
# set_property(TARGET assimp          PROPERTY FOLDER "ThirdPartyLibraries")


# Include Common SharedUtils
add_subdirectory(game)
add_subdirectory(engine)
