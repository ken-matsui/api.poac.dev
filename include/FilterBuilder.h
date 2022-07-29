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

#include <TransformBuilder.h>
#include <drogon/utils/string_view.h>
#include <string>

namespace drogon::orm {
template <typename T, bool SelectAll>
class FilterBuilder : public TransformBuilder<T, SelectAll, false> {
public:
  /**
   * @brief A default constructor for derived classes.
   *
   * @return FilterBuilder The FilterBuilder itself.
   */
  FilterBuilder() = default;

  /**
   * @brief A copy constructor to be called by QueryBuilder.
   *
   * @return FilterBuilder The FilterBuilder itself.
   */
  FilterBuilder(drogon::string_view from, drogon::string_view column) {
    this->from_ = from;
    this->columns_ = column;
  }

  /**
   * @brief Filter rows whose value is the same as `value`.
   *
   * @param column The column to be filtered.
   * @param value The value to filter rows.
   *
   * @return FilterBuilder& The FilterBuilder itself.
   */
  inline FilterBuilder&
  eq(const std::string& column, const std::string& value) {
    this->filters_.emplace_back(column + " = '" + value + "'");
    return *this;
  }

  /**
   * @brief Filter rows whose value is NOT the same as `value`.
   *
   * @param column The column to be filtered.
   * @param value The value to filter rows.
   *
   * @return FilterBuilder& The FilterBuilder itself.
   */
  inline FilterBuilder&
  neq(const std::string& column, const std::string& value) {
    this->filters_.emplace_back(column + " != '" + value + "'");
    return *this;
  }

  /**
   * @brief Filter rows whose value is greater than `value`.
   *
   * @param column The column to be filtered.
   * @param value The value to filter rows.
   *
   * @return FilterBuilder& The FilterBuilder itself.
   */
  inline FilterBuilder&
  gt(const std::string& column, const std::string& value) {
    this->filters_.emplace_back(column + " > '" + value + "'");
    return *this;
  }

  /**
   * @brief Filter rows whose value is greater than or equal to `value`.
   *
   * @param column The column to be filtered.
   * @param value The value to filter rows.
   *
   * @return FilterBuilder& The FilterBuilder itself.
   */
  inline FilterBuilder&
  gte(const std::string& column, const std::string& value) {
    this->filters_.emplace_back(column + " >= '" + value + "'");
    return *this;
  }

  /**
   * @brief Filter rows whose value is less than `value`.
   *
   * @param column The column to be filtered.
   * @param value The value to filter rows.
   *
   * @return FilterBuilder& The FilterBuilder itself.
   */
  inline FilterBuilder&
  lt(const std::string& column, const std::string& value) {
    this->filters_.emplace_back(column + " < '" + value + "'");
    return *this;
  }

  /**
   * @brief Filter rows whose value is less than or equal to `value`.
   *
   * @param column The column to be filtered.
   * @param value The value to filter rows.
   *
   * @return FilterBuilder& The FilterBuilder itself.
   */
  inline FilterBuilder&
  lte(const std::string& column, const std::string& value) {
    this->filters_.emplace_back(column + " <= '" + value + "'");
    return *this;
  }

  /**
   * @brief Filter rows whose value matches the `pattern`.
   *
   * @param column The column to be filtered.
   * @param pattern The pattern to filter rows.
   *
   * @return FilterBuilder& The FilterBuilder itself.
   */
  inline FilterBuilder&
  like(const std::string& column, const std::string& pattern) {
    this->filters_.emplace_back(column + " like '" + pattern + "'");
    return *this;
  }
};

} // namespace drogon::orm
