use diesel::sql_types::{Jsonb, Timestamptz, Uuid};
use diesel::Queryable;

#[derive(Queryable)]
pub(crate) struct Package {
    pub(crate) id: Uuid,
    pub(crate) published_at: Timestamptz,
    pub(crate) metadata: Jsonb,
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
