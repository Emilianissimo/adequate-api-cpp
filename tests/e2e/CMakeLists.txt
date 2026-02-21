file(GLOB_RECURSE TEST_SOURCES CONFIGURE_DEPENDS
        *.test.cpp
)

add_executable(e2e_tests
        main.cpp ${TEST_SOURCES}
)

if (APP_PWHASH_TEST_PROFILE)
    target_compile_definitions(e2e_tests PRIVATE APP_PWHASH_TEST_PROFILE=1)
endif()

target_link_libraries(
        e2e_tests
        PRIVATE
        GTest::gtest
)

include(GoogleTest)
gtest_discover_tests(e2e_tests
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)
