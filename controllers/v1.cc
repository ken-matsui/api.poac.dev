#include "controllers/v1.h"

// std
#include <optional>
#include <string>
#include <vector>

// internal
#include <QueryBuilder.h>
#include <models/Package.h> // NOLINT(build/include_order)
#include <models/User.h> // NOLINT(build/include_order)
#include <utils.hpp>

// external
#include <dotenv.h> // NOLINT(build/include_order)
#include <drogon/HttpClient.h> // NOLINT(build/include_order)
#include <drogon/utils/Utilities.h> // NOLINT(build/include_order)

using drogon_model::postgres::Package;
using drogon_model::postgres::User;

std::optional<std::string>
getAccessToken(const std::string& code) {
  const auto client = HttpClient::newHttpClient("https://github.com");
  auto oauthReq = HttpRequest::newHttpRequest();
  oauthReq->setMethod(drogon::Get);
  oauthReq->setPath("/login/oauth/access_token");
  oauthReq->addHeader("Accept", "application/json");
  oauthReq->setParameter("code", code);
  oauthReq->setParameter(
      "client_id", dotenv::getenv("GITHUB_OAUTH_CLIENT_ID", "")
  );
  oauthReq->setParameter(
      "client_secret", dotenv::getenv("GITHUB_OAUTH_CLIENT_SECRET", "")
  );

  const auto [result, response] = client->sendRequest(oauthReq);
  if (result != ReqResult::Ok) {
    // Error
    LOG_ERROR << result;
    return std::nullopt;
  }
  const auto resJson = response->getJsonObject();
  if (!resJson) {
    // Error
    LOG_ERROR << "The response is invalid";
    return std::nullopt;
  }
  if (!(*resJson)["access_token"].isString()) {
    // Error
    LOG_ERROR << "The access_token is invalid";
    return std::nullopt;
  }

  // Retrieve access_token
  return (*resJson)["access_token"].asString();
}

struct UserMeta {
  std::string userName;
  std::string avatarUrl;
  std::string name;
  std::string email;
};

std::optional<UserMeta>
getUserMeta(const std::string& accessToken) {
  const auto client = HttpClient::newHttpClient("https://api.github.com");
  auto req = HttpRequest::newHttpRequest();
  req->setMethod(drogon::Get);
  req->setPath("/user");
  req->addHeader("Authorization", "token " + accessToken);

  const auto [result, response] = client->sendRequest(req);
  if (result != ReqResult::Ok) {
    // Error
    LOG_ERROR << result;
    return std::nullopt;
  }

  const auto resJson = response->getJsonObject();
  if (!resJson) {
    // Error
    LOG_ERROR << "The response is invalid";
    return std::nullopt;
  }

  UserMeta userMeta;
  if (!(*resJson)["login"].isString()) {
    // Error
    LOG_ERROR << "The login is invalid";
    return std::nullopt;
  }
  userMeta.userName = (*resJson)["login"].asString();

  if (!(*resJson)["avatar_url"].isString()) {
    // Error
    LOG_ERROR << "The avatar_url is invalid";
    return std::nullopt;
  }
  userMeta.avatarUrl = (*resJson)["avatar_url"].asString();

  if (!(*resJson)["name"].isString()) {
    // Error
    LOG_ERROR << "The name is invalid";
    return std::nullopt;
  }
  userMeta.name = (*resJson)["name"].asString();
  return userMeta;
}

std::optional<std::string>
getEmail(const std::string& accessToken) {
  const auto client = HttpClient::newHttpClient("https://api.github.com");
  auto req = HttpRequest::newHttpRequest();
  req->setMethod(drogon::Get);
  req->setPath("/user/emails");
  req->addHeader("Authorization", "token " + accessToken);

  const auto [result, response] = client->sendRequest(req);
  if (result != ReqResult::Ok) {
    // Error
    LOG_ERROR << result;
    return std::nullopt;
  }

  const auto resJson = response->getJsonObject();
  if (!resJson) {
    // Error
    LOG_ERROR << "The response is invalid";
    return std::nullopt;
  }

  for (const auto& j : (*resJson)) {
    if (j["primary"].isBool() && j["primary"].asBool()) {
      return j["email"].asString();
    }
  }
  LOG_ERROR << "What the heck user would have no primary email?: "
            << (*resJson).toStyledString();
  return std::nullopt;
}

