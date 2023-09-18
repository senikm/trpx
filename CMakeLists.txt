cmake_minimum_required(VERSION 3.5)

project(compression_app)

##########################################################
#  test
#


set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_BUILD_TYPE Release)
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")


##########################################################
#  Paths
#

set(TERSE_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include")


##########################################################
#  sources/binaries
#

add_subdirectory(src)




##########################################################
#  test
#

add_subdirectory(test)


