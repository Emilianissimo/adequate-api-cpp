#pragma once

#include <string>
#include <string_view>
#include <optional>
#include <vector>

struct PgValue {
    std::string data;
    bool is_null{false};
};

struct PgRow {
    std::vector<PgValue> columns;
};

struct PgResult {
    std::vector<std::string> columns_names;
    std::vector<PgRow> rows;
};