std::optional<User>
createNewUser(const UserMeta& userMeta) {
  // Create a new user
  const std::vector<User> newUsers =
      drogon::orm::QueryBuilder<User>{}
          .from("public.user")
          .insert(
              {{"user_name", userMeta.userName},
               {"avatar_url", userMeta.avatarUrl},
               {"name", userMeta.name},
               {"email", userMeta.email},
               {"status", "active"}}
          )
          .execSync(drogon::app().getDbClient());
  if (newUsers.empty()) {
    LOG_ERROR << "The new user is empty. Something went wrong.";
    return std::nullopt;
  }
  return newUsers[0];
}

void
v1::authCallback(
    const HttpRequestPtr& req,
    std::function<void(const HttpResponsePtr&)>&& callback
) {
  std::string accessToken;
  if (const auto at = getAccessToken(req->getParameter("code"))) {
    accessToken = at.value();
  } else {
    callback(HttpResponse::newRedirectionResponse("https://poac.pm"));
    return;
  }

  UserMeta userMeta;
  if (const auto um = getUserMeta(accessToken)) {
    userMeta = um.value();
  } else {
    callback(HttpResponse::newRedirectionResponse("https://poac.pm"));
    return;
  }

  // Find the user
  const std::vector<User> users = drogon::orm::QueryBuilder<User>{}
                                      .from("public.user")
                                      .selectAll()
                                      .eq("user_name", userMeta.userName)
                                      .execSync(drogon::app().getDbClient());
  if (users.empty()) {
    // new user

    // get email
    if (const auto e = getEmail(accessToken)) {
      userMeta.email = e.value();
    } else {
      callback(HttpResponse::newRedirectionResponse("https://poac.pm"));
      return;
    }

    // Create a new user
    User user;
    if (const auto u = createNewUser(userMeta)) {
      user = u.value();
    } else {
      callback(HttpResponse::newRedirectionResponse("https://poac.pm"));
      return;
    }

    Json::Value newUserJson = user.toJson();
    newUserJson.removeMember("status");
    const std::string newUser = Json::FastWriter{}.write(newUserJson);
    const std::string userMetadata = drogon::utils::base64Encode(
        reinterpret_cast<const unsigned char*>(newUser.c_str()),
        newUser.size(),
        true
    );
    LOG_TRACE << userMetadata;
    callback(HttpResponse::newRedirectionResponse(
        "https://poac.pm/api/auth?access_token=" + accessToken
        + "&user_metadata=" + userMetadata
    ));
    return;
  } else {
    // not a new user
    const User& user = users[0];

    // disabled user
    if (user.getValueOfStatus() != "active") {
      LOG_ERROR << "A disabled user tried to log in: "
                << user.toJson().toStyledString();
      callback(HttpResponse::newRedirectionResponse("https://poac.pm"));
      return;
    }

    Json::Value userJson = user.toJson();
    userJson.removeMember("status");
    const std::string newUser = Json::FastWriter{}.write(userJson);
    const std::string userMetadata = drogon::utils::base64Encode(
        reinterpret_cast<const unsigned char*>(newUser.c_str()),
        newUser.size(),
        true
    );
    LOG_TRACE << userMetadata;
    callback(HttpResponse::newRedirectionResponse(
        "https://poac.pm/api/auth?access_token=" + accessToken
        + "&user_metadata=" + userMetadata
    ));
    return;

    // TODO(ken-matsui): update (name, avatar_url) if exists
  }
}

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
    sqlQuery.limit(perPage.asInt64());
  }
  const std::vector<Package> result =
      sqlQuery.execSync(drogon::app().getDbClient());
  callback(poac_api::ok(drogon::orm::toJson(result)));
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
