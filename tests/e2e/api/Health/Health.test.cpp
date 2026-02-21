#include <gtest/gtest.h>
#include "HealthCheckClient.h"

TEST(HealthEndpoint, Returns200)
{
    test::http::HealthCheckClient health("test_nginx", "80");

    const test::http::HealthResponse response = health.check();

    ASSERT_TRUE(response.isOk());
    ASSERT_EQ(response.status, "alive");
    ASSERT_EQ(response.statusCode, boost::beast::http::status::ok); // Duplicate, but for example

}
//
// Created by user on 21.02.2026.
//
