cmake_minimum_required(VERSION 3.0.0)
project(SAFE_DYNAMIC_MEMORY VERSION 0.1.0)

include(CTest)
enable_testing()

set(LIB_NAME "sdm")

add_compile_options(-Wall -Wextra -Wpedantic)

 
add_library(LIB_NAME SHARED sdm.cpp)
set_property(TARGET LIB_NAME PROPERTY CXX_STANDARD 11)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)

add_executable(sdmtest test.cpp)

add_test(sdmtest sdmtest COMMAND "--verbose")

target_link_libraries(sdmtest LIB_NAME)
