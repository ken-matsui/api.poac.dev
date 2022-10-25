use crate::package::models::{Package, PackageOverview, PackageSearchResult};
use crate::schema::packages;
use diesel::dsl::sql;
use diesel::prelude::*;
use diesel::sql_types::{Array, Integer, Text};
use diesel::PgConnection;
use poac_api_utils::{log_query, DbError};
use serde::Serialize;

pub(crate) fn get_all(
    conn: &mut PgConnection,
    filter: Option<String>,
) -> Result<Vec<Package>, DbError> {
    if filter == Some("unique".to_string()) {
        // Get packages with the latest version
        let query = packages::table
            .order((
                packages::name,
                sql::<Text>("string_to_array(version, '.')::int[]"),
            ))
            .distinct_on(packages::name);
        log_query(&query);

        let results = query.load::<Package>(conn)?;
        Ok(results)
    } else {
        // Get packages ordered by version (newer first)
        let query = packages::table.order((
            packages::name,
            sql::<Text>("string_to_array(version, '.')::int[]"),
        ));
        log_query(&query);

        let results = query.load::<Package>(conn)?;
        Ok(results)
    }
}

pub(crate) fn search(
    conn: &mut PgConnection,
    text: &str,
    per_page: Option<i64>,
    sort: Option<String>, // newly_published, relevance
) -> Result<PackageSearchResult, DbError> {
    let mut query = packages::table
        .select((
            packages::id,
            packages::published_at,
            packages::name,
            packages::version,
            packages::edition,
            packages::description,
            sql::<diesel::sql_types::BigInt>("count(id) over()"),
        ))
        .order((
            packages::name,
            sql::<Text>("string_to_array(version, '.')::int[]"),
        ))
        .distinct_on(packages::name)
        .filter(packages::name.like(format!("%{}%", text)))
        .group_by(packages::id)
        .into_boxed();

    if Some("newly_published".to_string()) == sort {
        query = query.order(packages::published_at.desc());
    }
    if let Some(per_page) = per_page {
        query = query.limit(per_page);
    }
    log_query(&query);

    let results = query.load(conn)?;
    if results.is_empty() {
        return Ok(PackageSearchResult::empty());
    }
    let (_, _, _, _, _, _, cnt) = results[0];

    let mut packages = Vec::<PackageOverview>::new();
    for (id, published_at, name, version, edition, description, _) in results {
        packages.push(PackageOverview::new(
            id,
            published_at,
            name,
            version,
            edition,
            description,
        ));
    }

    let results = PackageSearchResult::new(cnt, packages);
    Ok(results)
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

pub(crate) fn versions(conn: &mut PgConnection, name_: &str) -> Result<Vec<String>, DbError> {
    let query = packages::table
        .select(packages::version)
        .filter(packages::name.eq(name_));
    log_query(&query);

    let results = query.load::<String>(conn)?;
    Ok(results)
}

pub(crate) fn deps(
    conn: &mut PgConnection,
    name_: &str,
    version_: &str,
) -> Result<Option<serde_json::Value>, DbError> {
    let query = packages::table
        .select(packages::metadata.retrieve_as_object("dependencies"))
        .filter(packages::name.eq(name_))
        .filter(packages::version.eq(version_));
    log_query(&query);

    let result = query.first::<serde_json::Value>(conn).optional()?;
    Ok(result)
}

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
