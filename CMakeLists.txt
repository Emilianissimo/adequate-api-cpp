cmake_minimum_required(VERSION 3.16)
project(beast_api CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(FetchContent)
FetchContent_Declare(
    libbcrypt
    GIT_REPOSITORY https://github.com/Emilianissimo/libbcrypt.git
    GIT_TAG        master
)
FetchContent_MakeAvailable(libbcrypt)

find_package(nlohmann_json CONFIG REQUIRED)
find_package(PostgreSQL REQUIRED)

# For docker and linux CLION
if(UNIX AND NOT APPLE)
    find_package(Boost REQUIRED COMPONENTS system thread url)

    file(GLOB_RECURSE APP_SOURCES CONFIGURE_DEPENDS
            src/*.cpp
    )

    add_executable(app ${APP_SOURCES})

    target_link_libraries(app
        PRIVATE
        bcrypt
        Boost::system
        Boost::thread
        Boost::url
        nlohmann_json::nlohmann_json
        ${PostgreSQL_LIBRARIES}
        pthread
    )
    target_include_directories(app PRIVATE
        ${CMAKE_SOURCE_DIR}/src
        ${PostgreSQL_INCLUDE_DIRS}
        ${libbcrypt_SOURCE_DIR}/include
    )
elseif(APPLE)
    set(Boost_USE_STATIC_LIBS OFF)
    set(Boost_USE_MULTITHREADED ON)
    set(Boost_USE_STATIC_RUNTIME OFF)

    set(Boost_NO_BOOST_CMAKE ON)
    find_package(Boost 1.84 REQUIRED COMPONENTS thread)
    find_path(BOOST_SYSTEM_HEADER_DIR boost/system/error_code.hpp HINTS /opt/homebrew/include)

    if(NOT Boost_THREAD_LIBRARY)
        find_library(Boost_THREAD_LIBRARY NAMES boost_thread PATHS /opt/homebrew/lib)
    endif()

    if(NOT Boost_FOUND OR NOT Boost_THREAD_LIBRARY)
        message(FATAL_ERROR "Boost thread library not found. Boost.System is header-only; no separate binary expected.")
    endif()

    file(GLOB_RECURSE APP_SOURCES CONFIGURE_DEPENDS
        src/*.cpp
    )

    add_executable(app ${APP_SOURCES})

    target_link_libraries(app
        PRIVATE
        bcrypt
        ${Boost_THREAD_LIBRARY}
        Boost::thread
        nlohmann_json::nlohmann_json
        ${PostgreSQL_LIBRARIES}
        pthread
    )
    target_include_directories(app PRIVATE
        ${CMAKE_SOURCE_DIR}/src
        ${PostgreSQL_INCLUDE_DIRS}
        ${libbcrypt_SOURCE_DIR}/include
        ${Boost_INCLUDE_DIRS}
    )
elseif(WIN32)
    find_package(Boost REQUIRED COMPONENTS system thread url)

    file(GLOB_RECURSE APP_SOURCES CONFIGURE_DEPENDS
        src/*.cpp
    )

    add_executable(app ${APP_SOURCES})

    target_link_libraries(app
        PRIVATE
        bcrypt
        Boost::system
        Boost::thread
        Boost::url
        nlohmann_json::nlohmann_json
        ${PostgreSQL_LIBRARIES}
        pthread
    )
    target_include_directories(app PRIVATE
        ${CMAKE_SOURCE_DIR}/src
        ${PostgreSQL_INCLUDE_DIRS}
        ${libbcrypt_SOURCE_DIR}/include
    )
endif()
