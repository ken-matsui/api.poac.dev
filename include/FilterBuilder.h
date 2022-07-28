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

  /**
   * @brief Generate SQL query in string.
   *
   * @return std::string The string generated SQL query.
   */
  inline std::string
  to_string() const {
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
    return sql;
  }

  /**
   * @brief Perform execSqlSync.
   *
   * @param client The smart pointer to the database client object.
   * @return Result The result from DB.
   */
  inline Result
  execSyncImpl(const DbClientPtr& client) {
    Result r(nullptr);
    {
      auto binder = *client << to_string();
      binder << Mode::Blocking;
      binder >> [&r](const Result& result) { r = result; };
      binder.exec(); // exec may throw exception
    }
    return r;
  }

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

  /**
   * @brief Filter rows whose value is the same as `value`.
   *
   * @param column The column to be filtered.
   * @param value The value to filter rows.
   * @return FilterBuilder& The FilterBuilder itself.
   */
  inline FilterBuilder&
  eq(const std::string& column, const std::string& value) {
    filters_.emplace_back(column + " = '" + value + "'");
    return *this;
  }

  /**
   * @brief Filter rows whose value is NOT the same as `value`.
   *
   * @param column The column to be filtered.
   * @param value The value to filter rows.
   * @return FilterBuilder& The FilterBuilder itself.
   */
  inline FilterBuilder&
  neq(const std::string& column, const std::string& value) {
    filters_.emplace_back(column + " != '" + value + "'");
    return *this;
  }

  /**
   * @brief Filter rows whose value is greater than `value`.
   *
   * @param column The column to be filtered.
   * @param value The value to filter rows.
   * @return FilterBuilder& The FilterBuilder itself.
   */
  inline FilterBuilder&
  gt(const std::string& column, const std::string& value) {
    filters_.emplace_back(column + " > '" + value + "'");
    return *this;
  }

  /**
   * @brief Filter rows whose value is greater than or equal to `value`.
   *
   * @param column The column to be filtered.
   * @param value The value to filter rows.
   * @return FilterBuilder& The FilterBuilder itself.
   */
  inline FilterBuilder&
  gte(const std::string& column, const std::string& value) {
    filters_.emplace_back(column + " >= '" + value + "'");
    return *this;
  }

  /**
   * @brief Filter rows whose value is less than `value`.
   *
   * @param column The column to be filtered.
   * @param value The value to filter rows.
   * @return FilterBuilder& The FilterBuilder itself.
   */
  inline FilterBuilder&
  lt(const std::string& column, const std::string& value) {
    filters_.emplace_back(column + " < '" + value + "'");
    return *this;
  }

  /**
   * @brief Filter rows whose value is less than or equal to `value`.
   *
   * @param column The column to be filtered.
   * @param value The value to filter rows.
   * @return FilterBuilder& The FilterBuilder itself.
   */
  inline FilterBuilder&
  lte(const std::string& column, const std::string& value) {
    filters_.emplace_back(column + " <= '" + value + "'");
    return *this;
  }

  /**
   * @brief Filter rows whose value matches the `pattern`.
   *
   * @param column The column to be filtered.
   * @param pattern The pattern to filter rows.
   * @return FilterBuilder& The FilterBuilder itself.
   */
  inline FilterBuilder&
  like(const std::string& column, const std::string& pattern) {
    filters_.emplace_back(column + " like '" + pattern + "'");
    return *this;
  }

  /**
   * @brief Limit the result to `count`.
   *
   * @param count The number of rows to be limited.
   * @return FilterBuilder& The FilterBuilder itself.
   */
  inline FilterBuilder&
  limit(std::uint64_t count) {
    limit_ = count;
    return *this;
  }

  /**
   * @brief Add a offset to the query.
   *
   * @param offset The offset.
   * @return FilterBuilder& The FilterBuilder itself.
   */
  inline FilterBuilder&
  offset(std::uint64_t count) {
    offset_ = count;
    return *this;
  }

  /**
   * @brief Limit the result to an inclusive range.
   *
   * @param from The first index to limit the result.
   * @param to The last index to limit the result.
   * @return FilterBuilder& The FilterBuilder itself.
   */
  inline FilterBuilder&
  range(std::uint64_t from, std::uint64_t to) {
    offset_ = from;
    limit_ = to - from + 1; // inclusive
    return *this;
  }

  /**
   * @brief Ensure returning only one row.
   *
   * @return FilterBuilder<T, SelectAll, true> The FilterBuilder where Single is
   * true and all else is the same.
   */
  inline FilterBuilder<T, SelectAll, true>
  single() const {
    return {from_, columns_, filters_, limit_, offset_};
  }

#if __cplusplus >= 201703L || (defined _MSC_VER && _MSC_VER > 1900)
  std::conditional_t<
      SelectAll, std::conditional_t<Single, T, std::vector<T>>,
      std::conditional_t<Single, Row, Result>>
  execSync(const DbClientPtr& client) {
    const Result r = execSyncImpl(client);

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
#else
  template <
      bool SA = SelectAll, bool SI = Single,
      std::enable_if_t<SA, std::nullptr_t> = nullptr,
      std::enable_if_t<SI, std::nullptr_t> = nullptr>
  inline T
  execSync(const DbClientPtr& client) {
    return T(execSyncImpl(client)[0]);
  }

  template <
      bool SA = SelectAll, bool SI = Single,
      std::enable_if_t<SA, std::nullptr_t> = nullptr,
      std::enable_if_t<!SI, std::nullptr_t> = nullptr>
  std::vector<T>
  execSync(const DbClientPtr& client) {
    std::vector<T> ret;
    for (const Row& row : execSyncImpl(client)) {
      ret.template emplace_back(T(row));
    }
    return ret;
  }

  template <
      bool SA = SelectAll, bool SI = Single,
      std::enable_if_t<!SA, std::nullptr_t> = nullptr,
      std::enable_if_t<SI, std::nullptr_t> = nullptr>
  inline Row
  execSync(const DbClientPtr& client) {
    return execSyncImpl(client)[0];
  }

  template <
      bool SA = SelectAll, bool SI = Single,
      std::enable_if_t<!SA, std::nullptr_t> = nullptr,
      std::enable_if_t<!SI, std::nullptr_t> = nullptr>
  inline Result
  execSync(const DbClientPtr& client) {
    return execSyncImpl(client);
  }
#endif
};

} // namespace drogon::orm
