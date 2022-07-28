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
#include <future>
#include <memory>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

namespace drogon::orm {
template <typename T, bool SelectAll, bool Single = false>
class FilterBuilder {
  using ResultType = std::conditional_t<
      SelectAll, std::conditional_t<Single, T, std::vector<T>>,
      std::conditional_t<Single, Row, Result>>;

  std::string from_;
  std::string columns_;
  std::vector<std::string> filters_;
  optional<std::uint64_t> limit_;
  optional<std::uint64_t> offset_;
  // The order is important; use vector<pair> instead of unordered_map and
  // map.
  std::vector<std::pair<std::string, bool>> orders_;

public:
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

#ifdef __cpp_if_constexpr
  static ResultType
  convert_result(const Result& r) {
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
  static inline T
  convert_result(const Result& r) {
    return T(r[0]);
  }
  template <
      bool SA = SelectAll, bool SI = Single,
      std::enable_if_t<SA, std::nullptr_t> = nullptr,
      std::enable_if_t<!SI, std::nullptr_t> = nullptr>
  static inline std::vector<T>
  convert_result(const Result& r) {
    std::vector<T> ret;
    for (const Row& row : r) {
      ret.template emplace_back(T(row));
    }
    return ret;
  }
  template <
      bool SA = SelectAll, bool SI = Single,
      std::enable_if_t<!SA, std::nullptr_t> = nullptr,
      std::enable_if_t<SI, std::nullptr_t> = nullptr>
  static inline Row
  convert_result(const Result& r) {
    return r[0];
  }
  template <
      bool SA = SelectAll, bool SI = Single,
      std::enable_if_t<!SA, std::nullptr_t> = nullptr,
      std::enable_if_t<!SI, std::nullptr_t> = nullptr>
  static inline Result
  convert_result(const Result& r) {
    return r;
  }
#endif

  inline ResultType
  execSync(const DbClientPtr& client) {
    Result r(nullptr);
    {
      auto binder = *client << to_string();
      binder << Mode::Blocking;
      binder >> [&r](const Result& result) { r = result; };
      binder.exec(); // exec may throw exception
    }
    return convert_result(r);
  }

  std::future<ResultType>
  execAsync(const DbClientPtr& client) {
    auto binder = *client << to_string();
    std::shared_ptr<std::promise<ResultType>> prom =
        std::make_shared<std::promise<ResultType>>();
    binder >>
        [prom, this](const Result& r) { prom->set_value(convert_result(r)); };
    binder >> [prom](const std::exception_ptr& e) { prom->set_exception(e); };
    binder.exec();
    return prom->get_future();
  }

#ifdef __cpp_impl_coroutine
  namespace internal {
    struct [[nodiscard]] BuilderAwaiter : public CallbackAwaiter<ResultType> {
      BuilderAwaiter(internal::SqlBinder&& binder)
          : binder_(std::move(binder)) {}

      void
      await_suspend(std::coroutine_handle<> handle) {
        binder_ >> [handle, this](const drogon::orm::Result& result) {
          setValue(convert_result(result));
          handle.resume();
        };
        binder_ >> [handle, this](const std::exception_ptr& e) {
          setException(e);
          handle.resume();
        };
        binder_.exec();
      }

    private:
      internal::SqlBinder binder_;
    };
  } // namespace internal

  inline internal::BuilderAwaiter
  execCoro(const DbClientPtr& client) {
    auto binder = *client << to_string();
    return {std::move(binder)};
  }
#endif
};

} // namespace drogon::orm
