#pragma once

#include <drogon/HttpSimpleController.h>

namespace v1 {
class search : public drogon::HttpSimpleController<search> {
public:
  void
  asyncHandleHttpRequest(
      const drogon::HttpRequestPtr& req,
      std::function<void(const drogon::HttpResponsePtr&)>&& callback
  ) override;
  PATH_LIST_BEGIN

  PATH_ADD("/v1/search", drogon::Post, "JsonReq", "HasStrQuery");

  PATH_LIST_END
};
} // namespace v1
