file(GLOB_RECURSE TEST_SOURCES CONFIGURE_DEPENDS
        *.test.cpp
)

add_executable(e2e_tests
        main.cpp ${TEST_SOURCES}
        api/Health.test.cpp
)

target_link_libraries(
        e2e_tests
        PRIVATE
        GTest::gtest
)

include(GoogleTest)
gtest_discover_tests(e2e_tests
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)
