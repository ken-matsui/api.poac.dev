use crate::package::models::Package;
use crate::schema::packages;

use diesel::prelude::*;
use diesel::PgConnection;

use poac_api_utils::{log_query, DbError};

pub(crate) fn dependents(conn: &mut PgConnection, name_: &str) -> Result<Vec<Package>, DbError> {
    let query = packages::table.filter(
        packages::metadata
            .retrieve_as_object("dependencies")
            .has_key(name_),
    );
    log_query(&query);

    let result = query.load::<Package>(conn)?;
    Ok(result)
}
