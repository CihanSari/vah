cmake_minimum_required(VERSION 3.8)
project(VariantAccessHelper)
##
## TESTS
## create and configure the unit test target
##
enable_testing()
add_subdirectory(vah)
add_subdirectory(test)
