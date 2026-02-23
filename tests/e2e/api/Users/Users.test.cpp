#include <gtest/gtest.h>

#include <boost/asio.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>

#include <filesystem>
#include <fstream>
#include <random>
#include <sstream>

#include <nlohmann/json.hpp>
#include "UsersClient.h"
#include "../../auth/AuthSession.h"

using nlohmann::json;

static void assertUserShape(const json& u)
{
    ASSERT_TRUE(u.contains("id"));
    ASSERT_TRUE(u.contains("username"));
    ASSERT_TRUE(u.contains("email"));
    ASSERT_TRUE(u.contains("created_at"));
    ASSERT_TRUE(u.contains("updated_at"));

    ASSERT_TRUE(u["id"].is_number_integer());
    ASSERT_TRUE(u["username"].is_string());
    ASSERT_TRUE(u["email"].is_string());
    ASSERT_TRUE(u["picture"].is_null() || u["picture"].is_string());
    ASSERT_TRUE(u["created_at"].is_string());
    ASSERT_TRUE(u["updated_at"].is_string());
}

static void assertUserShapeOnCreate(const json& u)
{
    ASSERT_TRUE(u.contains("id"));
    ASSERT_TRUE(u.contains("username"));
    ASSERT_TRUE(u.contains("email"));

    ASSERT_TRUE(u["id"].is_number_integer());
    ASSERT_TRUE(u["username"].is_string());
    ASSERT_TRUE(u["email"].is_string());
}

constexpr int N = 21;

TEST(UsersIndex, ReturnsCreatedUsers)
{
    auto [bearer] = test::http::AuthSession::obtain("test_nginx", "80");

    test::http::UsersClient api("test_nginx", "80", bearer);

    for (int i = 1; i <= N - 1; ++i) {
        json payload{
                {"username", "user_" + std::to_string(i)},
                {"email",    "user_" + std::to_string(i) + "@example.com"},
                {"password",    "user_" + std::to_string(i) + "@example.com"}
        };

        auto [status, body, rawBody] = api.store(payload);

        ASSERT_EQ(status, boost::beast::http::status::created) << rawBody;
        assertUserShapeOnCreate(body);
    }

    auto [status, body, rawBody] = api.index("limit=50&offset=0");
    ASSERT_EQ(status, boost::beast::http::status::ok) << rawBody;
    ASSERT_TRUE(body.is_array());
    ASSERT_EQ(static_cast<int>(body.size()), N);

    assertUserShape(body.at(0));
}

TEST(UsersIndex, AppliesLimit)
{
    auto [bearer] = test::http::AuthSession::obtain("test_nginx", "80");
    test::http::UsersClient api("test_nginx", "80", bearer);

    int limit = 10;

    auto [status, body, rawBody] = api.index("limit=" + std::to_string(limit) + "&offset=0");
    ASSERT_EQ(status, boost::beast::http::status::ok) << rawBody;
    ASSERT_TRUE(body.is_array());
    ASSERT_EQ(static_cast<int>(body.size()), limit);
}

/// NEGATIVE CASES

TEST(UsersStore, Returns422OnInvalidPayload)
{
    auto [bearer] = test::http::AuthSession::obtain("test_nginx", "80");
    test::http::UsersClient api("test_nginx", "80", bearer);

    json payload{
        {"username", "bad_user"}
    };

    auto [status, body, rawBody] = api.store(payload);

    ASSERT_EQ(status, boost::beast::http::status::unprocessable_entity) << rawBody;
    ASSERT_TRUE(body.contains("error"));
    ASSERT_TRUE(body["error"].is_string());
}

/// UNAUTHORIZED

TEST(UsersAuth, StoreRequiresAuthorization_401)
{
    // без bearer
    test::http::UsersClient api("test_nginx", "80");

    json payload{
        {"username", "unauth_user"},
        {"email", "unauth_user@example.com"}
    };

    auto res = api.store(payload);

    ASSERT_EQ(res.status, boost::beast::http::status::unauthorized) << res.rawBody;

    auto j = json::parse(res.rawBody);
    ASSERT_TRUE(j.contains("error"));
    ASSERT_TRUE(j["error"].is_string());
    ASSERT_EQ(j["error"].get<std::string>(), "Missing Authorization header");
}

TEST(UsersAuth, IndexRequiresAuthorization_401)
{
    test::http::UsersClient api("test_nginx", "80");

    auto res = api.index("limit=10&offset=0");

    ASSERT_EQ(res.status, boost::beast::http::status::unauthorized) << res.rawBody;

    auto j = json::parse(res.rawBody);
    ASSERT_TRUE(j.contains("error"));
    ASSERT_TRUE(j["error"].is_string());
}

