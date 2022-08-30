use diesel::prelude::Queryable;
use serde::{Deserialize, Serialize};
use uuid::Uuid;

#[derive(Queryable, Serialize, Deserialize)]
pub(crate) struct User {
    pub(crate) id: Uuid,
    pub(crate) user_name: String,
    pub(crate) email: String,
    pub(crate) name: String,
    pub(crate) avatar_url: String,
    pub(crate) status: String,
}
