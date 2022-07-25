#include "controllers/v1_versions.h"

// std
#include <string>

// internal
#include "constants.hpp"
#include "models/Packages.h"
#include "utils.hpp"

using drogon_model::postgres::Packages;

void
v1::versions::asyncHandleHttpRequest(
    const HttpRequestPtr& req,
    std::function<void(const HttpResponsePtr&)>&& callback
) {
  const drogon::orm::DbClientPtr clientPtr = drogon::app().getDbClient();

  try {
    const auto result = clientPtr->execSqlSync(
        "select version from packages where name = $1", poac_api::getQuery(req)
    );
    Json::Value versions(Json::arrayValue);
    for (const drogon::orm::Row& row : result) {
      versions.append(row["version"].as<std::string>());
    }
    Json::Value res(Json::objectValue);
    res["data"] = versions;

    const drogon::HttpResponsePtr resp =
        drogon::HttpResponse::newHttpJsonResponse(res);
    resp->setStatusCode(drogon::k200OK);
    resp->setContentTypeCode(drogon::CT_APPLICATION_JSON);
    callback(resp);

    return;
  } catch (const drogon::orm::DrogonDbException& e) {
    LOG_ERROR << e.base().what();
    callback(poac_api::internalError());
    return;
  }
}
