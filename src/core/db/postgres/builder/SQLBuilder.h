#pragma once
#include <string>
#include <vector>
#include <optional>

class SQLBuilder {
public:
    explicit SQLBuilder(std::string base)
        : base_(std::move(base)) {}

    template <typename T>
    void where(const std::string& expr, T value) {
        if (hasWhere_) {
            sql_ += " AND " + expr + " $" + std::to_string(nextIndex());
        } else {
            sql_ += " WHERE " + expr + " $" + std::to_string(nextIndex());
            hasWhere_ = true;
        }
        params_.emplace_back(toString(std::move(value)));
    }

    template <typename T>
    void whereAny(const std::string& expr, const std::string& cast, const std::vector<T>& values) {
        if (values.empty()) return;
        if (hasWhere_) {
            sql_ += " AND " + expr + " = ANY($" + std::to_string(nextIndex()) + "::" + cast + "[])";
        } else {
            sql_ += " WHERE " + expr + " = ANY($" + std::to_string(nextIndex()) + "::" + cast + "[])";
            hasWhere_ = true;
        }
        params_.emplace_back(arrayToPg(values));
    }

    // LIMIT / OFFSET
    void limit(std::optional<std::size_t> n) {
        if (!n) return;
        sql_ += " LIMIT $" + std::to_string(nextIndex()) + "::int";
        params_.emplace_back(std::to_string(*n));
    }

    void offset(std::optional<std::size_t> n) {
        if (!n) return;
        sql_ += " OFFSET $" + std::to_string(nextIndex()) + "::int";
        params_.emplace_back(std::to_string(*n));
    }

    // SQL
    std::string str() const {
        return base_ + sql_;
    }

    const std::vector<std::optional<std::string>>& params() const {
        return params_;
    }

private:
    std::string base_;
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
