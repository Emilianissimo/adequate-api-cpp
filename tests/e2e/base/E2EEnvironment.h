//
// Created by user on 21.02.2026.
//

#ifndef BEAST_API_E2EENVIRONMENT_H
#define BEAST_API_E2EENVIRONMENT_H

#include <sys/wait.h>
#include <gtest/gtest.h>
#include <cstdlib>
#include <string>
#include <thread>
#include <chrono>

class E2EEnvironment final : public ::testing::Environment
{
public:
    void SetUp() override
    {
        waitForHealth("http://test_nginx/health");
    }

private:
    static std::string nullDevice()
    {
        return "/dev/null";
    }

    static void waitForHealth(const std::string& url)
    {
        // Using simple curl for sure
        const auto null = nullDevice();
        for (int i = 0; i < 60; ++i) {
            std::cout << "Waiting for health endpoint to starup: " << i << std::endl;
            const std::string cmd =
                "curl -fsS -o " + null + " " + url + " 2>" + null;

            auto res = std::system(cmd.c_str());
            if (res == 0)
            {
                std::cout << "Application is up, running tests..." << std::endl;
                return;
            }

            if (res == -1) { /* system() failed */ }

            int code = 999;
            if (WIFEXITED(res)) code = WEXITSTATUS(res);

            std::cout << "status=" << res << " exit=" << code << "\n";

            res = res / 256;
            if (res == 6)
            {
                std::cout << "Application is down. Could not resolve host." << std::endl;
            } else
            {
                std::cout << res << std::endl;
            }

            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
        FAIL() << "Health endpoint not ready: " << url;
    }
};

#endif //BEAST_API_E2EENVIRONMENT_H
