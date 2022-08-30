use actix_web::error::{ErrorBadRequest, ErrorInternalServerError};
use actix_web::{get, web, HttpResponse, Result};
use awc::http::StatusCode;
use awc::Client;
use diesel::prelude::*;
use poac_api_utils::{log_query, DbError, DbPool};
use serde::{Deserialize, Serialize};
use std::env;

#[derive(Deserialize)]
struct Query {
    code: String,
}

#[get("/auth/callback")]
async fn auto_callback(
    pool: web::Data<DbPool>,
    web::Query(query): web::Query<Query>,
) -> Result<HttpResponse> {
    let access_token = get_access_token(query.code).await?;
    let user_meta = get_user_meta(&access_token).await?;
    let user = web::block(move || {
        let mut conn = pool.get()?;
        find_user(&mut conn, user_meta)
    })
    .await?
    .map_err(ErrorInternalServerError)?;

    match user {
        None => {
            // This is a new user
        }
        Some(user) => {
            // This is NOT a new user
        }
    }

    unimplemented!()
}

#[derive(Deserialize)]
struct AccessToken {
    access_token: String,
}

async fn get_access_token(code: String) -> Result<String> {
    let client = Client::new();
    let mut result = client
        .get(format!(
            "https://github.com/login/oauth/access_token?code={}&client_id={}&client_secret={}",
            code,
            env::var("GITHUB_OAUTH_CLIENT_ID").expect("GITHUB_OAUTH_CLIENT_ID must be set"),
            env::var("GITHUB_OAUTH_CLIENT_SECRET").expect("GITHUB_OAUTH_CLIENT_SECRET must be set")
        ))
        .insert_header(("Accept", "application/json"))
        .send()
        .await
        .map_err(ErrorInternalServerError)?;

    if result.status() != StatusCode::OK {
        return Err(ErrorBadRequest(format!("Code seems invalid: {}", code)));
    }

    let body = result.body().await.map_err(ErrorInternalServerError)?;
    let access_token =
        serde_json::from_slice::<AccessToken>(&*body).map_err(ErrorInternalServerError)?;

    Ok(access_token.access_token)
}

#[derive(Deserialize)]
struct UserMeta {
    name: String,
    #[serde(rename(deserialize = "login"))]
    user_name: String,
    avatar_url: String,
    email: Option<String>,
}

async fn get_user_meta(access_token: &str) -> Result<UserMeta> {
    let client = Client::new();
    let mut result = client
        .get("https://api.github.com/user")
        .bearer_auth(format!("token {}", access_token))
        .send()
        .await
        .map_err(ErrorInternalServerError)?;

    if result.status() != StatusCode::OK {
        return Err(ErrorBadRequest(format!(
            "AccessToken seems invalid: {}",
            access_token
        )));
    }

    let body = result.body().await.map_err(ErrorInternalServerError)?;
    let user_meta = serde_json::from_slice::<UserMeta>(&*body).map_err(ErrorInternalServerError)?;

    Ok(user_meta)
}

#[derive(Queryable, Serialize, Deserialize)]
struct User {
    name: String,
    user_name: String,
    avatar_url: String,
    status: String,
}

fn find_user(conn: &mut PgConnection, user_meta: UserMeta) -> Result<Option<User>, DbError> {
    use crate::schema::users::dsl::*;

    let query = users
        .select((name, user_name, avatar_url, status))
        .filter(user_name.eq(user_meta.user_name));
    log_query(&query);

    let result = query.first::<User>(conn).optional()?;
    Ok(result)
}

pub(crate) fn init_routes(cfg: &mut web::ServiceConfig) {
    cfg.service(auto_callback);
}
