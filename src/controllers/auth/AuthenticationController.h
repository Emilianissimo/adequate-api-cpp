#pragma once
#ifndef BEAST_API_AUTHENTICATIONCONTROLLER_H
#define BEAST_API_AUTHENTICATIONCONTROLLER_H

#include "core/request/Request.h"
#include "core/http/ResponseTypes.h"
#include "core/interfaces/HttpInterface.h"
#include "services/auth/AuthenticationService.h"

class AuthenticationController {
public:
    explicit AuthenticationController(const AuthenticationService& service) : service_(service) {}
    net::awaitable<Outcome> login(Request& request);
    net::awaitable<Outcome> registration(Request& request) const;
private:
    AuthenticationService service_;
};


#endif //BEAST_API_AUTHENTICATIONCONTROLLER_H