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

    pub(crate) fn maybe_ok(data: Option<T>, msg: String) -> HttpResponse {
        if let Some(data) = data {
            Self::ok(data)
        } else {
            HttpResponse::NotFound().json(ErrResponse::new(msg))
        }
    }
}

#[derive(Serialize)]
struct ErrResponse<T> {
    error: T,
}

impl<T: Serialize> ErrResponse<T> {
    fn new(error: T) -> Self {
        Self { error }
    }
}

#[inline]
pub(crate) fn concat_org_name(org: String, name: String) -> String {
    format!("{}/{}", org, name)
}

use diesel::pg::Pg;
use diesel::query_builder::QueryFragment;

#[inline]
pub(crate) fn log_query<T: QueryFragment<Pg>>(query: &T) {
    use diesel::debug_query;

    log::debug!("{}", debug_query::<Pg, _>(&query).to_string());
}
