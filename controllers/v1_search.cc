#include "controllers/v1_search.h"

// std
#include <string>
#include <vector>

// internal
#include "models/Packages.h"

// external
#include <drogon/orm/Mapper.h> // NOLINT(build/include_order)

using drogon_model::postgres::Packages;

void
v1::search::asyncHandleHttpRequest(
    const drogon::HttpRequestPtr& req,
    std::function<void(const drogon::HttpResponsePtr&)>&& callback
) {
  const auto request = req->getJsonObject();
  if (request) {
    if (const Json::Value query = request->get("query", ""); query.isString()) {
      drogon::orm::Mapper<Packages> mp(drogon::app().getDbClient());
      const std::vector<Packages> packages = mp.findBy(drogon::orm::Criteria(
          Packages::Cols::_name, drogon::orm::CompareOperator::Like,
          "%" + query.asString() + "%"
      ));

      Json::Value packages_j(Json::arrayValue);
      for (const Packages& row : packages) {
        packages_j.append(row.toJson());
      }
      Json::Value res(Json::objectValue);
      res["data"] = packages_j;

      const drogon::HttpResponsePtr resp =
          drogon::HttpResponse::newHttpJsonResponse(res);
      resp->setStatusCode(drogon::k200OK);
      resp->setContentTypeCode(drogon::CT_APPLICATION_JSON);
      callback(resp);

      return;
    }
  }

  const drogon::HttpResponsePtr resp =
      drogon::HttpResponse::newHttpJsonResponse(Json::Value());
  resp->setStatusCode(drogon::k400BadRequest);
  resp->setContentTypeCode(drogon::CT_APPLICATION_JSON);
  callback(resp);
}
