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

namespace drogon::orm {
template <typename T, bool SelectAll, bool Single = false>
class BaseBuilder {
  using ResultType = std::conditional_t<
      SelectAll, std::conditional_t<Single, T, std::vector<T>>,
      std::conditional_t<Single, Row, Result>>;

protected:
  /**
   * @brief Generate SQL query in string.
   *
   * @return std::string The string generated SQL query.
   */
  virtual inline std::string
  to_string() const = 0;

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
