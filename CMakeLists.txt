cmake_minimum_required(VERSION 3.25)

set(CMAKE_EXPORT_COMPILE_COMMANDS 1)
set(CMAKE_TOOLCHAIN_FILE "/home/linuxdev/vcpkg/scripts/buildsystems/vcpkg.cmake") 
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON) 

project("bdb" LANGUAGES CXX)

find_package(PkgConfig REQUIRED)

pkg_check_modules(libedit REQUIRED IMPORTED_TARGET libedit)

include(CTest)

add_subdirectory("src")
add_subdirectory("tools")

if(BUILD_TESTING)
    find_package(Catch2 CONFIG REQUIRED)
    add_subdirectory("test")
endif()
