use crate::auth::get_user_meta::UserMeta;
use diesel::prelude::*;
use diesel::PgConnection;
use poac_api_utils::{log_query, DbError};
use serde::{Deserialize, Serialize};

#[derive(Queryable, Serialize, Deserialize)]
pub(crate) struct User {
    name: String,
    user_name: String,
    avatar_url: String,
    status: String,
}

pub(crate) fn find_user(
    conn: &mut PgConnection,
    user_meta: UserMeta,
) -> Result<Option<User>, DbError> {
    use crate::schema::users::dsl::*;

    let query = users
        .select((name, user_name, avatar_url, status))
        .filter(user_name.eq(user_meta.user_name));
    log_query(&query);

    let result = query.first::<User>(conn).optional()?;
    Ok(result)
}
