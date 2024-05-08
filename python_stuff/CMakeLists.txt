cmake_minimum_required(VERSION 3.10)
project(shoora_python)

# Find Python
find_package(Python REQUIRED COMPONENTS Interpreter Development)

# Find NumPy
execute_process(COMMAND ${Python_EXECUTABLE} -c "import numpy; print(numpy.get_include())" OUTPUT_VARIABLE NUMPY_INCLUDE_DIR OUTPUT_STRIP_TRAILING_WHITESPACE)
find_path(NUMPY_INCLUDE_DIR numpy/arrayobject.h HINTS ${NUMPY_INCLUDE_DIR})

# Find pybind11
include_directories(extern/eigen)
add_subdirectory(extern/eigen)


add_subdirectory(extern/pybind11)

# Add the main executable
add_executable(python_example numpy_integration.cpp)

# Include directories
target_include_directories(python_example PRIVATE ${Python_INCLUDE_DIRS} ${NUMPY_INCLUDE_DIR})
target_link_libraries(python_example PRIVATE pybind11::embed)