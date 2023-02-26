use crate::package::actions;
use actix_web::error::ErrorInternalServerError;
use actix_web::{get, web, HttpResponse, Result};
use poac_api_utils::{concat_org_name, DbPool, Response};

async fn specific_impl(pool: web::Data<DbPool>, name: String) -> Result<HttpResponse> {
    let name_ = name.clone();

    let packages = web::block(move || {
        let mut conn = pool.get()?;
        actions::specific(&mut conn, &name_)
    })
    .await?
    .map_err(ErrorInternalServerError)?;

    Ok(Response::ok(packages))
}

#[get("/packages/{org}/{name}/specific")]
async fn specific(
    pool: web::Data<DbPool>,
    param: web::Path<(String, String)>,
) -> Result<HttpResponse> {
    let (org, name) = param.into_inner();
    specific_impl(pool, concat_org_name(org, name)).await
}

#[get("/packages/{name}/specific")]
async fn specific_official(
    pool: web::Data<DbPool>,
    name: web::Path<String>,
) -> Result<HttpResponse> {
    specific_impl(pool, name.into_inner()).await
}

pub(crate) fn init_routes(cfg: &mut web::ServiceConfig) {
    cfg.service(specific);
    cfg.service(specific_official);
}
