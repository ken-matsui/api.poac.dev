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
#include <BaseBuilder.h>
#include <future>
#include <memory>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

namespace drogon::orm {
template <typename T, bool SelectAll, bool Single = false>
class FilterBuilder : public BaseBuilder<T, SelectAll, Single> {
  std::string from_;
  std::string columns_;
  std::vector<std::string> filters_;
  optional<std::uint64_t> limit_;
  optional<std::uint64_t> offset_;
  // The order is important; use vector<pair> instead of unordered_map and
  // map.
  std::vector<std::pair<std::string, bool>> orders_;

  /**
   * @brief Generate SQL query in string.
   *
   * @return std::string The string generated SQL query.
   */
  inline std::string
  to_string() const override {
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
    if (!orders_.empty()) {
      sql += " order by " + orders_[0].first + " "
             + std::string(orders_[0].second ? "asc" : "desc");
      for (int i = 1; i < orders_.size(); ++i) {
        sql += ", " + orders_[i].first + " "
               + std::string(orders_[i].second ? "asc" : "desc");
      }
    }
    return sql;
  }

public:
  FilterBuilder(string_view from, string_view columns)
      : from_(from), columns_(columns) {}
  FilterBuilder(
      string_view from, string_view columns,
      const std::vector<std::string>& filters,
      const optional<std::uint64_t>& limit,
      const optional<std::uint64_t>& offset,
      const std::vector<std::pair<std::string, bool>>& orders
  )
      : from_(from), columns_(columns), filters_(filters), limit_(limit),
        offset_(offset), orders_(orders) {}

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
   * @brief Order the result.
   *
   * @param column The column to order by.
   * @param asc If `true`, ascending order. If `false`, descending order.
   * @return FilterBuilder& The FilterBuilder itself.
   */
  inline FilterBuilder&
  order(const std::string& column, bool asc = true) {
    orders_.emplace_back(column, asc);
    return *this;
  }

  /**
   * @brief Ensure returning only one row.
   *
   * @return FilterBuilder<T, SelectAll, true> The FilterBuilder where Single
   * is true and all else is the same.
   */
  inline FilterBuilder<T, SelectAll, true>
  single() const {
    return {from_, columns_, filters_, limit_, offset_, orders_};
  }
};

} // namespace drogon::orm
