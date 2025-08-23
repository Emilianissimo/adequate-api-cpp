#pragma once

#include <string>
#include <vector>
#include <variant>
#include <boost/asio/awaitable.hpp>

struct Row {

};

using Rows = std::vector<Row>;

struct DbInterface {
    virtual ~DbInterface() = default;
    virtual boost::asio::awaitable<Rows> query(std::string sql) = 0;
};
