//
// Created by user on 15.02.2026.
//

#ifndef BEAST_API_PASSWORDHASHERINTERFACE_H
#define BEAST_API_PASSWORDHASHERINTERFACE_H

#pragma once
#include <string>

namespace app::security {

    struct HashResult {
        std::string encoded;
        bool needsRehash = false;
    };

    class PasswordHasherInterface {
    public:
        virtual ~PasswordHasherInterface() = default;

        virtual std::string hash(const std::string& password) const = 0;

        virtual bool verify(const std::string& password,
                            const std::string& encodedHash,
                            bool* needsRehash = nullptr) const = 0;
    };

} // namespace app::security

#endif //BEAST_API_PASSWORDHASHERINTERFACE_H
