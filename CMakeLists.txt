cmake_minimum_required(VERSION 3.16)
project(drogon_cpp_api)
set(CMAKE_CXX_STANDARD 20)

find_package(Drogon REQUIRED)

file(GLOB_RECURSE APP_SOURCES CONFIGURE_DEPENDS
    src/*.cpp
    src/*.cc
    src/*.c
)

add_executable(api ${APP_SOURCES})
target_link_libraries(api PRIVATE Drogon::Drogon)
target_include_directories(api PRIVATE src)
