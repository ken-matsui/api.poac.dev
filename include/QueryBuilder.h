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

namespace drogon::orm {

template <typename T>
class QueryBuilder {
  std::string from_;

public:
  inline QueryBuilder
  from(const std::string& table) {
    from_ = table;
    return *this;
  }

  inline FilterBuilder<T>
  select(const std::string& columns) {
    return {from_, columns};
  }
};

} // namespace drogon::orm
