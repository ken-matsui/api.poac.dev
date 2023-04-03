use crate::package::models::Package;
use crate::schema::packages;

use diesel::dsl::sql;
use diesel::prelude::*;
use diesel::sql_types::Text;
use diesel::PgConnection;

use poac_api_utils::{log_query, DbError};

pub(crate) fn details(conn: &mut PgConnection, name_: &str) -> Result<Vec<Package>, DbError> {
    let query = packages::table
        .order((
            packages::name,
            sql::<Text>("string_to_array(version, '.')::int[]"),
        ))
        .filter(packages::name.like(format!("%{}%", name_)));
    log_query(&query);

    let result = query.load::<Package>(conn)?;
    Ok(result)
}
