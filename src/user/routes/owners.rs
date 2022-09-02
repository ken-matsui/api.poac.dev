use crate::user::actions;
use actix_web::error::ErrorInternalServerError;
use actix_web::{get, web, HttpResponse, Result};
use poac_api_utils::{concat_org_name, DbPool, Response};

async fn owners_impl(pool: web::Data<DbPool>, name: String) -> Result<HttpResponse> {
    let users = web::block(move || {
        let mut conn = pool.get()?;
        actions::owners(&mut conn, &name)
    })
    .await?
    .map_err(ErrorInternalServerError)?;

    Ok(Response::ok(users))
}

#[get("/packages/{org}/{name}/owners")]
async fn owners(
    pool: web::Data<DbPool>,
    param: web::Path<(String, String)>,
) -> Result<HttpResponse> {
    let (org, name) = param.into_inner();
    owners_impl(pool, concat_org_name(org, name)).await
}

#[get("/packages/{name}/owners")]
async fn owners_official(pool: web::Data<DbPool>, name: web::Path<String>) -> Result<HttpResponse> {
    owners_impl(pool, name.into_inner()).await
}

pub(crate) fn init_routes(cfg: &mut web::ServiceConfig) {
    cfg.service(owners);
    cfg.service(owners_official);
}
