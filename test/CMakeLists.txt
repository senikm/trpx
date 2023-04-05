add_subdirectory(googletest-main)



set(target "CompressTests")


add_executable(${target} "${CMAKE_CURRENT_SOURCE_DIR}/terse_tests.cpp" ../src/terse_functions.cpp)

target_link_libraries(${target} PRIVATE gtest gtest_main)
target_include_directories(${target} PUBLIC ${TERSE_INCLUDE_DIR})








