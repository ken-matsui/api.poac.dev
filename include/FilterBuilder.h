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
#include <string>
#include <type_traits>
#include <unordered_map>

namespace drogon::orm {
template <typename T, bool SelectAll = false, bool UseUpdate = false>
class FilterBuilder : public std::conditional_t<
                          UseUpdate,
                          // For `UPDATE`, any transforms, such as limit, are prohibited.
                          BaseBuilder<T, SelectAll, false>,
                          TransformBuilder<T, SelectAll, false>> {
public:
  /**
   * @brief A default constructor for derived classes.
   *
   * @return FilterBuilder The FilterBuilder itself.
   */
  FilterBuilder() = default;

  /**
   * @brief A copy constructor to be called by QueryBuilder with `SELECT`.
   *
   * @param from The table.
   * @param columns The columns.
   *
   * @return FilterBuilder The FilterBuilder itself.
   */
  FilterBuilder(
      const Method method, const std::string& from, const std::string& columns
  ) {
    this->method_ = method;
    this->from_ = from;
    this->columns_ = columns;
  }

  /**
   * @brief A copy constructor to be called by QueryBuilder with `INSERT`.
   *
   * @param from The table.
   * @param columns The columns.
   *
   * @return FilterBuilder The FilterBuilder itself.
   */
  FilterBuilder(
      const Method method,
      const std::string& from,
      const std::unordered_map<std::string, std::string>& values,
      bool returning
  ) {
    this->method_ = method;
    this->from_ = from;
    this->values_ = values;
    this->returning_ = returning;
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
    this->assert_column(column);
    this->filters_.push_back({column, CompareOperator::EQ, value});
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
    this->assert_column(column);
    this->filters_.push_back({column, CompareOperator::NE, value});
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
    this->assert_column(column);
    this->filters_.push_back({column, CompareOperator::GT, value});
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
    this->assert_column(column);
    this->filters_.push_back({column, CompareOperator::GE, value});
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
    this->assert_column(column);
    this->filters_.push_back({column, CompareOperator::LT, value});
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
    this->assert_column(column);
    this->filters_.push_back({column, CompareOperator::LE, value});
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
    this->assert_column(column);
    this->filters_.push_back({column, CompareOperator::Like, pattern});
    return *this;
  }
};
} // namespace drogon::orm
