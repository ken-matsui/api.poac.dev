/**
 *
 *  HasStrQuery.h
 *
 */

#pragma once

#include <drogon/HttpFilter.h>

using namespace drogon;

class HasStrQuery : public HttpFilter<HasStrQuery> {
public:
  HasStrQuery() {}

  void
  doFilter(
      const HttpRequestPtr& req, FilterCallback&& fcb,
      FilterChainCallback&& fccb
  ) override;
};
