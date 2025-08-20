cmake_minimum_required(VERSION 3.16)
project(beast_api CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Boost REQUIRED COMPONENTS system thread)

file(GLOB_RECURSE APP_SOURCES CONFIGURE_DEPENDS
    src/*.cpp
)

add_executable(app ${APP_SOURCES})

target_link_libraries(app
    Boost::system
    Boost::thread
    pthread
)