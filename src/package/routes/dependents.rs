use crate::package::actions;
use crate::utils::{concat_org_name, DbPool, Response};
use actix_web::error::ErrorInternalServerError;
use actix_web::{get, web, HttpResponse, Result};

async fn dependents_impl(pool: web::Data<DbPool>, name: String) -> Result<HttpResponse> {
    let name_ = name.clone();

    let packages = web::block(move || {
        let mut conn = pool.get()?;
        actions::dependents(&mut conn, &name_)
    })
    .await?
    .map_err(ErrorInternalServerError)?;

    Ok(Response::ok(packages))
}

#[get("/v1/packages/{org}/{name}/dependents")]
async fn dependents(
    pool: web::Data<DbPool>,
    param: web::Path<(String, String)>,
) -> Result<HttpResponse> {
    let (org, name) = param.into_inner();
    dependents_impl(pool, concat_org_name(org, name)).await
}

pub(crate) fn init_routes(cfg: &mut web::ServiceConfig) {
    cfg.service(dependents);
}
