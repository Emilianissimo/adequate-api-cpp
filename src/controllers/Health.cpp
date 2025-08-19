#include <drogon/HttpController.h>
#include <json/json.h>
using namespace drogon;

class Health : public HttpController<Health> {
public:
	METHOD_LIST_BEGIN
		ADD_METHOD_TO(Health::list, "/health", Get, Head);
	METHOD_LIST_END

	void list(const HttpRequestPtr&, std::function<void(const HttpResponsePtr&)>&& cb)
	{
		Json::Value body;
		body["status"] = "alive";

		auto response = HttpResponse::newHttpJsonResponse(body);

		response->setStatusCode(k200OK);
		cb(response);
	}
};
