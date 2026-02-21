cmake_minimum_required(VERSION 3.16)
project(beast_api CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_compile_definitions(PROJECT_ROOT="${CMAKE_SOURCE_DIR}")

# ASam: be careful, could cause free() errors itself too (throws address list for example on working parts of offloading pattern)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fsanitize=address,undefined -fno-omit-frame-pointer")
set(CMAKE_LINKER_FLAGS_DEBUG "${CMAKE_LINKER_FLAGS_DEBUG} -fsanitize=address,undefined")

include(FetchContent)
FetchContent_Declare(
    jwt-cpp
    GIT_REPOSITORY https://github.com/Thalhammer/jwt-cpp.git
    GIT_TAG v0.7.1
)

if(WIN32)
    set(OPENSSL_ROOT_DIR "C:/msys64/ucrt64" CACHE PATH "OpenSSL root")
    set(JWT_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    set(JWT_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

    # Let find_package(CONFIG) and find modules discover MSYS2 prefix
    list(PREPEND CMAKE_PREFIX_PATH "C:/msys64/ucrt64")

    # PostgreSQL (libpq) explicit hints
    set(PostgreSQL_INCLUDE_DIR "C:/msys64/ucrt64/include" CACHE PATH "" FORCE)
    set(PostgreSQL_LIBRARY     "C:/msys64/ucrt64/lib/libpq.dll.a" CACHE FILEPATH "" FORCE)

    # Some FindPostgreSQL.cmake variants also check these:
    set(PostgreSQL_LIBRARY_DIR "C:/msys64/ucrt64/lib" CACHE PATH "" FORCE)
endif()

FetchContent_MakeAvailable(jwt-cpp)

find_package(nlohmann_json CONFIG REQUIRED)
find_package(PostgreSQL REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(Poco REQUIRED Net Foundation)

find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBMAGIC REQUIRED IMPORTED_TARGET libmagic)
pkg_check_modules(SODIUM REQUIRED IMPORTED_TARGET libsodium)

# For docker and linux CLION
if(UNIX AND NOT APPLE)
    find_package(Boost REQUIRED COMPONENTS system thread url)

    file(GLOB_RECURSE APP_SOURCES CONFIGURE_DEPENDS
            src/*.cpp
    )

    add_executable(app ${APP_SOURCES})

    target_link_libraries(app
        PRIVATE
        Boost::system
        Boost::thread
        Boost::url
        nlohmann_json::nlohmann_json
        jwt-cpp
        OpenSSL::SSL
        OpenSSL::Crypto
        ${PostgreSQL_LIBRARIES}
        pthread
        Poco::Net
        Poco::Foundation
        PkgConfig::LIBMAGIC
        ${SODIUM_LIBRARIES}
    )
    target_include_directories(app PRIVATE
        ${CMAKE_SOURCE_DIR}/src
        ${PostgreSQL_INCLUDE_DIRS}
        ${SODIUM_INCLUDE_DIRS}
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
        ${Boost_THREAD_LIBRARY}
        Boost::thread
        nlohmann_json::nlohmann_json
        jwt-cpp
        OpenSSL::SSL
        OpenSSL::Crypto
        ${PostgreSQL_LIBRARIES}
        pthread
        Poco::Net
        Poco::Foundation
        PkgConfig::LIBMAGIC
        ${SODIUM_LIBRARIES}
    )
    target_include_directories(app PRIVATE
        ${CMAKE_SOURCE_DIR}/src
        ${PostgreSQL_INCLUDE_DIRS}
        ${Boost_INCLUDE_DIRS}
        ${SODIUM_INCLUDE_DIRS}
    )
elseif(WIN32)
    find_package(Boost REQUIRED COMPONENTS system thread url)

    file(GLOB_RECURSE APP_SOURCES CONFIGURE_DEPENDS
        src/*.cpp
    )

    add_executable(app ${APP_SOURCES})

    target_link_libraries(app
        PRIVATE
        Boost::system
        Boost::thread
        Boost::url
        nlohmann_json::nlohmann_json
        jwt-cpp
        OpenSSL::SSL
        OpenSSL::Crypto
        ${PostgreSQL_LIBRARIES}
        # pthread
        Poco::Net
        Poco::Foundation
        PkgConfig::LIBMAGIC
        ${SODIUM_LIBRARIES}
    )
    target_include_directories(app PRIVATE
        ${CMAKE_SOURCE_DIR}/src
        ${PostgreSQL_INCLUDE_DIRS}
        ${Boost_INCLUDE_DIRS}
        ${SODIUM_INCLUDE_DIRS}
    )
endif()

target_compile_definitions(app PRIVATE HAVE_LIBMAGIC=1)

option(APP_PWHASH_TEST_PROFILE "Use low pwhash limits (test profile)" OFF)
if (APP_PWHASH_TEST_PROFILE)
    target_compile_definitions(app PRIVATE APP_PWHASH_TEST_PROFILE=1)
endif()

# Testing
option(ENABLE_TESTS "Enable unit and e2e tests" ON)
if (ENABLE_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
