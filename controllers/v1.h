#pragma once

class v1 : public drogon::HttpController<v1> {
public:
  METHOD_LIST_BEGIN

  METHOD_ADD(v1::authCallback, "/auth/callback", Get);

  METHOD_ADD(v1::owners, "/packages/{name}/owners", Get);
  METHOD_ADD(v1::ownersOrg, "/packages/{org}/{name}/owners", Get);

  METHOD_ADD(v1::userPackages, "/users/{userName}/packages", Get);

  METHOD_LIST_END
};
