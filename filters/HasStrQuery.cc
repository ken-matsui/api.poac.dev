#include "HasStrQuery.h"

// internal
#include "constants.hpp"

using namespace drogon;

void
HasStrQuery::doFilter(
    const HttpRequestPtr& req, FilterCallback&& fcb, FilterChainCallback&& fccb
) {
  const auto request = req->getJsonObject();
  if (const Json::Value query = request->get("query", ""); query.isString()) {
    // Passed
    fccb();
    return;
  }

  // Check failed
  fcb(poac_api::badRequest("The `query` key must be string"));
}
