use crate::schema::packages;

use diesel::prelude::*;
use diesel::PgConnection;
use serde::Serialize;

use poac_api_utils::{log_query, DbError};

#[derive(Serialize)]
pub(crate) struct RepoInfo {
    pub(crate) repository: String,
    pub(crate) sha256sum: String,
}

pub(crate) fn repo_info(
    conn: &mut PgConnection,
    name_: &str,
    version_: &str,
) -> Result<Option<RepoInfo>, DbError> {
    let query = packages::table
        .select((packages::repository, packages::sha256sum))
        .filter(packages::name.eq(name_))
        .filter(packages::version.eq(version_));
    log_query(&query);

    let result = query
        .first::<(String, String)>(conn)
        .optional()?
        .map(|(repo, sha)| RepoInfo {
            repository: repo,
            sha256sum: sha,
        });
    Ok(result)
}
