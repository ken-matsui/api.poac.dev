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

use actix_web::error::{ErrorInternalServerError, ErrorUnauthorized};
use actix_web::{get, web, HttpResponse, Result};
use diesel::prelude::*;
use poac_api_utils::{DbError, DbPool, Response};
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
    let user = find_user(pool.clone(), user_meta.clone()).await?;

    let user = match user {
        None => {
            // Create a new user so that there was not the same user.
            let email = get_email(&access_token).await?;

            let pool_cloned = pool.clone();
            let user_meta_cloned = user_meta.clone();
            web::block(move || {
                let mut conn = pool_cloned.get()?;
                create_user(&mut conn, &user_meta_cloned, &email)
            })
            .await?
            .map_err(ErrorInternalServerError)?
        }
        Some(user) => {
            // This is NOT a new user
            if user.status != "active" {
                log::warn!("A disabled user tried to log in: {:?}", user);
                return Err(ErrorUnauthorized("You are not authorized."));
            }

            // Stale user info (user_meta must be up-to-date)
            if user_meta.name != user.name || user_meta.avatar_url != user.avatar_url {
                web::block(move || -> Result<crate::user::models::User, DbError> {
                    use crate::schema::users::dsl::{avatar_url, id, name, user_name, users};

                    let mut conn = pool.get()?;
                    let user = diesel::update(users)
                        .filter(user_name.eq(user_meta.user_name))
                        .set((
                            name.eq(&user_meta.name),
                            avatar_url.eq(&user_meta.avatar_url),
                        ))
                        .returning((id, name, user_name, avatar_url))
                        .get_result::<crate::user::models::User>(&mut conn)?;

                    Ok(user)
                })
                .await?
                .map_err(ErrorInternalServerError)?
            } else {
                crate::user::models::User::from(user)
            }
        }
    };

    // TODO: Convert user to base64
    Ok(Response::ok(user))
}

pub(crate) fn init_routes(cfg: &mut web::ServiceConfig) {
    cfg.service(auth_callback);
}
