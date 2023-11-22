# CMakeLists.txt
cmake_minimum_required(VERSION 3.5)

project(TcpMlpackExample)

set(CMAKE_CXX_STANDARD 11)

# Set NS-3 directory (replace this with your actual NS-3 directory)
set(NS3_DIR "/usr/local/Cellar/ns-3/3.40")

# Set mlpack directory (replace this with your actual mlpack directory)
set(MLPACK_DIR "/usr/local/Cellar/mlpack/4.2.1")

# Include the NS-3 build system
set(CMAKE_MODULE_PATH ${NS3_DIR}/build/cmake/Modules ${CMAKE_MODULE_PATH})
find_package(NS3 REQUIRED)

# Add the NS-3 include directories
include_directories(${NS3_INCLUDE_DIRS})

# Add mlpack include directory
include_directories(${MLPACK_DIR}/include)

# Define the target executable
add_executable(tcp_mlpack_example tcp_mlpack_example.cc)

# Link the target executable with NS-3 libraries and mlpack
target_link_libraries(tcp_mlpack_example ${NS3_LIBRARIES} ${MLPACK_DIR}/lib/libmlpack.so)
 