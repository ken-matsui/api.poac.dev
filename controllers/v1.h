#pragma once

#include <drogon/HttpController.h>

using namespace drogon;

class v1 : public drogon::HttpController<v1> {
public:
  METHOD_LIST_BEGIN

  METHOD_ADD(v1::search, "/search", Post, "JsonReq", "HasStrQuery");
  METHOD_ADD(v1::versions, "/versions", Post, "JsonReq", "HasStrQuery");

  METHOD_LIST_END

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
