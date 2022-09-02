use actix_web::error::{ErrorBadRequest, ErrorInternalServerError};
use actix_web::Result;
use awc::http::StatusCode;
use awc::Client;
use serde::Deserialize;
use std::env;

#[derive(Deserialize)]
struct AccessToken {
    access_token: String,
}

pub(crate) async fn get_access_token(code: String) -> Result<String> {
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
