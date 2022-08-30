mod get_all;
mod search;
mod versions;

use crate::package::actions;
use crate::utils::{DbPool, Response};
use actix_web::error::ErrorInternalServerError;
use actix_web::{post, web, HttpResponse, Result};
use serde::Deserialize;

#[derive(Deserialize, Clone)]
struct NameVerBody {
    name: String,
    version: String,
}

#[post("/v1/repoinfo")]
async fn repo_info(pool: web::Data<DbPool>, body: web::Json<NameVerBody>) -> Result<HttpResponse> {
    let body_ = body.clone();

    let packages = web::block(move || {
        let mut conn = pool.get()?;
        actions::repo_info(&mut conn, &body_.name, &body_.version)
    })
    .await?
    .map_err(ErrorInternalServerError)?;

    let body_ = body.into_inner();
    Ok(Response::maybe_ok(
        packages,
        format!(
            "No package found where name = `{}` & version = `{}`",
            body_.name, body_.version
        ),
    ))
}

#[post("/v1/deps")]
async fn deps(pool: web::Data<DbPool>, body: web::Json<NameVerBody>) -> Result<HttpResponse> {
    let body_ = body.clone();

    let packages = web::block(move || {
        let mut conn = pool.get()?;
        actions::deps(&mut conn, &body_.name, &body_.version)
    })
    .await?
    .map_err(ErrorInternalServerError)?;

    let body_ = body.into_inner();
    Ok(Response::maybe_ok(
        packages,
        format!(
            "No package found where name = `{}` & version = `{}`",
            body_.name, body_.version
        ),
    ))
}

pub(crate) fn init_routes(cfg: &mut web::ServiceConfig) {
    cfg.configure(get_all::init_routes);
    cfg.configure(search::init_routes);
    cfg.service(repo_info);
    cfg.configure(versions::init_routes);
    cfg.service(deps);
}
