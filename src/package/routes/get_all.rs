use crate::package::actions;
use actix_web::error::ErrorInternalServerError;
use actix_web::{get, web, HttpResponse, Result};
use poac_api_utils::{DbPool, Response};
use serde::Deserialize;

#[derive(Deserialize)]
pub(crate) struct QueryParam {
    filter: Option<String>,
}

#[get("/packages")]
async fn get_all(
    pool: web::Data<DbPool>,
    web::Query(query): web::Query<QueryParam>,
) -> Result<HttpResponse> {
    let packages = web::block(move || {
        let mut conn = pool.get()?;
        actions::get_all(&mut conn, query.filter)
    })
    .await?
    .map_err(ErrorInternalServerError)?;

    Ok(Response::ok(packages))
}

pub(crate) fn init_routes(cfg: &mut web::ServiceConfig) {
    cfg.service(get_all);
}
