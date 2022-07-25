#pragma once

#include <drogon/HttpSimpleController.h>

using namespace drogon;

namespace v1 {
class versions : public drogon::HttpSimpleController<versions> {
public:
  void
  asyncHandleHttpRequest(
      const HttpRequestPtr& req,
      std::function<void(const HttpResponsePtr&)>&& callback
  ) override;
  PATH_LIST_BEGIN

  PATH_ADD("/v1/versions", drogon::Post);

  PATH_LIST_END
};
} // namespace v1
