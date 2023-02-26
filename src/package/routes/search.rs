use crate::package::actions;
use actix_web::error::ErrorInternalServerError;
use actix_web::{post, web, HttpResponse, Result};
use poac_api_utils::{DbPool, Response};
use serde::Deserialize;

#[derive(Deserialize)]
pub(crate) struct Body {
    pub(crate) query: String,
    pub(crate) page: Option<i64>,
    pub(crate) per_page: Option<i64>,
    pub(crate) sort: Option<String>,   // newly_published, relevance
    pub(crate) filter: Option<String>, // unique, all
}

#[post("/packages/search")]
async fn search(pool: web::Data<DbPool>, web::Json(body): web::Json<Body>) -> Result<HttpResponse> {
    let packages = web::block(move || {
        let mut conn = pool.get()?;
        actions::search(&mut conn, &body)
    })
    .await?
    .map_err(ErrorInternalServerError)?;

    Ok(Response::ok(packages))
}

pub(crate) fn init_routes(cfg: &mut web::ServiceConfig) {
    cfg.service(search);
}
