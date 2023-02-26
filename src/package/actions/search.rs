use crate::constants::PER_PAGE;
use crate::package::models::{PackageOverview, PackageSearchResult};
use crate::package::routes::search::Body;
use crate::schema::packages;

use diesel::dsl::sql;
use diesel::prelude::*;
use diesel::sql_types::Text;
use diesel::PgConnection;

use poac_api_utils::{log_query, DbError};

pub(crate) fn search(conn: &mut PgConnection, body: &Body) -> Result<PackageSearchResult, DbError> {
    let page = body.page.unwrap_or(1);
    let per_page = body.per_page.unwrap_or(PER_PAGE);
    let sort = body.sort.clone().unwrap_or("relevance".to_string());
    let filter = body.filter.clone().unwrap_or("unique".to_string());

    let start_index = (page - 1) * per_page;

    let mut query = if filter == "unique" {
        packages::table
            .select((
                packages::id,
                packages::published_at,
                packages::name,
                packages::version,
                packages::edition,
                packages::description,
                sql::<diesel::sql_types::BigInt>("count(id) over()"),
            ))
            .distinct_on(packages::name)
            .group_by(packages::id)
            .into_boxed()
    } else {
        packages::table
            .select((
                packages::id,
                packages::published_at,
                packages::name,
                packages::version,
                packages::edition,
                packages::description,
                sql::<diesel::sql_types::BigInt>("count(id) over()"),
            ))
            .group_by(packages::id)
            .into_boxed()
    };

    query = query
        .order((
            packages::name,
            sql::<Text>("string_to_array(version, '.')::int[]"),
        ))
        .filter(packages::name.like(format!("%{}%", body.query)))
        .offset(start_index)
        .limit(per_page);

    if sort == "newly_published" {
        query = query.order(packages::published_at.desc());
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
