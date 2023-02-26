use crate::user::models::User;

use diesel::prelude::*;
use diesel::PgConnection;

use poac_api_utils::{log_query, DbError};

pub(crate) fn owners(conn: &mut PgConnection, name_: &str) -> Result<Vec<User>, DbError> {
    use crate::schema::ownerships::dsl::{ownerships, package_name, user_id};
    use crate::schema::users::dsl::*;

    let query = users
        .select((id, name, user_name, avatar_url))
        .left_join(ownerships.on(package_name.eq(name_)))
        .filter(user_id.eq(id))
        .filter(status.eq("active"));
    log_query(&query);

    let results = query.load::<User>(conn)?;
    Ok(results)
}
