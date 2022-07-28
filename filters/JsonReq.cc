#include "JsonReq.h"

// internal
#include <utils.hpp>

void
JsonReq::doFilter(
    const HttpRequestPtr& req, FilterCallback&& fcb, FilterChainCallback&& fccb
) {
  const auto request = req->getJsonObject();
  if (request) {
    // Passed
    fccb();
    return;
  }

  // Check failed
  fcb(poac_api::badRequest("Request body must be JSON"));
}
