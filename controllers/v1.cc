#include "controllers/v1.h"

// std
#include <string>
#include <vector>

// internal
#include "models/Package.h"

#include <QueryBuilder.h>
#include <constants.hpp>

using drogon_model::postgres::Package;

void
v1::deps(
    const HttpRequestPtr& req,
    std::function<void(const HttpResponsePtr&)>&& callback
) {
  const auto request = req->getJsonObject();

  // Validations
  const Json::Value name = request->get("name", "");
  const Json::Value version = request->get("version", "");
  if (!(name.isString() && version.isString())) {
    callback(poac_api::badRequest("`name` & `version` must be string"));
    return;
  }

  // SQL query
  const drogon::orm::DbClientPtr clientPtr = drogon::app().getDbClient();
  try {
    const auto result = clientPtr->execSqlSync(
        "select metadata->'dependencies' as dependencies from package where name = $1 and version = $2 limit 1",
        name.asString(), version.asString()
    );

    // Response build
    Json::Value package(Json::objectValue);
    package["dependencies"] = result[0]["dependencies"].as<Json::Value>();
    callback(poac_api::ok(package));
  } catch (const drogon::orm::DrogonDbException& e) {
    LOG_ERROR << e.base().what();
    callback(poac_api::internalError());
  }
}

void
v1::repoinfo(
    const HttpRequestPtr& req,
    std::function<void(const HttpResponsePtr&)>&& callback
) {
  const auto request = req->getJsonObject();

  // Validations
  const Json::Value name = request->get("name", "");
  const Json::Value version = request->get("version", "");
  if (!(name.isString() && version.isString())) {
    callback(poac_api::badRequest("`name` & `version` must be string"));
    return;
  }

  // SQL query
  const drogon::orm::DbClientPtr clientPtr = drogon::app().getDbClient();
  try {
    const auto result = clientPtr->execSqlSync(
        "select repository, sha256sum from package where name = $1 and version = $2 limit 1",
        name.asString(), version.asString()
    );

    // Response build
    Json::Value package(Json::objectValue);
    package["repository"] = result[0]["repository"].as<std::string>();
    package["sha256sum"] = result[0]["sha256sum"].as<std::string>();
    callback(poac_api::ok(package));
  } catch (const drogon::orm::DrogonDbException& e) {
    LOG_ERROR << e.base().what();
    callback(poac_api::internalError());
  }
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

void
v1::search(
    const HttpRequestPtr& req,
    std::function<void(const HttpResponsePtr&)>&& callback
) {
  const auto request = req->getJsonObject();

  // Validations
  const Json::Value query = request->get("query", "");
  if (!query.isString()) {
    callback(poac_api::badRequest("`query` must be string"));
    return;
  }
  const Json::Value perPage = request->get("perPage", 0);
  if (!perPage.isUInt64()) {
    callback(poac_api::badRequest("`perPage` must be uint64"));
    return;
  }

  // SQL query
  auto sqlQuery =
      drogon::orm::QueryBuilder<Package>{}.from("package").select("*").like(
          "name", "%" + query.asString() + "%"
      );
  if (perPage.asUInt() != 0) {
    sqlQuery = sqlQuery.limit(perPage.asInt64());
  }
  const std::vector<Package> result =
      sqlQuery.execSync(drogon::app().getDbClient());
  callback(poac_api::ok(toJson(result)));
}

void
v1::versions(
    const HttpRequestPtr& req,
    std::function<void(const HttpResponsePtr&)>&& callback
) {
  const auto request = req->getJsonObject();

  // Validations
  const Json::Value name = request->get("name", "");
  if (!name.isString()) {
    callback(poac_api::badRequest("`name` must be string"));
    return;
  }

  // SQL query
  const drogon::orm::DbClientPtr clientPtr = drogon::app().getDbClient();
  try {
    const auto result = clientPtr->execSqlSync(
        "select version from package where name = $1", name.asString()
    );

    // Response build
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
