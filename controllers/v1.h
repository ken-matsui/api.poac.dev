#pragma once

#include <drogon/HttpController.h>

using namespace drogon;

class v1 : public drogon::HttpController<v1> {
public:
  METHOD_LIST_BEGIN

  // Users
  METHOD_ADD(v1::auth_callback, "/auth/callback", Get);
  //  METHOD_ADD(v1::users_login, "/users/login", Post, "JsonReq");

  METHOD_ADD(v1::deps, "/deps", Post, "JsonReq");
  METHOD_ADD(v1::repoinfo, "/repoinfo", Post, "JsonReq");
  METHOD_ADD(v1::search, "/search", Post, "JsonReq");
  METHOD_ADD(v1::versions, "/versions", Post, "JsonReq");

  METHOD_LIST_END

  static void
  auth_callback(
      const HttpRequestPtr& req,
      std::function<void(const HttpResponsePtr&)>&& callback
  );

  static void
  deps(
      const HttpRequestPtr& req,
      std::function<void(const HttpResponsePtr&)>&& callback
  );
  static void
  repoinfo(
      const HttpRequestPtr& req,
      std::function<void(const HttpResponsePtr&)>&& callback
  );
  static void
  search(
      const HttpRequestPtr& req,
      std::function<void(const HttpResponsePtr&)>&& callback
  );
  static void
  versions(
      const HttpRequestPtr& req,
      std::function<void(const HttpResponsePtr&)>&& callback
  );
};
