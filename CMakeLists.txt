cmake_minimum_required(VERSION 3.0.0)

project(SAF_ALO VERSION 0.1.0)


set(CMAKE_CXX_STANDARD 14)

add_compile_options(-Wall -Wextra -Wpedantic)


set(LIB_NAME "safalo")
add_library(${LIB_NAME} SHARED safalo.cpp)


#### testing 
enable_testing()
find_package(GTest REQUIRED)
include(GoogleTest)


set(TEST_NAME "test_typename")
add_executable(${TEST_NAME} ${TEST_NAME}.cpp)
target_link_libraries(${TEST_NAME} 
                        GTest::gtest 
                        GTest::gtest_main
                        )
add_test(${TEST_NAME} ${TEST_NAME})

set(TEST_NAME "test_bumpalo")
add_executable(${TEST_NAME} ${TEST_NAME}.cpp)
target_compile_definitions(${TEST_NAME} PRIVATE DEBUG_BUMPALO DEBUG_BUMPALOBASE)
target_link_libraries(${TEST_NAME} 
                        GTest::gtest 
                        GTest::gtest_main
                        )
add_test(${TEST_NAME} ${TEST_NAME})

set(TEST_NAME "test_bumpalo_with_map")
add_executable(${TEST_NAME} ${TEST_NAME}.cpp)
target_compile_definitions(${TEST_NAME} PRIVATE DEBUG_BUMPALO DEBUG_BUMPALOBASE)
target_link_libraries(${TEST_NAME} 
                        GTest::gtest 
                        GTest::gtest_main
                        )
add_test(${TEST_NAME} ${TEST_NAME})


set(TEST_NAME "test_bumpalobase")
add_executable(${TEST_NAME} ${TEST_NAME}.cpp)
target_compile_definitions(${TEST_NAME} PRIVATE DEBUG_BUMPALOBASE)
target_link_libraries(${TEST_NAME} 
                        GTest::gtest 
                        GTest::gtest_main
                        )
add_test(${TEST_NAME} ${TEST_NAME})

set(TEST_NAME "test_bumpalobase_exhausted")
add_executable(${TEST_NAME} ${TEST_NAME}.cpp)
target_compile_definitions(${TEST_NAME} PRIVATE DEBUG_BUMPALOBASE)
target_link_libraries(${TEST_NAME} 
                        GTest::gtest 
                        GTest::gtest_main
                        )
add_test(${TEST_NAME} ${TEST_NAME})

set(TEST_NAME "test_bumpalobase_pool_not_created_yet")
add_executable(${TEST_NAME} ${TEST_NAME}.cpp)
target_compile_definitions(${TEST_NAME} PRIVATE DEBUG_BUMPALOBASE)
target_link_libraries(${TEST_NAME} 
                        GTest::gtest 
                        GTest::gtest_main
                        )
add_test(${TEST_NAME} ${TEST_NAME})


set(TEST_NAME "test_safalo_permit")
add_executable(${TEST_NAME} ${TEST_NAME}.cpp safalo.cpp)
target_compile_definitions(${TEST_NAME} PRIVATE DEBUG_SAFALO)
add_test(${TEST_NAME} ${TEST_NAME})


set(TEST_NAME "test_safalo_prohibit")
add_executable(${TEST_NAME} ${TEST_NAME}.cpp safalo.cpp)
target_compile_definitions(${TEST_NAME} PRIVATE DEBUG_SAFALO)
add_test(${TEST_NAME} ${TEST_NAME})
set_property(TEST  ${TEST_NAME} PROPERTY PASS_REGULAR_EXPRESSION "Allocations Prohibited")
