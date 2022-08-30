mod create_user;
mod find_user;
mod get_access_token;
mod get_email;
mod get_user_meta;

use create_user::create_user;
use find_user::find_user;
use get_access_token::get_access_token;
use get_email::get_email;
use get_user_meta::get_user_meta;

use actix_web::error::ErrorInternalServerError;
use actix_web::{get, web, HttpResponse, Result};
use poac_api_utils::{DbPool, Response};
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
    let user_meta = get_user_meta(&access_token).await?;

    let pool_cloned = pool.clone();
    let user_meta_cloned = user_meta.clone();
    let user = web::block(move || {
        let mut conn = pool_cloned.get()?;
        find_user(&mut conn, user_meta_cloned)
    })
    .await?
    .map_err(ErrorInternalServerError)?;

    match user {
        None => {
            // Create a new user so that there was not the same user.
            let email = get_email(&access_token).await?;
            let user = web::block(move || {
                let mut conn = pool.get()?;
                create_user(&mut conn, &user_meta, &email)
            })
            .await?
            .map_err(ErrorInternalServerError)?;
            Ok(Response::ok(user))
        }
        Some(user) => {
            // This is NOT a new user
            Err(ErrorInternalServerError(""))
        }
    }
}

pub(crate) fn init_routes(cfg: &mut web::ServiceConfig) {
    cfg.service(auth_callback);
}
