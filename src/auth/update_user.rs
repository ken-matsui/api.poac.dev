use crate::auth::find_user::User as OldUser;
use crate::auth::get_user_meta::UserMeta;
use crate::user::models::User;
use actix_web::error::ErrorInternalServerError;
use actix_web::{web, Result};
use diesel::prelude::*;
use poac_api_utils::{DbError, DbPool};

/// Update if user info is stale (user_meta must be up-to-date)
pub(crate) async fn update_user(
    pool: web::Data<DbPool>,
    user_meta: UserMeta,
    user: OldUser,
) -> Result<User> {
    let user = if user_meta.name != user.name || user_meta.avatar_url != user.avatar_url {
        web::block(move || -> Result<User, DbError> {
            use crate::schema::users::dsl::{avatar_url, id, name, user_name, users};

            let mut conn = pool.get()?;
            let user = diesel::update(users)
                .filter(user_name.eq(user_meta.user_name))
                .set((
                    name.eq(&user_meta.name),
                    avatar_url.eq(&user_meta.avatar_url),
                ))
                .returning((id, name, user_name, avatar_url))
                .get_result::<User>(&mut conn)?;

            Ok(user)
        })
        .await?
        .map_err(ErrorInternalServerError)?
    } else {
        User::from(user)
    };

    Ok(user)
}
