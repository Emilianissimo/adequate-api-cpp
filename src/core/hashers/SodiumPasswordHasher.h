//
// Created by user on 15.02.2026.
//

#ifndef BEAST_API_SODIUMPASSWORDHASHER_H
#define BEAST_API_SODIUMPASSWORDHASHER_H
#include "core/hashers/interfaces/PasswordHasherInterface.h"
#include <sodium.h>
#include <stdexcept>
#include <memory>

namespace app::security {

// Profile to choose: interactive / moderate / sensitive
struct PwhashParams {
    std::size_t opslimit;
    std::size_t memlimit;
};

class SodiumPasswordHasher final : public PasswordHasherInterface {
public:
    explicit SodiumPasswordHasher(const PwhashParams params = defaultForBuild())
        : params_(params) {}

    std::string hash(const std::string& password) const override {
        const auto out = std::make_unique<char[]>(crypto_pwhash_STRBYTES);

        // crypto_pwhash_str returns string: $argon2id$... (salt+params)
        if (crypto_pwhash_str(
                out.get(),
                password.c_str(),
                password.size(),
                params_.opslimit,
                params_.memlimit) != 0)
        {
            throw std::runtime_error("pwhash: out of memory / failed");
        }
        return std::string(out.get());
    }

    bool verify(const std::string& password,
                const std::string& encodedHash,
                bool* needsRehash = nullptr) const override
    {
        // verify
        const int ok = crypto_pwhash_str_verify(
            encodedHash.c_str(),
            password.c_str(),
            password.size()
        );

        if (ok != 0) {
            if (needsRehash) *needsRehash = false;
            return false;
        }

        if (needsRehash) {
            *needsRehash = (crypto_pwhash_str_needs_rehash(
                encodedHash.c_str(),
                params_.opslimit,
                params_.memlimit
            ) != 0);
        }

        return true;
    }

    static PwhashParams defaultForBuild()
    {
#if defined(APP_PWHASH_TEST_PROFILE) && APP_PWHASH_TEST_PROFILE
        return { crypto_pwhash_OPSLIMIT_MIN, crypto_pwhash_MEMLIMIT_MIN };
#else
        return defaultInteractive();
#endif
    }

    static PwhashParams defaultInteractive() {
        return {
            crypto_pwhash_OPSLIMIT_INTERACTIVE,
            crypto_pwhash_MEMLIMIT_INTERACTIVE
        };
    }

    static PwhashParams defaultModerate() {
        return {
            crypto_pwhash_OPSLIMIT_MODERATE,
            crypto_pwhash_MEMLIMIT_MODERATE
        };
    }

    static PwhashParams defaultSensitive() {
        return {
            crypto_pwhash_OPSLIMIT_SENSITIVE,
            crypto_pwhash_MEMLIMIT_SENSITIVE
        };
    }

    PwhashParams params() const
    {
        return params_;
    }

private:
    PwhashParams params_;
};

} // namespace app::security

#endif //BEAST_API_SODIUMPASSWORDHASHER_H
