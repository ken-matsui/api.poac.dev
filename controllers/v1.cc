#include "controllers/v1.h"

// std
#include <optional>
#include <string>
#include <utility>
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

std::optional<Json::Value>
findUser(const std::string& accessToken, UserMeta userMeta) {
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
      return std::nullopt;
    }

    // Create a new user
    User user;
    if (const auto u = createNewUser(userMeta)) {
      user = u.value();
    } else {
      return std::nullopt;
    }

    return user.toJson();
  } else {
    // not a new user
    const User& user = users[0];

    // disabled user
    if (user.getValueOfStatus() != "active") {
      LOG_ERROR << "A disabled user tried to log in: "
                << user.toJson().toStyledString();
      return std::nullopt;
    }

    // Stale user info (userMeta must be up-to-date)
    if (userMeta.name != user.getValueOfName()
        || userMeta.avatarUrl != user.getValueOfAvatarUrl()) {
      drogon::orm::QueryBuilder<User>{}
          .from("public.user")
          // Do not update email for performance.
          .update({{"name", userMeta.name}, {"avatar_url", userMeta.avatarUrl}})
          .eq("user_name", userMeta.userName)
          .execSync(drogon::app().getDbClient());
    }

    return user.toJson();
  }
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

  Json::Value user;
  if (const auto u = findUser(accessToken, userMeta)) {
    user = u.value();
  } else {
    callback(HttpResponse::newRedirectionResponse("https://poac.pm"));
    return;
  }
  user.removeMember("status");
  const std::string newUser = Json::FastWriter{}.write(user);
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
}

void
v1::owners(
    const HttpRequestPtr& req,
    std::function<void(const HttpResponsePtr&)>&& callback,
    const std::string& name
) {
  // Get dependents of given package name
  const drogon::orm::Result result =
      drogon::orm::QueryBuilder<User>{}
          .from("public.user u")
          .select("u.id as id, name, user_name, avatar_url")
          .custom("left join ownership o on o.package_name = $1", name)
          .custom("where o.user_id = u.id")
          .execSync(drogon::app().getDbClient());

  Json::Value owners(Json::arrayValue);
  for (const drogon::orm::Row& row : result) {
    Json::Value owner(Json::objectValue);
    owner["id"] = row["id"].as<std::string>();
    owner["name"] = row["name"].as<std::string>();
    owner["user_name"] = row["user_name"].as<std::string>();
    owner["avatar_url"] = row["avatar_url"].as<std::string>();
    owners.append(owner);
  }
  callback(poac_api::ok(owners));
}
void
v1::ownersOrg(
    const HttpRequestPtr& req,
    std::function<void(const HttpResponsePtr&)>&& callback,
    const std::string& org,
    const std::string& name
) {
  owners(req, std::move(callback), org + "/" + name);
}

void
v1::userPackages(
    const HttpRequestPtr& req,
    std::function<void(const HttpResponsePtr&)>&& callback,
    const std::string& userName
) {
  // Get owned packages
  const drogon::orm::Result result =
      drogon::orm::QueryBuilder<Package>{}
          .from("package p")
          .select("distinct on (name) p.*")
          .eq("u.user_name", userName, false)
          .custom("left join ownership o on o.package_name = p.name")
          .custom("left join public.user u on u.id = o.user_id")
          .order("p.name", true, false)
          .order("string_to_array(p.version, '.')::int[]", false, false)
          .execSync(drogon::app().getDbClient());

  std::vector<Package> packages;
  for (const drogon::orm::Row& r : result) {
    packages.emplace_back(r);
  }
  callback(poac_api::ok(drogon::orm::toJson(packages)));
}
