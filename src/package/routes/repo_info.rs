use crate::package::actions;
use crate::utils::{DbPool, Response};
use actix_web::error::ErrorInternalServerError;
use actix_web::{get, web, HttpResponse, Result};

async fn repo_info_impl(
    pool: web::Data<DbPool>,
    name: String,
    version: String,
) -> Result<HttpResponse> {
    let name_ = name.clone();
    let version_ = version.clone();

    let packages = web::block(move || {
        let mut conn = pool.get()?;
        actions::repo_info(&mut conn, &name_, &version_)
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

#[get("/v1/packages/{org}/{name}/{version}/repoinfo")]
async fn repo_info(
    pool: web::Data<DbPool>,
    param: web::Path<(String, String, String)>,
) -> Result<HttpResponse> {
    let (org, name, version) = param.into_inner();
    repo_info_impl(pool, format!("{}/{}", org, name), version).await
}

#[get("/v1/packages/{name}/{version}/repoinfo")]
async fn repo_info_official(
    pool: web::Data<DbPool>,
    param: web::Path<(String, String)>,
) -> Result<HttpResponse> {
    let (name, version) = param.into_inner();
    repo_info_impl(pool, name, version).await
}

pub(crate) fn init_routes(cfg: &mut web::ServiceConfig) {
    cfg.service(repo_info);
    cfg.service(repo_info_official);
}
