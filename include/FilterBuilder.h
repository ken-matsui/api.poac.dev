/**
 *
 *  @file FilterBuilder.h
 *  @author Ken Matsui
 *
 *  Copyright 2022, Ken Matsui.  All rights reserved.
 *  https://github.com/drogonframework/drogon
 *  Use of this source code is governed by a MIT license
 *  that can be found in the License file.
 *
 *  Drogon
 *
 */

#pragma once

#include <drogon/orm/DbClient.h>
#include <drogon/utils/optional.h>
#include <drogon/utils/string_view.h>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

namespace drogon::orm {

template <typename T, bool SelectAll, bool Single = false>
class FilterBuilder {
  std::string from_;
  std::string columns_;
  std::vector<std::string> filters_;
  optional<std::uint64_t> limit_;
  optional<std::uint64_t> offset_;

public:
  FilterBuilder(string_view from, string_view columns)
      : from_(from), columns_(columns) {}
  FilterBuilder(
      string_view from, string_view columns,
      const std::vector<std::string>& filters,
      const optional<std::uint64_t>& limit,
      const optional<std::uint64_t>& offset
  )
      : from_(from), columns_(columns), filters_(filters), limit_(limit),
        offset_(offset) {}

  inline FilterBuilder&
  eq(const std::string& column, const std::string& value) {
    filters_.emplace_back(column + " = '" + value + "'");
    return *this;
  }

  inline FilterBuilder&
  neq(const std::string& column, const std::string& value) {
    filters_.emplace_back(column + " != '" + value + "'");
    return *this;
  }

  inline FilterBuilder&
  gt(const std::string& column, const std::string& value) {
    filters_.emplace_back(column + " > '" + value + "'");
    return *this;
  }

  inline FilterBuilder&
  gte(const std::string& column, const std::string& value) {
    filters_.emplace_back(column + " >= '" + value + "'");
    return *this;
  }

  inline FilterBuilder&
  lt(const std::string& column, const std::string& value) {
    filters_.emplace_back(column + " < '" + value + "'");
    return *this;
  }

  inline FilterBuilder&
  lte(const std::string& column, const std::string& value) {
    filters_.emplace_back(column + " <= '" + value + "'");
    return *this;
  }

  inline FilterBuilder&
  like(const std::string& column, const std::string& pattern) {
    filters_.emplace_back(column + " like '" + pattern + "'");
    return *this;
  }

  inline FilterBuilder&
  limit(std::uint64_t count) {
    limit_ = count;
    return *this;
  }

  inline FilterBuilder&
  offset(std::uint64_t count) {
    offset_ = count;
    return *this;
  }

  /**
   * @brief Limit result to an inclusive range.
   */
  inline FilterBuilder&
  range(std::uint64_t from, std::uint64_t to) {
    offset_ = from;
    limit_ = to - from + 1; // inclusive
    return *this;
  }

  /**
   * @brief Ensure returning only one row.
   */
  inline FilterBuilder<T, SelectAll, true>
  single() {
    return {from_, columns_, filters_, limit_, offset_};
  }

  inline std::conditional_t<
      SelectAll, std::conditional_t<Single, T, std::vector<T>>,
      std::conditional_t<Single, Row, Result>>
  execSync(const DbClientPtr& client) {
    std::string sql = "select " + columns_ + " from " + from_;
    if (!filters_.empty()) {
      sql += " where " + filters_[0];
      for (int i = 1; i < filters_.size(); ++i) {
        sql += " and " + filters_[i];
      }
    }
    if (limit_.has_value()) {
      sql += " limit " + std::to_string(limit_.value());
    }
    if (offset_.has_value()) {
      sql += " offset " + std::to_string(offset_.value());
    }

    Result r(nullptr);
    {
      auto binder = *client << std::move(sql);
      binder << Mode::Blocking;
      binder >> [&r](const Result& result) { r = result; };
      binder.exec(); // exec may throw exception
    }

    if constexpr (SelectAll) {
      if constexpr (Single) {
        return T(r[0]);
      } else {
        std::vector<T> ret;
        for (const Row& row : r) {
          ret.template emplace_back(T(row));
        }
        return ret;
      }
    } else {
      if constexpr (Single) {
        return r[0];
      } else {
        return r;
      }
    }
  }
};

} // namespace drogon::orm
