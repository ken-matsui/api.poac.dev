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
