use actix_web::error::{ErrorBadRequest, ErrorInternalServerError, ErrorNotFound};
use actix_web::Result;
use awc::http::StatusCode;
use awc::Client;
use serde::Deserialize;

#[derive(Deserialize, Debug)]
struct UserEmail {
    email: String,
    primary: bool,
}

pub(crate) async fn get_email(access_token: &str) -> Result<String> {
    let client = Client::new();
    let mut result = client
        .get("https://api.github.com/user/emails")
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
    let emails =
        serde_json::from_slice::<Vec<UserEmail>>(&body).map_err(ErrorInternalServerError)?;

    for email in &emails {
        if email.primary {
            return Ok(email.email.clone());
        }
    }

    log::error!("What the heck user have no primary email?: {:?}", emails);
    Err(ErrorNotFound("Email not found"))
}
