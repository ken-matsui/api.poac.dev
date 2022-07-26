#include "controllers/v1.h"

// std
#include <string>
#include <vector>

// internal
#include "models/Packages.h"

#include <constants.hpp>
#include <utils.hpp>

// external
#include <drogon/orm/Mapper.h> // NOLINT(build/include_order)

using drogon_model::postgres::Packages;

void
v1::search(
    const HttpRequestPtr& req,
    std::function<void(const HttpResponsePtr&)>&& callback
) {
  drogon::orm::Mapper<Packages> mp(drogon::app().getDbClient());
  const std::vector<Packages> packages = mp.findBy(drogon::orm::Criteria(
      Packages::Cols::_name, drogon::orm::CompareOperator::Like,
      "%" + poac_api::getQuery(req) + "%"
  ));

  Json::Value packages_j(Json::arrayValue);
  for (const Packages& row : packages) {
    packages_j.append(row.toJson());
  }
  callback(poac_api::ok(packages_j));
}

void
v1::versions(
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