/// PATCH AND MEDIA TESTING

static std::string readFileBin(const std::filesystem::path& p) {
    std::ifstream f(p, std::ios::binary);
    if (!f) throw std::runtime_error("Cannot open file: " + p.string());
    return std::string((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
}

static std::string randomBoundary() {
    static constexpr char chars[] =
        "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
    std::mt19937 rng{std::random_device{}()};
    std::uniform_int_distribution<int> dist(0, (int)sizeof(chars) - 2);

    std::string b = "----BeastApiTestBoundary";
    for (int i = 0; i < 24; ++i) b.push_back(chars[dist(rng)]);
    return b;
}

static std::string buildMultipartFile(
    const std::string& boundary,
    const std::string& fieldName,
    const std::string& filename,
    const std::string& contentType,
    const std::string& bytes
) {
    std::ostringstream o;
    o << "--" << boundary << "\r\n";
    o << "Content-Disposition: form-data; name=\"" << fieldName
      << "\"; filename=\"" << filename << "\"\r\n";
    o << "Content-Type: " << contentType << "\r\n\r\n";
    o.write(bytes.data(), (std::streamsize)bytes.size());
    o << "\r\n--" << boundary << "--\r\n";
    return o.str();
}


static const json* findUserById(const json& arr, int64_t id) {
    for (const auto& u : arr) {
        if (u.contains("id") && u["id"].is_number_integer() && u["id"].get<int64_t>() == id)
            return &u;
    }
    return nullptr;
}

static std::string extractPathFromUrl(const std::string& url) {
    if (url.rfind("http://", 0) == 0 || url.rfind("https://", 0) == 0) {
        auto pos = url.find('/', url.find("://") + 3);
        return (pos == std::string::npos) ? "/" : url.substr(pos);
    }
    return url;
}

TEST(UsersUpdate, PatchMultipartPng_ThenIndexFindById_ThenFetchSameFileBytes)
{
    auto [bearer] = test::http::AuthSession::obtain("test_nginx", "80");
    test::http::UsersClient api("test_nginx", "80", bearer);

    // 1) create
    json payload{
        {"username", "user_media_1"},
        {"email",    "user_media_1@example.com"},
        {"password", "user_media_1@example.com"}
    };

    auto [stCreated, createdBody, rawCreated] = api.store(payload);
    ASSERT_EQ(stCreated, boost::beast::http::status::created) << rawCreated;

    const int64_t id = createdBody["id"].get<int64_t>();

    // 2) PATCH multipart png
    const auto pngPath = std::filesystem::path(TEST_ASSETS_DIR) / "test.png";
    const std::string png = readFileBin(pngPath);

    const std::string boundary = randomBoundary();
    const std::string multipartBody = buildMultipartFile(boundary, "picture", "test.png", "image/png", png);

    auto patchRes = api.patchPictureMultipart(id, boundary, multipartBody);

    ASSERT_TRUE(
        patchRes.status == boost::beast::http::status::ok ||
        patchRes.status == boost::beast::http::status::no_content
    ) << patchRes.rawBody;

    // 3) find index
    auto [stIndex, arr, rawIndex] = api.index("limit=200&offset=0");
    ASSERT_EQ(stIndex, boost::beast::http::status::ok) << rawIndex;

    auto uPtr = findUserById(arr, id);
    ASSERT_NE(uPtr, nullptr) << rawIndex;

    const std::string pictureUrl = (*uPtr)["picture"].get<std::string>();
    const std::string path = extractPathFromUrl(pictureUrl);

    // 4) file bytes
    std::cout << path << std::endl;
    auto fileRes = api.getRaw(path, {{"accept", "image/png"}});
    ASSERT_EQ(fileRes.status, boost::beast::http::status::ok);
    ASSERT_EQ(fileRes.rawBody, png);

    // 5) DELETE user
    auto delRes = api.remove(id);

    ASSERT_TRUE(
        delRes.status == boost::beast::http::status::ok ||
        delRes.status == boost::beast::http::status::no_content
    ) << delRes.body;

    // 6) user absent in index
    auto [stIndex2, arr2, rawIndex2] = api.index("limit=200&offset=0");
    ASSERT_EQ(stIndex2, boost::beast::http::status::ok) << rawIndex2;
    ASSERT_EQ(findUserById(arr2, id), nullptr);

    // 7) file inaccessible
    auto fileRes2 = api.getRaw(path, {{"accept", "image/png"}});
    ASSERT_TRUE(
        fileRes2.status == boost::beast::http::status::not_found ||
        fileRes2.status == boost::beast::http::status::gone
    ) << fileRes2.rawBody;
}

//
// Created by user on 21.02.2026.
//
