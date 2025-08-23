#pragma once
#include <string>
#include <optional>
#include <boost/asio/awaitable.hpp>

struct CacheInterface {
  virtual ~CacheInterface() = default;
  virtual boost::asio::awaitable<void> set(std::string key, std::string value, int ttl)=0;
  virtual boost::asio::awaitable<std::optional<std::string>> get(std::string key)=0;
};
