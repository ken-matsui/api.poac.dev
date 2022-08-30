use diesel::debug_query;
use diesel::pg::Pg;
use diesel::query_builder::QueryFragment;

#[inline]
pub fn log_query<T: QueryFragment<Pg>>(query: &T) {
    log::debug!("{}", debug_query::<Pg, _>(&query).to_string());
}
