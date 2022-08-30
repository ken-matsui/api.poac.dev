use crate::package::Package;
use crate::utils::DbPool;
use actix_web::error::ErrorInternalServerError;
use actix_web::{get, post, web, HttpResponse, Result};
use serde::Deserialize;

#[derive(Deserialize)]
struct QueryParam {
    filter: Option<String>,
}

#[get("/v1/packages")]
async fn packages(
    pool: web::Data<DbPool>,
    web::Query(query): web::Query<QueryParam>,
) -> Result<HttpResponse> {
    let other_packages = web::block(move || {
        let mut conn = pool.get()?;
        Package::find_all(&mut conn, query.filter)
    })
    .await?
    .map_err(ErrorInternalServerError)?;

    Ok(HttpResponse::Ok().json(other_packages))
}

#[derive(Deserialize)]
struct Info {
    query: String,
}

#[post("/v1/search")]
async fn search(pool: web::Data<DbPool>, web::Json(body): web::Json<Info>) -> Result<HttpResponse> {
    let other_packages = web::block(move || {
        let mut conn = pool.get()?;
        Package::find(&mut conn, &body.query)
    })
    .await?
    .map_err(ErrorInternalServerError)?;

    Ok(HttpResponse::Ok().json(other_packages))
}

pub(crate) fn init_routes(cfg: &mut web::ServiceConfig) {
    cfg.service(packages);
    cfg.service(search);
}
