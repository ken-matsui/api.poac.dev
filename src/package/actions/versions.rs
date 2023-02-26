use crate::schema::packages;

use diesel::prelude::*;
use diesel::PgConnection;

use poac_api_utils::{log_query, DbError};

pub(crate) fn versions(conn: &mut PgConnection, name_: &str) -> Result<Vec<String>, DbError> {
    let query = packages::table
        .select(packages::version)
        .filter(packages::name.eq(name_));
    log_query(&query);

    let results = query.load::<String>(conn)?;
    Ok(results)
}
