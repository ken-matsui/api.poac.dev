use diesel::r2d2::{ConnectionManager, Pool};
use diesel::PgConnection;

pub(crate) type DbPool = Pool<ConnectionManager<PgConnection>>;
