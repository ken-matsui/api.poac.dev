use crate::schema::packages;

use diesel::prelude::*;
use diesel::PgConnection;

use poac_api_utils::{log_query, DbError};

pub(crate) fn deps(
    conn: &mut PgConnection,
    name_: &str,
    version_: &str,
) -> Result<Option<serde_json::Value>, DbError> {
    let query = packages::table
        .select(
            packages::metadata
                .retrieve_as_object("dependencies")
                .nullable(),
        )
        .filter(packages::name.eq(name_))
        .filter(packages::version.eq(version_));
    log_query(&query);

    let result = query.first::<Option<serde_json::Value>>(conn).optional()?;
    if let Some(result) = result {
        if let Some(result) = result {
            Ok(Some(result))
        } else {
            Ok(Some(serde_json::json!({})))
        }
    } else {
        Ok(None)
    }
}
