use crate::auth::get_user_meta::UserMeta;
use diesel::prelude::*;
use diesel::PgConnection;
use poac_api_utils::{log_query, DbError};
use serde::{Deserialize, Serialize};
use uuid::Uuid;

#[derive(Queryable, Serialize, Deserialize, Debug)]
pub(crate) struct User {
    id: Uuid,
    pub(crate) name: String,
    user_name: String,
    pub(crate) avatar_url: String,
    pub(crate) status: String,
}

impl From<User> for crate::user::models::User {
    fn from(user: User) -> Self {
        Self {
            id: user.id,
            name: user.name,
            user_name: user.user_name,
            avatar_url: user.avatar_url,
        }
    }
}

pub(crate) fn find_user(
    conn: &mut PgConnection,
    user_meta: UserMeta,
) -> Result<Option<User>, DbError> {
    use crate::schema::users::dsl::*;

    let query = users
        .select((id, name, user_name, avatar_url, status))
        .filter(user_name.eq(user_meta.user_name));
    log_query(&query);

    let result = query.first::<User>(conn).optional()?;
    Ok(result)
}
