use actix_web::HttpResponse;
use diesel::r2d2::{ConnectionManager, Pool};
use diesel::PgConnection;
use serde::Serialize;

pub(crate) type DbPool = Pool<ConnectionManager<PgConnection>>;
pub(crate) type DbError = Box<dyn std::error::Error + Send + Sync>;

#[derive(Serialize)]
pub(crate) struct Response<T> {
    data: T,
}

impl<T: Serialize> Response<T> {
    pub(crate) fn new(data: T) -> Self {
        Self { data }
    }

    pub(crate) fn ok(data: T) -> HttpResponse {
        HttpResponse::Ok().json(Self::new(data))
    }
}
