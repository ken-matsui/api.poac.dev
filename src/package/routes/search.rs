use crate::package::actions;
use actix_web::error::ErrorInternalServerError;
use actix_web::{post, web, HttpResponse, Result};
use poac_api_utils::{DbPool, Response};
use serde::Deserialize;

#[derive(Deserialize)]
struct SearchBody {
    query: String,
    per_page: Option<i64>,
}

#[post("/packages/search")]
async fn search(
    pool: web::Data<DbPool>,
    web::Json(body): web::Json<SearchBody>,
) -> Result<HttpResponse> {
    let packages = web::block(move || {
        let mut conn = pool.get()?;
        actions::search(&mut conn, &body.query, body.per_page)
    })
    .await?
    .map_err(ErrorInternalServerError)?;

    Ok(Response::ok(packages))
}

pub(crate) fn init_routes(cfg: &mut web::ServiceConfig) {
    cfg.service(search);
}
