#include "controllers/v1_versions.h"

// std
#include <string>

// internal
#include "models/Packages.h"

#include <constants.hpp>
#include <utils.hpp>

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
    callback(poac_api::ok(versions));
  } catch (const drogon::orm::DrogonDbException& e) {
    LOG_ERROR << e.base().what();
    callback(poac_api::internalError());
  }
}
