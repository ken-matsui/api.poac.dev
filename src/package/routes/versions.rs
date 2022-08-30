use crate::package::actions;
use crate::utils::{DbPool, Response};
use actix_web::error::ErrorInternalServerError;
use actix_web::{get, web, HttpResponse, Result};

async fn versions_impl(pool: web::Data<DbPool>, name: String) -> Result<HttpResponse> {
    let packages = web::block(move || {
        let mut conn = pool.get()?;
        actions::versions(&mut conn, &name)
    })
    .await?
    .map_err(ErrorInternalServerError)?;

    Ok(Response::ok(packages))
}

#[get("/v1/packages/{org}/{name}/versions")]
async fn versions(
    pool: web::Data<DbPool>,
    param: web::Path<(String, String)>,
) -> Result<HttpResponse> {
    let (org, name) = param.into_inner();
    versions_impl(pool, format!("{}/{}", org, name)).await
}

#[get("/v1/packages/{name}/versions")]
async fn versions_official(
    pool: web::Data<DbPool>,
    name: web::Path<String>,
) -> Result<HttpResponse> {
    versions_impl(pool, name.into_inner()).await
}

pub(crate) fn init_routes(cfg: &mut web::ServiceConfig) {
    cfg.service(versions);
    cfg.service(versions_official);
}
