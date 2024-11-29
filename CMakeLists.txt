cmake_minimum_required(VERSION 3.25)

project("bdb" LANGUAGES CXX)

add_subdirectory("src")
add_subdirectory("tools")

if(BUILD_TESTING)
    add_subdirectory("test")
endif()
