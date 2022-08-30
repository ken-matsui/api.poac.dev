use diesel::r2d2::{ConnectionManager, Pool};
use diesel::PgConnection;

pub(crate) type DbPool = Pool<ConnectionManager<PgConnection>>;
pub(crate) type DbError = Box<dyn std::error::Error + Send + Sync>;
