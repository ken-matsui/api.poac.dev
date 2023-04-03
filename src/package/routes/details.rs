use crate::package::actions;
use actix_web::error::ErrorInternalServerError;
use actix_web::{get, web, HttpResponse, Result};
use poac_api_utils::{concat_org_name, DbPool, Response};

async fn details_impl(pool: web::Data<DbPool>, name: String) -> Result<HttpResponse> {
    let name_ = name.clone();

    let packages = web::block(move || {
        let mut conn = pool.get()?;
        actions::details(&mut conn, &name_)
    })
    .await?
    .map_err(ErrorInternalServerError)?;

    Ok(Response::ok(packages))
}

#[get("/packages/{org}/{name}/details")]
async fn details(
    pool: web::Data<DbPool>,
    param: web::Path<(String, String)>,
) -> Result<HttpResponse> {
    let (org, name) = param.into_inner();
    details_impl(pool, concat_org_name(org, name)).await
}

#[get("/packages/{name}/details")]
async fn details_official(
    pool: web::Data<DbPool>,
    name: web::Path<String>,
) -> Result<HttpResponse> {
    details_impl(pool, name.into_inner()).await
}

pub(crate) fn init_routes(cfg: &mut web::ServiceConfig) {
    cfg.service(details);
    cfg.service(details_official);
}
