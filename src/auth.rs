mod create_user;
mod find_user;
mod get_access_token;
mod get_email;
mod get_user_meta;
mod update_user;

use create_user::create_user;
use find_user::find_user;
use get_access_token::get_access_token;
use get_email::get_email;
use get_user_meta::get_user_meta;
use update_user::update_user;

use actix_web::http::header;
use actix_web::{get, web, HttpResponse, Result};
use base64::{engine::general_purpose, Engine};
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
    let user_meta = get_user_meta(&access_token).await?;
    let maybe_user = find_user(pool.clone(), user_meta.clone()).await?;
    let user = match maybe_user {
        None => {
            // Create a new user so that there was not the same user.
            let email = get_email(&access_token).await?;
            create_user(pool.clone(), user_meta, email).await?
        }
        Some(user) => {
            // This is NOT a new user
            user.check_status()?;
            update_user(pool.clone(), user_meta, user).await?
        }
    };

    let base64_user = general_purpose::URL_SAFE.encode(serde_json::to_string(&user)?.as_bytes());
    let redirect_uri = format!(
        "https://poac.dev/api/auth?access_token={access_token}&user_metadata={base64_user}"
    );
    Ok(HttpResponse::TemporaryRedirect()
        .append_header((header::LOCATION, redirect_uri))
        .finish())
}

pub(crate) fn init_routes(cfg: &mut web::ServiceConfig) {
    cfg.service(auth_callback);
}
