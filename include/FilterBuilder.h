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
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

namespace drogon::orm {

enum class FilterOperator {

};

template <typename T, bool Single = false>
class FilterBuilder {
public:
  std::string from_;
  std::string columns_;
  std::vector<std::string> filters;
  std::vector<std::string> transforms;

  inline FilterBuilder&
  eq(const std::string& column, const std::string& value) {
    filters.emplace_back(column + " = '" + value + "'");
    return *this;
  }

  inline FilterBuilder&
  neq(const std::string& column, const std::string& value) {
    filters.emplace_back(column + " != '" + value + "'");
    return *this;
  }

  inline FilterBuilder&
  gt(const std::string& column, const std::string& value) {
    filters.emplace_back(column + " > '" + value + "'");
    return *this;
  }

  inline FilterBuilder&
  gte(const std::string& column, const std::string& value) {
    filters.emplace_back(column + " >= '" + value + "'");
    return *this;
  }

  inline FilterBuilder&
  lt(const std::string& column, const std::string& value) {
    filters.emplace_back(column + " < '" + value + "'");
    return *this;
  }

  inline FilterBuilder&
  lte(const std::string& column, const std::string& value) {
    filters.emplace_back(column + " <= '" + value + "'");
    return *this;
  }

  inline FilterBuilder&
  like(const std::string& column, const std::string& pattern) {
    filters.emplace_back(column + " like '" + pattern + "'");
    return *this;
  }

  inline FilterBuilder&
  limit(std::uint64_t count) {
    transforms.emplace_back("limit " + std::to_string(count));
    return *this;
  }

  inline FilterBuilder&
  offset(std::uint64_t count) {
    transforms.emplace_back("offset " + std::to_string(count));
    return *this;
  }

  /**
   * @brief Limit result to an inclusive range.
   */
  inline FilterBuilder&
  range(std::uint64_t from, std::uint64_t to) {
    offset(from);
    limit(to - from + 1); // inclusive
    return *this;
  }

  /**
   * @brief Ensure returning only one row.
   */
  inline FilterBuilder<T, true>
  single() {
    return {from_, columns_, filters, transforms};
  }

  inline std::conditional_t<Single, T, std::vector<T>>
  execSync(const DbClientPtr& client) {
    std::string sql = "select " + columns_ + " from " + from_;
    if (!filters.empty()) {
      sql += " where " + filters[0];
      for (int i = 1; i < filters.size(); ++i) {
        sql += " and " + filters[i];
      }
    }
    if (!transforms.empty()) {
      for (const std::string& t : transforms) {
        sql += " " + t;
      }
    }

    Result r(nullptr);
    {
      auto binder = *client << std::move(sql);
      binder << Mode::Blocking;
      binder >> [&r](const Result& result) { r = result; };
      binder.exec(); // exec may be throw exception;
    }

    if constexpr (Single) {
      return T(r[0]);
    } else {
      std::vector<T> ret;
      for (const Row& row : r) {
        ret.template emplace_back(T(row));
      }
      return ret;
    }
  }
};

} // namespace drogon::orm
