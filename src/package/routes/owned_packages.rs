use crate::package::actions;
use actix_web::error::ErrorInternalServerError;
use actix_web::{get, web, HttpResponse, Result};
use poac_api_utils::{DbPool, Response};

#[get("/users/{user_name}/packages")]
async fn owned_packages(
    pool: web::Data<DbPool>,
    user_name: web::Path<String>,
) -> Result<HttpResponse> {
    let user_name = user_name.into_inner();

    let packages = web::block(move || {
        let mut conn = pool.get()?;
        actions::owned_packages(&mut conn, &user_name)
    })
    .await?
    .map_err(ErrorInternalServerError)?;

    Ok(Response::ok(packages))
}

pub(crate) fn init_routes(cfg: &mut web::ServiceConfig) {
    cfg.service(owned_packages);
}
