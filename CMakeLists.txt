cmake_minimum_required (VERSION 3.8)
project (GNSS_Parser VERSION 0.0.1 LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

file(GLOB src ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

add_library(GNSS_Parser SHARED ${src})
add_executable(GNSS_Parser_Test ${src})