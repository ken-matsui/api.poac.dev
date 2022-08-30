mod log_query;
mod response;
mod ty;

pub use log_query::log_query;
pub use response::Response;
pub use ty::{DbError, DbPool};

#[inline]
pub fn concat_org_name(org: String, name: String) -> String {
    format!("{}/{}", org, name)
}
