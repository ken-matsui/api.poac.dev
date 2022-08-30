use crate::package::Package;
use crate::utils::{log_query, DbError};
use diesel::dsl::sql;
use diesel::prelude::*;
use diesel::sql_types::Text;
use diesel::PgConnection;
use serde::Serialize;

pub(crate) fn get_all(
    conn: &mut PgConnection,
    filter: Option<String>,
) -> Result<Vec<Package>, DbError> {
    use crate::schema::packages::dsl::*;

    if filter == Some("unique".to_string()) {
        // Get packages with the latest version
        let query = packages.distinct_on(name).order(name);
        // TODO: .order(sql::<Text>("string_to_array(version, '.')::int[]"))
        log_query(&query);

        let results = query.load::<Package>(conn)?;
        Ok(results)
    } else {
        // Get packages ordered by version (newer first)
        let query = packages
            .order(name)
            .order(sql::<Text>("string_to_array(version, '.')::int[]"));
        log_query(&query);

        let results = query.load::<Package>(conn)?;
        Ok(results)
    }
}

pub(crate) fn search(
    conn: &mut PgConnection,
    query: &str,
    per_page: Option<i64>,
) -> Result<Vec<Package>, DbError> {
    use crate::schema::packages::dsl::*;

    let query = packages.filter(name.like(format!("%{}%", query)));

    if let Some(per_page) = per_page {
        let query = query.limit(per_page);
        log_query(&query);

        let results = query.load::<Package>(conn)?;
        Ok(results)
    } else {
        log_query(&query);

        let results = query.load::<Package>(conn)?;
        Ok(results)
    }
}

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
    use crate::schema::packages::dsl::*;

    let query = packages
        .select((repository, sha256sum))
        .filter(name.eq(name_))
        .filter(version.eq(version_));
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

pub(crate) fn versions(conn: &mut PgConnection, name_: &str) -> Result<Vec<String>, DbError> {
    use crate::schema::packages::dsl::*;

    let query = packages.select(version).filter(name.eq(name_));
    log_query(&query);

    let results = query.load::<String>(conn)?;
    Ok(results)
}

pub(crate) fn deps(
    conn: &mut PgConnection,
    name_: &str,
    version_: &str,
) -> Result<Option<serde_json::Value>, DbError> {
    use crate::schema::packages::dsl::*;

    let query = packages
        .select(metadata.retrieve_as_object("dependencies"))
        .filter(name.eq(name_))
        .filter(version.eq(version_));
    log_query(&query);

    let result = query.first::<serde_json::Value>(conn).optional()?;
    Ok(result)
}

pub(crate) fn dependents(conn: &mut PgConnection, name_: &str) -> Result<Vec<Package>, DbError> {
    use crate::schema::packages::dsl::*;

    let query = packages.filter(metadata.retrieve_as_object("dependencies").has_key(name_));
    log_query(&query);

    let result = query.load::<Package>(conn)?;
    Ok(result)
}