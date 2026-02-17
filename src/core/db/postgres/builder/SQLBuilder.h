#pragma once
#include <string>
#include <vector>
#include <optional>
#include <boost/algorithm/string/join.hpp>

#include "core/file_system/FileSystemService.h"

class SQLBuilder {
public:
    explicit SQLBuilder(std::string tableName)
        : tableName_(std::move(tableName)) {}

    void select(const std::vector<std::string>& fields = {}) {
        if (fields.empty()) {
            this->sql_ += "SELECT * FROM " + this->tableName_;
            return;
        };

        this->sql_ += "SELECT " + boost::algorithm::join(fields, ",") + " FROM " + this->tableName_;
    }

    void exists() {
        this->sql_ = "SELECT EXISTS(" + this->sql_ + ")";
    }

    void insert(const std::vector<std::string>& fields = {}) {
        if (fields.empty()) {
            throw std::invalid_argument("Fields must be provided for INSERT");
        }

        std::vector<std::string> placeholders;
        placeholders.reserve(fields.size());

        for (std::size_t i = 0; i < fields.size(); ++i) {
            placeholders.push_back("$" + std::to_string(nextIndex()));
        }

        this->sql_ += "INSERT INTO " + this->tableName_ + "(" + boost::algorithm::join(fields, ",") + ") VALUES (" + boost::algorithm::join(placeholders, ",") + ")";
    }

    void update(const std::vector<std::string>& fields = {})
    {
        if (fields.empty())
        {
            throw std::invalid_argument("Fields must be provided for UPDATE");
        }

        std::vector<std::string> sets;
        sets.reserve(fields.size());

        for (const auto& f : fields)
            sets.push_back(f + " = $" + std::to_string(nextIndex()));

        this->sql_ += "UPDATE " + this->tableName_ + " SET " + boost::algorithm::join(sets, ",");
    }

    void returning(const std::string& field) {
        this->sql_ += " RETURNING " + field;
    }

    template <typename T>
    void where(const std::string& expr, const T& value) {
        if (this->hasWhere_) {
            this->sql_ += " AND " + expr + " = $" + std::to_string(nextIndex());
        } else {
            this->sql_ += " WHERE " + expr + " = $" + std::to_string(nextIndex());
            this->hasWhere_ = true;
        }
        this->params_.emplace_back(toString(value));
    }

    template <typename T>
    void whereAny(const std::string& expr, const std::string& cast, const std::vector<T>& values) {
        if (values.empty()) return;
        if (this->hasWhere_) {
            this->sql_ += " AND " + expr + " = ANY($" + std::to_string(nextIndex()) + "::" + cast + "[])";
        } else {
            this->sql_ += " WHERE " + expr + " = ANY($" + std::to_string(nextIndex()) + "::" + cast + "[])";
            this->hasWhere_ = true;
        }
        this->params_.emplace_back(arrayToPg(values));
    }

    void limit(std::optional<std::size_t> n) {
        if (!n) return;
        this->sql_ += " LIMIT $" + std::to_string(nextIndex()) + "::int";
        this->params_.emplace_back(std::to_string(*n));
    }

    void offset(std::optional<std::size_t> n) {
        if (!n) return;
        this->sql_ += " OFFSET $" + std::to_string(nextIndex()) + "::int";
        this->params_.emplace_back(std::to_string(*n));
    }

    void orderBy(const std::string& fieldName) {
        this->sql_ += " ORDER BY " + fieldName;
    }

    /// SQL
    [[nodiscard]] std::string str() const {
        return this->sql_;
    }

    [[nodiscard]] const std::vector<std::optional<std::string>>& params() const {
        return this->params_;
    }

private:
    std::string tableName_;
    std::string sql_;
    bool hasWhere_{false};
    std::vector<std::optional<std::string>> params_;
    int index_{0};

    int nextIndex() { return ++index_; }

    template <typename T>
    static std::string toString(const T& v) {
        if constexpr (std::is_same_v<T, std::string>) {
            return v;
        } else {
            return std::to_string(v);
        }
    }

    template <typename T>
    static std::string arrayToPg(const std::vector<T>& vec) {
        std::string out = "{";
        for (size_t i = 0; i < vec.size(); i++) {
            if (i > 0) out += ",";
            out += toString(vec[i]);
        }
        out += "}";
        return out;
    }
};
