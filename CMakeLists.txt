cmake_minimum_required(VERSION 3.16)
project(beast_api CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(Boost REQUIRED COMPONENTS system thread)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(PostgreSQL REQUIRED)

file(GLOB_RECURSE APP_SOURCES CONFIGURE_DEPENDS
    src/*.cpp
)

add_executable(app ${APP_SOURCES})

target_link_libraries(app
    PRIVATE
    Boost::system
    Boost::thread
    nlohmann_json::nlohmann_json
    ${PostgreSQL_LIBRARIES}
    pthread
)

target_include_directories(app PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${PostgreSQL_INCLUDE_DIRS}
)
