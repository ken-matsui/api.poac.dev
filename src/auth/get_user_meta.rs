use actix_web::error::{ErrorBadRequest, ErrorInternalServerError};
use actix_web::Result;
use awc::http::StatusCode;
use awc::Client;
use serde::Deserialize;

#[derive(Deserialize, Clone)]
pub(crate) struct UserMeta {
    pub(crate) name: String,
    #[serde(rename(deserialize = "login"))]
    pub(crate) user_name: String,
    pub(crate) avatar_url: String,
    pub(crate) email: Option<String>,
}

pub(crate) async fn get_user_meta(access_token: &str) -> Result<UserMeta> {
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
