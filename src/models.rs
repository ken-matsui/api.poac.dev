use chrono::NaiveDateTime;
use diesel::debug_query;
use diesel::dsl::sql;
use diesel::pg::Pg;
use diesel::prelude::*;
use diesel::sql_types::Text;
use serde::{Deserialize, Serialize};
use uuid::Uuid;

type DbError = Box<dyn std::error::Error + Send + Sync>;

#[derive(Queryable, Serialize, Deserialize)]
pub(crate) struct Package {
    pub(crate) id: Uuid,
    pub(crate) published_at: NaiveDateTime,
    pub(crate) metadata: serde_json::Value,
    pub(crate) name: String,
    pub(crate) version: String,
    pub(crate) edition: i16,
    pub(crate) authors: Vec<String>,
    pub(crate) repository: String,
    pub(crate) description: String,
    pub(crate) license: String,
    pub(crate) readme: Option<String>,
    pub(crate) sha256sum: String,
}

impl Package {
    pub(crate) fn find_all(
        conn: &mut PgConnection,
        filter: Option<String>,
    ) -> Result<Vec<Self>, DbError> {
        use crate::schema::packages::dsl::*;

        if filter == Some("unique".to_string()) {
            // Get packages with the latest version
            let query = packages.distinct_on(name).order(name);
            // TODO: .order(sql::<Text>("string_to_array(version, '.')::int[]"))

            log::debug!("{}", debug_query::<Pg, _>(&query).to_string());

            let results = query.load::<Self>(conn)?;
            Ok(results)
        } else {
            // Get packages ordered by version (newer first)
            let query = packages
                .order(name)
                .order(sql::<Text>("string_to_array(version, '.')::int[]"));

            log::debug!("{}", debug_query::<Pg, _>(&query).to_string());

            let results = query.load::<Self>(conn)?;
            Ok(results)
        }
    }

    pub(crate) fn find(conn: &mut PgConnection, query: &str) -> Result<Vec<Self>, DbError> {
        use crate::schema::packages::dsl::*;

        let query = packages.filter(name.like(format!("%{}%", query)));

        log::debug!("{}", debug_query::<Pg, _>(&query).to_string());

        let results = query.load::<Self>(conn)?;
        Ok(results)
    }
}
