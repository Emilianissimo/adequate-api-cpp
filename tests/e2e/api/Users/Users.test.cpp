#include <gtest/gtest.h>
#include <nlohmann/json.hpp>
#include "UsersClient.h"

using nlohmann::json;

static void assertUserShape(const json& u)
{
    ASSERT_TRUE(u.contains("id"));
    ASSERT_TRUE(u.contains("username"));
    ASSERT_TRUE(u.contains("email"));
    ASSERT_TRUE(u.contains("picture"));
    ASSERT_TRUE(u.contains("created_at"));
    ASSERT_TRUE(u.contains("updated_at"));

    ASSERT_TRUE(u["id"].is_number_integer());
    ASSERT_TRUE(u["username"].is_string());
    ASSERT_TRUE(u["email"].is_string());
    ASSERT_TRUE(u["picture"].is_null() || u["picture"].is_string());
    ASSERT_TRUE(u["created_at"].is_string());
    ASSERT_TRUE(u["updated_at"].is_string());
}

TEST(UsersIndex, ReturnsCreatedUsers)
{
    test::http::UsersClient api("test_nginx", "80");

    constexpr int N = 20;

    for (int i = 1; i <= N; ++i) {
        json payload{
                {"username", "user_" + std::to_string(i)},
                {"email",    "user_" + std::to_string(i) + "@example.com"},
                {"password",    "user_" + std::to_string(i) + "@example.com"}
        };

        auto [status, body, rawBody] = api.store(payload);

        ASSERT_EQ(status, boost::beast::http::status::created) << rawBody;
        assertUserShape(body);
    }

    auto [status, body, rawBody] = api.index("limit=50&offset=0");
    ASSERT_EQ(status, boost::beast::http::status::ok) << rawBody;
    ASSERT_TRUE(body.is_array());
    ASSERT_EQ(static_cast<int>(body.size()), N);

    assertUserShape(body.at(0));
}

TEST(UsersIndex, AppliesLimit)
{
    test::http::UsersClient api("test_nginx", "80");

    auto [status, body, rawBody] = api.index("limit=10&offset=0");
    ASSERT_EQ(status, boost::beast::http::status::ok) << rawBody;
    ASSERT_TRUE(body.is_array());
    ASSERT_EQ(static_cast<int>(body.size()), 10);
}

/// NEGATIVE CASES

TEST(UsersStore, Returns422OnInvalidPayload)
{
    test::http::UsersClient api("test_nginx", "80");

    json payload{
        {"username", "bad_user"}
    };

    auto [status, body, rawBody] = api.store(payload);

    ASSERT_EQ(status, boost::beast::http::status::unprocessable_entity) << rawBody;
    ASSERT_TRUE(body.contains("error"));
    ASSERT_TRUE(body["error"].is_string());
}

//
// Created by user on 21.02.2026.
//