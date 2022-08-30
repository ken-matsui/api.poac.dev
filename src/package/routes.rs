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
async fn get_all(
    pool: web::Data<DbPool>,
    web::Query(query): web::Query<QueryParam>,
) -> Result<HttpResponse> {
    let packages = web::block(move || {
        let mut conn = pool.get()?;
        Package::get_all(&mut conn, query.filter)
    })
    .await?
    .map_err(ErrorInternalServerError)?;

    Ok(HttpResponse::Ok().json(packages))
}

#[derive(Deserialize)]
struct SearchBody {
    query: String,
    per_page: Option<i64>,
}

#[post("/v1/search")]
async fn search(
    pool: web::Data<DbPool>,
    web::Json(body): web::Json<SearchBody>,
) -> Result<HttpResponse> {
    let packages = web::block(move || {
        let mut conn = pool.get()?;
        Package::search(&mut conn, &body.query, body.per_page)
    })
    .await?
    .map_err(ErrorInternalServerError)?;

    Ok(HttpResponse::Ok().json(packages))
}

#[derive(Deserialize)]
struct RepoInfoBody {
    name: String,
    version: String,
}

#[post("/v1/repoinfo")]
async fn repo_info(
    pool: web::Data<DbPool>,
    web::Json(body): web::Json<RepoInfoBody>,
) -> Result<HttpResponse> {
    let packages = web::block(move || {
        let mut conn = pool.get()?;
        Package::repo_info(&mut conn, &body.name, &body.version)
    })
    .await?
    .map_err(ErrorInternalServerError)?;

    Ok(HttpResponse::Ok().json(packages))
}

pub(crate) fn init_routes(cfg: &mut web::ServiceConfig) {
    cfg.service(get_all);
    cfg.service(search);
    cfg.service(repo_info);
}