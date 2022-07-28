#include "controllers/v1.h"

// std
#include <string>
#include <vector>

// internal
#include "models/Package.h"

#include <QueryBuilder.h>
#include <utils.hpp>

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

  const drogon::orm::Row result =
      drogon::orm::QueryBuilder<Package>{}
          .from("package")
          .select("metadata->'dependencies' as dependencies")
          .eq("name", name.asString())
          .eq("version", version.asString())
          .limit(1)
          .single()
          .execSync(drogon::app().getDbClient());

  // Response build
  Json::Value package(Json::objectValue);
  package["dependencies"] = result["dependencies"].as<Json::Value>();
  callback(poac_api::ok(package));
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

  const drogon::orm::Row result = drogon::orm::QueryBuilder<Package>{}
                                      .from("package")
                                      .select("repository, sha256sum")
                                      .eq("name", name.asString())
                                      .eq("version", version.asString())
                                      .limit(1)
                                      .single()
                                      .execSync(drogon::app().getDbClient());

  // Response build
  Json::Value package(Json::objectValue);
  package["repository"] = result["repository"].as<std::string>();
  package["sha256sum"] = result["sha256sum"].as<std::string>();
  callback(poac_api::ok(package));
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
      drogon::orm::QueryBuilder<Package>{}.from("package").selectAll().like(
          "name", "%" + query.asString() + "%"
      );
  if (perPage.asUInt() != 0) {
    sqlQuery = sqlQuery.limit(perPage.asInt64());
  }
  const std::vector<Package> result =
      sqlQuery.execSync(drogon::app().getDbClient());
  callback(poac_api::ok(poac_api::toJson(result)));
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

  const drogon::orm::Result result = drogon::orm::QueryBuilder<Package>{}
                                         .from("package")
                                         .select("version")
                                         .eq("name", name.asString())
                                         .execSync(drogon::app().getDbClient());

  // Response build
  Json::Value versions(Json::arrayValue);
  for (const drogon::orm::Row& row : result) {
    versions.append(row["version"].as<std::string>());
  }
  callback(poac_api::ok(versions));
}
