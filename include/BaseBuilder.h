/**
 *
 *  @file BaseBuilder.h
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

#ifdef __cpp_impl_coroutine
#include <drogon/utils/coroutine.h>
#endif

namespace drogon::orm {
// Forward declaration to be a friend
template <typename T, bool SelectAll, bool Single = false>
class TransformBuilder;

template <typename T, bool SelectAll, bool Single = false>
class BaseBuilder {
  using ResultType = std::conditional_t<
      SelectAll, std::conditional_t<Single, T, std::vector<T>>,
      std::conditional_t<Single, Row, Result>>;

protected:
  // Make the constructor of `TransformBuilder<T, SelectAll, true>` through
  // `TransformBuilder::single()` be able to read these protected members.
  friend class TransformBuilder<T, SelectAll, true>;

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
  struct [[nodiscard]] BuilderAwaiter : public CallbackAwaiter<ResultType> {
    BuilderAwaiter(internal::SqlBinder&& binder) : binder_(std::move(binder)) {}

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

  inline internal::BuilderAwaiter
  execCoro(const DbClientPtr& client) {
    auto binder = *client << to_string();
    return {std::move(binder)};
  }
#endif
};

} // namespace drogon::orm
