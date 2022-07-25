#pragma once

// std
#include <string>

// external
#include <drogon/HttpRequest.h> // NOLINT(build/include_order)
#include <json/json.h> // NOLINT(build/include_order)

namespace poac_api {

/// Retrieve query from request body without validation.
/// Works with JsonReq & HasStrQuery filters.
/// Without them, this function may throw exceptions.
inline std::string
getQuery(const drogon::HttpRequestPtr& req) {
  const auto request = req->getJsonObject();
  const Json::Value query = request->get("query", "");
  return query.asString();
}

} // namespace poac_api
