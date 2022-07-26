#include "controllers/v1.h"

// std
#include <string>
#include <vector>

// internal
#include "models/Packages.h"

#include <constants.hpp>

// external
#include <drogon/orm/Mapper.h> // NOLINT(build/include_order)

using drogon_model::postgres::Packages;

// TODO(ken-matsui): Support `perPage`
void
v1::search(
    const HttpRequestPtr& req,
    std::function<void(const HttpResponsePtr&)>&& callback
) {
  const auto request = req->getJsonObject();
  if (const Json::Value query = request->get("query", ""); query.isString()) {
    drogon::orm::Mapper<Packages> mp(drogon::app().getDbClient());
    const std::vector<Packages> result = mp.findBy(drogon::orm::Criteria(
        Packages::Cols::_name, drogon::orm::CompareOperator::Like,
        "%" + query.asString() + "%"
    ));

    Json::Value packages(Json::arrayValue);
    for (const Packages& row : result) {
      packages.append(row.toJson());
    }
    callback(poac_api::ok(packages));
  } else {
    callback(poac_api::badRequest("`query` must be string"));
  }
}

void
v1::versions(
    const HttpRequestPtr& req,
    std::function<void(const HttpResponsePtr&)>&& callback
) {
  const auto request = req->getJsonObject();
  if (const Json::Value name = request->get("name", ""); name.isString()) {
    const drogon::orm::DbClientPtr clientPtr = drogon::app().getDbClient();

    try {
      const auto result = clientPtr->execSqlSync(
          "select version from packages where name = $1", name.asString()
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
  } else {
    callback(poac_api::badRequest("`name` must be string"));
  }
}

void
v1::repoinfo(
    const HttpRequestPtr& req,
    std::function<void(const HttpResponsePtr&)>&& callback
) {
  const auto request = req->getJsonObject();
  const Json::Value name = request->get("name", "");
  const Json::Value version = request->get("version", "");

  if (name.isString() && version.isString()) {
    const drogon::orm::DbClientPtr clientPtr = drogon::app().getDbClient();

    try {
      const auto result = clientPtr->execSqlSync(
          "select repository, sha256sum from packages where name = $1 and version = $2 limit 1",
          name.asString(), version.asString()
      );

      Json::Value packages(Json::arrayValue);
      for (const drogon::orm::Row& row : result) {
        Json::Value package(Json::objectValue);
        package["repository"] = row["repository"].as<std::string>();
        package["sha256sum"] = row["sha256sum"].as<std::string>();
        packages.append(package);
      }
      callback(poac_api::ok(packages));
    } catch (const drogon::orm::DrogonDbException& e) {
      LOG_ERROR << e.base().what();
      callback(poac_api::internalError());
    }
  } else {
    callback(poac_api::badRequest("`name` & `version` must be string"));
  }
}
