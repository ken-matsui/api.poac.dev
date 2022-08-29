use chrono::NaiveDateTime;
use diesel::prelude::*;
use serde::{Deserialize, Serialize};
use uuid::Uuid;

type DbError = Box<dyn std::error::Error + Send + Sync>;

#[derive(Queryable)]
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
    pub(crate) fn find_all(conn: &mut PgConnection) -> Result<Vec<Self>, DbError> {
        use crate::schema::packages::dsl::*;

        let results = packages.load::<Self>(conn)?;

        Ok(results)
    }
}
