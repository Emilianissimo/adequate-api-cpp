#ifndef BEAST_API_AUTHENTICATIONCONTROLLER_H
#define BEAST_API_AUTHENTICATIONCONTROLLER_H

#include "core/request/Request.h"
#include "core/http/ResponseTypes.h"
#include "core/http/interfaces/HttpInterface.h"
#include "core/openapi/OpenApiRegistrar.h"
#include "services/auth/AuthenticationService.h"
#include "services/users/UsersService.h"

class AuthenticationController {
public:
    static void registerOpenApi()
    {
        OpenApiRegistrar::registerSchema<LoginSerializer>();
        OpenApiRegistrar::registerSchema<RegisterSerializer>();
        OpenApiRegistrar::registerSchema<TokenResponseSerializer>();
    }

    explicit AuthenticationController(
        const AuthenticationService& service,
        const UsersService& usersService
        ) : service_(service)
        , usersService_(usersService)
    {}
    net::awaitable<Outcome> registration(const Request& request) const;
    net::awaitable<Outcome> login(Request& request) const;
private:
    AuthenticationService service_;
    UsersService usersService_;
};


#endif //BEAST_API_AUTHENTICATIONCONTROLLER_H
