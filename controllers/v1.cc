struct UserMeta {
  std::string userName;
  std::string avatarUrl;
  std::string name;
  std::string email;
};

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
//  std::string accessToken;
//  if (const auto at = getAccessToken(req->getParameter("code"))) {
//    accessToken = at.value();
//  } else {
//    callback(HttpResponse::newRedirectionResponse("https://poac.pm"));
//    return;
//  }
//
//  UserMeta userMeta;
//  if (const auto um = getUserMeta(accessToken)) {
//    userMeta = um.value();
//  } else {
//    callback(HttpResponse::newRedirectionResponse("https://poac.pm"));
//    return;
//  }

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
