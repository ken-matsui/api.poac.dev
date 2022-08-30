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
