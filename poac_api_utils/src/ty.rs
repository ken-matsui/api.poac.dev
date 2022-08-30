use diesel::r2d2::{ConnectionManager, Pool};
use diesel::PgConnection;

pub type DbPool = Pool<ConnectionManager<PgConnection>>;
pub type DbError = Box<dyn std::error::Error + Send + Sync>;
