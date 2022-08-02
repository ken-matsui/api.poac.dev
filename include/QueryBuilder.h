/**
 *
 *  @file QueryBuilder.h
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

#include <FilterBuilder.h>
#include <string>
#include <unordered_map>

namespace drogon::orm {
template <typename T>
class QueryBuilder : public FilterBuilder<T, true> {
  /**
   * @brief When a user does not set the table name explicitly, then retrieve
   * it from model `T`.
   *
   * @return std::string The table name
   */
  inline const std::string&
  getTableName() const {
    return this->from_.empty() ? T::tableName : this->from_;
  }

public:
  /**
   * @brief Set from which table to return.
   *
   * @param table The table.
   *
   * @return QueryBuilder& The QueryBuilder itself.
   */
  inline QueryBuilder&
  from(const std::string& table) {
    this->from_ = table;
    return *this;
  }

  /**
   * @brief Select specific columns.
   *
   * @param columns The columns.
   *
   * @return FilterBuilder<T, false> A new FilterBuilder.
   *
   * @note If you would return all rows, please use the `selectAll` method.
   * The method can return rows as model type `T`.
   */
  inline FilterBuilder<T, false>
  select(const std::string& columns) const {
    return {Method::Select, getTableName(), columns};
  }

  /**
   * @brief Select all columns.
   *
   * @return FilterBuilder<T, true> A new FilterBuilder.
   */
  inline FilterBuilder<T, true>
  selectAll() const {
    return {Method::Select, getTableName(), "*"};
  }

  /**
   * @brief Insert values.
   *
   * @param values The values to insert (key: column, value: value)
   * TODO(ken-matsui): @param returning Enable returning. For MySQL, nothing
   * returns.
   *
   * @return BaseBuilder<T, false> A new BaseBuilder.
   *
   * @note TODO(ken-matsui): Currently, insert does not support filters &
   * transforms such as insert into select syntax.
   */
  inline BaseBuilder<T, true>
  insert(const std::unordered_map<std::string, std::string>& values) const {
    if (values.empty()) {
      throw UsageError("The values should not be empty.");
    }

    for (const auto& value : values) {
      this->assert_column(value.first);
    }
    // TODO(ken-matsui): Switch returning. We should change the return type to
    // void.
    return FilterBuilder<T, true>{Method::Insert, getTableName(), values, true};
  }

  /**
   * @brief Update values.
   *
   * @param values The values to update (key: column, value: value)
   * TODO(ken-matsui): @param returning Enable returning. For MySQL, nothing
   * returns.
   *
   * @return FilterBuilder<T, true, true> A new FilterBuilder.
   */
  inline FilterBuilder<T, true, true>
  update(const std::unordered_map<std::string, std::string>& values) const {
    if (values.empty()) {
      throw UsageError("The values should not be empty.");
    }

    for (const auto& value : values) {
      this->assert_column(value.first);
    }
    // TODO(ken-matsui): Switch returning. We should change the return type to
    // void.
    return {Method::Update, getTableName(), values, true};
  }
};
} // namespace drogon::orm
