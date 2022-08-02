/**
 *
 *  JsonReq.h
 *
 */

#pragma once

#include <drogon/HttpFilter.h>

using namespace drogon;

class JsonReq : public HttpFilter<JsonReq> {
public:
  JsonReq() {}

  void
  doFilter(
      const HttpRequestPtr& req, FilterCallback&& fcb,
      FilterChainCallback&& fccb
  ) override;
};