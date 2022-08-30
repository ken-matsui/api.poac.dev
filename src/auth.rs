mod find_user;
mod get_access_token;
mod get_email;
mod get_user_meta;

use find_user::find_user;
use get_access_token::get_access_token;
use get_email::get_email;
use get_user_meta::get_user_meta;

use actix_web::error::ErrorInternalServerError;
use actix_web::{get, web, HttpResponse, Result};
use poac_api_utils::DbPool;
use serde::Deserialize;

#[derive(Deserialize)]
struct Query {
    code: String,
}

#[get("/auth/callback")]
async fn auth_callback(
    pool: web::Data<DbPool>,
    web::Query(query): web::Query<Query>,
) -> Result<HttpResponse> {
    let access_token = get_access_token(query.code).await?;
    let mut user_meta = get_user_meta(&access_token).await?;
    let user_meta_for_mv = user_meta.clone();
    let user = web::block(move || {
        let mut conn = pool.get()?;
        find_user(&mut conn, user_meta_for_mv)
    })
    .await?
    .map_err(ErrorInternalServerError)?;

    match user {
        None => {
            // This is a new user
            let email = get_email(&access_token).await?;
            user_meta.email = Some(email);
        }
        Some(user) => {
            // This is NOT a new user
        }
    }

    unimplemented!()
}

pub(crate) fn init_routes(cfg: &mut web::ServiceConfig) {
    cfg.service(auth_callback);
}
