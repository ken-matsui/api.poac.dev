#pragma once

// std
#include <string>

// external
#include <drogon/HttpResponse.h> // NOLINT(build/include_order)
#include <json/json.h> // NOLINT(build/include_order)

namespace poac_api {

inline Json::Value
error(const std::string& msg) {
  Json::Value res(Json::objectValue);
  res["error"] = msg;
  return res;
}

inline drogon::HttpResponsePtr
badRequest(const std::string& msg) {
  drogon::HttpResponsePtr resp =
      drogon::HttpResponse::newHttpJsonResponse(poac_api::error(msg));
  resp->setStatusCode(drogon::k400BadRequest);
  resp->setContentTypeCode(drogon::CT_APPLICATION_JSON);
  return resp;
}

inline drogon::HttpResponsePtr
internalError() {
  // For now, we avoid exposing internal error messages.
  drogon::HttpResponsePtr resp =
      drogon::HttpResponse::newHttpJsonResponse(Json::Value());
  resp->setStatusCode(drogon::k500InternalServerError);
  resp->setContentTypeCode(drogon::CT_APPLICATION_JSON);
  return resp;
}

inline drogon::HttpResponsePtr
ok(const Json::Value& value) {
  Json::Value res(Json::objectValue);
  res["data"] = value;

  drogon::HttpResponsePtr resp = drogon::HttpResponse::newHttpJsonResponse(res);
  resp->setStatusCode(drogon::k200OK);
  resp->setContentTypeCode(drogon::CT_APPLICATION_JSON);
  return resp;
}

template <typename Value>
inline Json::Value
toJson(const std::vector<Value>& container) {
  Json::Value values(Json::arrayValue);
  for (const Value& c : container) {
    values.append(c.toJson());
  }
  return values;
}

} // namespace poac_api
