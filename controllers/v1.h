#pragma once

// std
#include <string>

// external
#include <drogon/HttpController.h> // NOLINT(build/include_order)

using namespace drogon;

class v1 : public drogon::HttpController<v1> {
public:
  METHOD_LIST_BEGIN

  METHOD_ADD(v1::authCallback, "/auth/callback", Get);

  METHOD_ADD(v1::packages, "/packages", Get);

  METHOD_ADD(v1::dependents, "/packages/{name}/dependents", Get);
  METHOD_ADD(v1::dependents_org, "/packages/{org}/{name}/dependents", Get);

  METHOD_ADD(v1::deps, "/deps", Post, "JsonReq");
  METHOD_ADD(v1::repoinfo, "/repoinfo", Post, "JsonReq");
  METHOD_ADD(v1::search, "/search", Post, "JsonReq");
  METHOD_ADD(v1::versions, "/versions", Post, "JsonReq");

  METHOD_LIST_END

  static void
  authCallback(
      const HttpRequestPtr& req,
      std::function<void(const HttpResponsePtr&)>&& callback
  );

  static void
  packages(
      const HttpRequestPtr& req,
      std::function<void(const HttpResponsePtr&)>&& callback
  );

  static void
  dependents(
      const HttpRequestPtr& req,
      std::function<void(const HttpResponsePtr&)>&& callback,
      const std::string& name
  );
  static void
  dependents_org(
      const HttpRequestPtr& req,
      std::function<void(const HttpResponsePtr&)>&& callback,
      const std::string& org,
      const std::string& name
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
