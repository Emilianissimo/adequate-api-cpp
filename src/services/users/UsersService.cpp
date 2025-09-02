#include "services/users/UsersServiceInterface.h"
#include "repositories/users/UsersRepository.h"
#include <algorithm>

class UsersService final : public UsersServiceInterface {
public:
    explicit UsersService(UsersRepository& repo) : repo_(repo) {}

    net::awaitable<std::vector<UserSerializer>> list(std::size_t limit, std::size_t offset) override {
        limit = std::clamp<std::size_t>(limit, 1, 1000);

        // place for transactions if you need them
        std::vector<UserEntity> users = co_await repo_.get_list(limit, offset);
        std::vector<UserSerializer> serializedUsers;
        serializedUsers.reserve(users.size());

        // We can retrieve various variants of data, modified too
        for (UserEntity& user : users) {
            UserSerializer serializedUser;
            serializedUser.id = user.id;
            serializedUser.username = user.username;
            serializedUser.email = user.email;
            serializedUser.picture = user.picture;
            serializedUsers.emplace_back(std::move(serializedUser));
        }
        co_return serializedUsers;
    }

private:
    UsersRepository& repo_;
};
