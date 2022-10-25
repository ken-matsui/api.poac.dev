use chrono::NaiveDateTime;
use diesel::prelude::Queryable;
use serde::{Deserialize, Serialize};
use uuid::Uuid;

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

#[derive(Serialize, Deserialize)]
pub(crate) struct PackageOverview {
    pub(crate) id: Uuid,
    pub(crate) published_at: NaiveDateTime,
    pub(crate) name: String,
    pub(crate) version: String,
    pub(crate) edition: i16,
    pub(crate) description: String,
}

impl PackageOverview {
    pub fn new(
        id: Uuid,
        published_at: NaiveDateTime,
        name: String,
        version: String,
        edition: i16,
        description: String,
    ) -> Self {
        PackageOverview {
            id,
            published_at,
            name,
            version,
            edition,
            description,
        }
    }
}

#[derive(Serialize, Deserialize)]
pub(crate) struct PackageSearchResult {
    pub(crate) total_count: i64,
    pub(crate) results: Vec<PackageOverview>,
}

impl PackageSearchResult {
    pub(crate) fn empty() -> Self {
        PackageSearchResult {
            total_count: 0,
            results: Vec::new(),
        }
    }

    pub(crate) fn new(total_count: i64, results: Vec<PackageOverview>) -> Self {
        PackageSearchResult {
            total_count,
            results,
        }
    }
}
