use crate::package::models::Package;
use crate::schema::packages;

use diesel::dsl::sql;
use diesel::prelude::*;
use diesel::sql_types::Text;
use diesel::PgConnection;

use poac_api_utils::{log_query, DbError};

pub(crate) fn get_all(
    conn: &mut PgConnection,
    filter: Option<String>,
) -> Result<Vec<Package>, DbError> {
    let mut query = if filter == Some("unique".to_string()) {
        // Get packages with the latest version
        packages::table.distinct_on(packages::name).into_boxed()
    } else {
        // Get packages ordered by version (newer first)
        packages::table.into_boxed()
    };

    query = query.order((
        packages::name,
        sql::<Text>("string_to_array(version, '.')::int[]"),
    ));
    log_query(&query);

    let results = query.load::<Package>(conn)?;
    Ok(results)
}
