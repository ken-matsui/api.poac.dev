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

} // namespace poac_api
