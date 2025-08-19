#include <drogon/HttpController.h>
#include <json/json.h>
using namespace drogon;

class Health : public HttpController<Health> {
public:
	METHOD_LIST_BEGIN
		ADD_METHOD_TO(Health::list, "/health", Get, Head);
	METHOD_LIST_END

	drogon::Task<HttpResponsePtr> list(const HttpRequestPtr& req)
	{
		Json::Value body;
		body["status"] = "alive";

		bool db_ok = false;
		bool redis_ok = false;

		try {
			auto db = app().getDbClient("default");
			auto result = co_await db->execSqlCoro("SELECT 1");
			db_ok = (result.size() == 1);
			body["db"] = db_ok ? "up" : "down";
			if (!db_ok) body["errors"]["db"] = "unexpected result";
		} catch (const std::exception& e) {
			body["db"] = "down";
			body["errors"]["db"] = e.what();
		}

		try {
			auto redis = app().getRedisClient("default");
			auto redis_response = co_await redis->execCommandCoro("PING");
			redis_ok = (redis_response.type() == drogon::nosql::RedisResultType::kString &&
				redis_response.asString() == "PONG");
			body["redis"] = redis_ok ? "ok" : "fail";
			if (!redis_ok) body["errors"]["redis"] = "unexpected reply";
		} catch (const std::exception& e) {
			body["redis"] = "fail";
			body["errors"]["redis"] = e.what();
		}

		auto response = HttpResponse::newHttpJsonResponse(body);
		response->setStatusCode(k200OK);

		if (req->method() == Head) {
			response->setBody("");
		}
		co_return response;
	}
};
