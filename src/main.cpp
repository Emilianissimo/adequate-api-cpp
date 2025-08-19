#include <drogon/drogon.h>
#include <iostream>

int main(int argc, char* argv[])
{
	using namespace drogon;

	std::string configPath;
	for (int i = 1;i + 1 < argc; ++i) {
		if (std::string(argv[i]) == "-c") {
			configPath = argv[i + 1];
			break;
		}
	}

	if (!configPath.empty()) {
		app().loadConfigFile(configPath);
	} else {
		app().addListener("0.0.0.0", 8080);
		app().setLogLevel(trantor::Logger::kDebug);
	}

	app().run();
	return 0;
}
