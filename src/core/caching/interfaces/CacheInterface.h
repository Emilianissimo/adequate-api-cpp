#pragma once
#include <string>
#include <optional>
#include <boost/asio/awaitable.hpp>

namespace net = boost::asio;

struct CachingInterface {
  virtual ~CachingInterface() = default;
  virtual net::awaitable<void> set(std::string key, std::string value, int ttl)=0;
  virtual net::awaitable<std::optional<std::string>> get(std::string key)=0;
};
