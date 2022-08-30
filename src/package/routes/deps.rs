use crate::package::actions;
use actix_web::error::ErrorInternalServerError;
use actix_web::{get, web, HttpResponse, Result};
use poac_api_utils::{concat_org_name, DbPool, Response};

async fn deps_impl(pool: web::Data<DbPool>, name: String, version: String) -> Result<HttpResponse> {
    let name_ = name.clone();
    let version_ = version.clone();

    let packages = web::block(move || {
        let mut conn = pool.get()?;
        actions::deps(&mut conn, &name_, &version_)
    })
    .await?
    .map_err(ErrorInternalServerError)?;

    Ok(Response::maybe_ok(
        packages,
        format!(
            "No package found where name = `{}` & version = `{}`",
            name, version
        ),
    ))
}

#[get("/v1/packages/{org}/{name}/{version}/deps")]
async fn deps(
    pool: web::Data<DbPool>,
    param: web::Path<(String, String, String)>,
) -> Result<HttpResponse> {
    let (org, name, version) = param.into_inner();
    deps_impl(pool, concat_org_name(org, name), version).await
}

#[get("/v1/packages/{name}/{version}/deps")]
async fn deps_official(
    pool: web::Data<DbPool>,
    param: web::Path<(String, String)>,
) -> Result<HttpResponse> {
    let (name, version) = param.into_inner();
    deps_impl(pool, name, version).await
}

pub(crate) fn init_routes(cfg: &mut web::ServiceConfig) {
    cfg.service(deps);
    cfg.service(deps_official);
}
