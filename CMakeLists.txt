cmake_minimum_required(VERSION 3.27)

project(ATP_SERVER
    VERSION 0.1
    LANGUAGES CXX
)
# CMAKE Standard
set (CMAKE_CXX_STANDARD 17)
# Adding global flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic")

# including the header files
include_directories(include)
# including the source files
add_subdirectory(src)
