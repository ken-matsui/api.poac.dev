use crate::package::models::Package;
use crate::schema::packages;

use diesel::dsl::sql;
use diesel::prelude::*;
use diesel::sql_types::{Array, Integer};
use diesel::PgConnection;

use poac_api_utils::{log_query, DbError};

pub(crate) fn owned_packages(
    conn: &mut PgConnection,
    user_name_: &str,
) -> Result<Vec<Package>, DbError> {
    use crate::schema::{ownerships, users};

    let query = packages::table
        .distinct_on(packages::name)
        .select(packages::all_columns)
        .left_join(ownerships::table.on(ownerships::package_name.eq(packages::name)))
        .left_join(users::table.on(users::id.eq(ownerships::user_id)))
        .filter(users::user_name.eq(user_name_))
        .order((
            packages::name,
            sql::<Array<Integer>>("string_to_array(version, '.')::int[]").desc(),
        ));
    log_query(&query);

    let result = query.load::<Package>(conn)?;
    Ok(result)
}
